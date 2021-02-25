/*
 *  Copyright 2011, 2012, DFKI GmbH Robotics Innovation Center
 *
 *  This file is part of the MARS simulation framework.
 *
 *  MARS is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation, either version 3
 *  of the License, or (at your option) any later version.
 *
 *  MARS is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with MARS.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 *  GraphicsCamera.cpp
 *  General GraphicsCamera to inherit from.
 *
 *  Created by Roemmermann on 20.10.09.
 */

#include "GraphicsCamera.h"
#include <osgGA/GUIEventAdapter>
#include <osgUtil/LineSegmentIntersector>
#include <cstdio>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Texture2D>
#include <osg/PositionAttitudeTransform>
#include <iostream>

#define LMB osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON
#define MMB osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON
#define RMB osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON

namespace mars {
  namespace graphics {

    using mars::interfaces::cameraStruct;
    using mars::utils::Vector;
    using mars::utils::Quaternion;

    GraphicsCamera::GraphicsCamera(osg::ref_ptr<osg::Camera> _camera,
                                   int g_width, int g_height) {

      mainCamera = _camera;
      hudCamera = 0;
      keyswitchManipulator = 0;
      nodeMask = mainCamera->getNodeMask();
      camera = TRACKBALL;
      previousType = camera;
      pivot = osg::Vec3f(0.0, 0.0, 0.0);
      camType = 1;
      switch_eyes = true;
      moveSpeed = 0.5;
      // set up ODE like camera

      xpos = 0;
      ypos = 0;
      xrot = 0;
      yrot = 0;
      d_xp = 0.0;
      d_yp = -5.0;
      d_zp = 0.0;
      d_xr = 0.0;
      d_yr = 90.0;
      d_zr = 0.0;

      l_settings = 0;
      //dsc = 0;
      stereo = 0;
      left = 1;
      width = g_width;
      height = g_height;

      f_nearPlane = MY_ZNEAR; //0.01;
      f_farPlane  = MY_ZFAR;  //1000;
      FILE* hack_aperture = fopen("aperture.cfg", "r");
      if(hack_aperture) {
        fscanf(hack_aperture, "%lf\n", &f_aperture);
        fprintf(stderr, "HACK: aperture: %g\n", f_aperture);
        fclose(hack_aperture);
      }
      else f_aperture  = 0.74;
      f_focal = 0.1;
      f_ratio = 4/3;
      f_win_ratio = 0.6666;


      separation = -0.2;
      calcEyeSep();

      f_top = f_nearPlane*tan(f_aperture);
      f_bottom = -f_top;

      cameraRotation.makeRotate(osg::DegreesToRadians(0.0),
                                osg::Vec3(0,1,0), // roll
                                osg::DegreesToRadians(double(d_yr)),
                                osg::Vec3(1,0,0) , // pitch
                                osg::DegreesToRadians(double(d_xr)),
                                osg::Vec3(0,0,1) ); // heading

      cameraTrans.makeTranslate( double(d_xp),double(d_yp),double(d_zp) );
      myCameraMatrix = cameraRotation * cameraTrans;

      isMovingLeft = isMovingRight = isMovingBack = isMovingForward = false;
      isoMinHeight = 2;
      isoMaxHeight = 20;
      logTrackingRotation = false;
    }

    GraphicsCamera::~GraphicsCamera(void) {
      if (l_settings) free(l_settings);
    }

    void GraphicsCamera::changeCameraTypeToPerspective() {
      osg::Matrix projection;
      float aspectRatio = static_cast<float>(width)/static_cast<float>(height);

      projection.makePerspective(50.0f, aspectRatio, MY_ZNEAR, MY_ZFAR);
      if (!l_settings) {
        mainCamera->setProjectionMatrix(projection);
        //if(hudCamera) hudCamera->setProjectionMatrix(projection);
      }
      camType = 1;
    }

    void GraphicsCamera::changeCameraTypeToOrtho() {
      osg::Matrix projection;
      float aspectRatio = static_cast<float>(width)/static_cast<float>(height);
      actOrtH = 5;
      double w = actOrtH * aspectRatio;
      projection.makeOrtho(-w/2,w/2,-actOrtH/2,actOrtH/2,1.0f,10000.0f);
      if (!l_settings) mainCamera->setProjectionMatrix(projection);
      camType = 2;
    }

    void GraphicsCamera::toggleTrackball() {
      if(camera == TRACKBALL) {
        setCamera(previousType);
      }
      else {
        previousType = camera;
        setCamera(TRACKBALL);
      }
    }

    void GraphicsCamera::setCamera(int type) {
      camera = type;
      if(camera == OSG_CAM) {
        if(keyswitchManipulator.valid()) {
          osg::Matrixd tmp_matrix;
          tmp_matrix.makeIdentity();
          //keyswitchManipulator->setByMatrix(tmp_matrix);
        }
      }
      if(camera == ISO_CAM){
        //move camera up and make it look down
        //get current position and translate along y axis to desired position
        d_zp = 10;
        osg::Matrixd rotMat;
        rotMat.makeRotate(osg::DegreesToRadians(30.f), osg::Vec3(1, 0, 0));
        osg::Quat q = rotMat.getRotate();
        updateViewportQuat(d_xp, d_yp, d_zp, q.x(), q.y(), q.z(), q.w());
      }
      if(camera == TRACKBALL) {
        pivot = osg::Vec3f(d_xp, d_yp, d_zp) + cameraRotation.getRotate()*osg::Vec3f(0, 0, -5);
      }
    }

    Vector GraphicsCamera::getCameraPosition() {
      Vector camPos(d_xp, d_yp, d_zp);
      return camPos;
    }

    void GraphicsCamera::update(void) {
      //frameStamp->setFrameNumber(frameStamp->getFrameNumber()+1);
      //frameStamp->setReferenceTime(osgTimer.delta_s(initialTick,osgTimer.tick()));

      //eventQueue->frame(frameStamp->getReferenceTime());
      //eventVisitor->setTraversalNumber(frameStamp->getFrameNumber());

      /*
        #ifdef DEBUG_TIME
        fprintf(stderr, "debug_time\n");
        QTime timer;
        timer.start();
        #endif
      */
      // pass frame stamp to the SceneView so that the update, cull
      // and draw traversals all use the same FrameStamp
      //   setFrameStamp(frameStamp.get());

      // eventually update camera position

      if(tracking.valid()) {
        osg::Vec3d p = tracking->getPosition();
        osg::Quat q = tracking->getAttitude();
        if(logTrackingRotation) {
          q.x() = q.y() = q.z() = 0.0;
          q.w() = 1.0;
        }
        p += q*offsetPos;
        q *= offsetRot;
        updateViewportQuat(p.x(), p.y(), p.z(), q.x(), q.y(), q.z(), q.w());
      }

      if (l_settings && l_settings->id) {
        osg::Vec3d v1, v2, v3;
        Vector node_pos;// = control->nodes->getPosition(l_settings->id);
        Vector cam_pos;
        Vector n;
        double s;

        cam_pos.x() = l_settings->pos[0];
        cam_pos.y() = l_settings->pos[1];
        cam_pos.z() = l_settings->pos[2];

        if(l_settings->rotate_with) {
          Quaternion q(1,0,0,0);// = control->nodes->getRotation(l_settings->id);
          //      cam_pos = QVRotate(q, cam_pos);
          cam_pos = (q * cam_pos);
        }
        cam_pos.x() += node_pos.x();
        cam_pos.y() += node_pos.y();
        cam_pos.z() += node_pos.z();

        n = cam_pos - node_pos;
        s = n.z() / (n.x()*n.x() + n.y()*n.y() + n.z()*n.z());
        n *= -s;
        n.z() += 1;
        v1[0] = node_pos.x();
        v1[1] = node_pos.y();
        v1[2] = node_pos.z();
        v2[0] = cam_pos.x();
        v2[1] = cam_pos.y();
        v2[2] = cam_pos.z();
        v3[0] = n.x();
        v3[1] = n.y();
        v3[2] = n.z();
        myCameraMatrix.makeLookAt(v2, v1, v3);
        mainCamera->setViewMatrix(myCameraMatrix);
      }
      else {
        //camera switcher
        if (camera == OSG_CAM){
          if (keyswitchManipulator && keyswitchManipulator.valid() &&
              keyswitchManipulator->getCurrentMatrixManipulator()) {
            //osgGA::MatrixManipulator* mm = keyswitchManipulator->getCurrentMatrixManipulator();
            osg::Matrixd tmp_matrix;
            tmp_matrix.makeIdentity();
            myCameraMatrix.preMult(keyswitchManipulator->getMatrix());
            //myCameraMatrix = keyswitchManipulator->getMatrix();
            keyswitchManipulator->setByMatrix(tmp_matrix);
            osg::Vec3d t_vec = myCameraMatrix.getTrans();
            d_xp = t_vec.x();
            d_yp = t_vec.y();
            d_zp = t_vec.z();
            osg::Quat t_q = myCameraMatrix.getRotate();
            cameraRotation.makeRotate(t_q);
          }
        }
        osg::Matrixd i = myCameraMatrix.inverse(myCameraMatrix);
        mainCamera->setViewMatrix(i);
      }
      if (stereo) {
        // here we paint the scene and have to switch the eye each frame
        //double delta;
        //while((delta = osgTimer.delta_m(lastTick, osgTimer.tick())) < 16.66)
        //if(16.66-delta > 2) usleep((15.66-delta)*1000);
        //lastTick = osgTimer.tick();
        if(switch_eyes) left *= -1;
        osg::Matrixd trans;
        osg::Matrixd i = myCameraMatrix;//.inverse(myCameraMatrix);

        if (left == 1) {
          mainCamera->setProjectionMatrixAsFrustum(f_left[0],
                                                   f_right[0],
                                                   f_bottom*f_win_ratio,
                                                   f_top*f_win_ratio,
                                                   f_nearPlane,
                                                   f_farPlane);
        }
        else {
          mainCamera->setProjectionMatrixAsFrustum(f_left[1],
                                                   f_right[1],
                                                   f_bottom*f_win_ratio,
                                                   f_top*f_win_ratio,
                                                   f_nearPlane,
                                                   f_farPlane);
        }

        trans.makeIdentity();
        trans.makeTranslate(eyeSep*left, 0, 0);
        i = trans*myCameraMatrix;// *trans;
        mainCamera->setViewMatrix(i.inverse(i));
      }

      /*
        #ifdef DEBUG_TIME
        cout << "Scene_Draw: " << timer.elapsed() << endl;
        timer.restart();
        #endif

        drawFPS++;
        if (!(drawFPS % 60)){
        QString myTitle = myName;
        if (showFramecount){
        myTitle.append("  [fps:");
        double tmpFps = 60/(frameStamp->getReferenceTime() - fps);
        myTitle.append(QString::number(tmpFps,'f',2));
        myTitle.append("]");
        }
        //set window title for main window
        if (myParent != NULL){
        //myParent->setWindowTitle(myTitle);
        }
        //else setWindowTitle(myTitle);
        fps = frameStamp->getReferenceTime();
        }
        #ifdef DEBUG_TIME
        cout << "Draw FPS: " << timer.elapsed() << endl;
        #endif
      */

      //move camera according to its movent state
      if(isMovingForward) moveForward(moveSpeed);
      if(isMovingBack) moveForward(-moveSpeed);
      if(isMovingLeft) moveRight(-moveSpeed);
      if(isMovingRight) moveRight(moveSpeed);
    }

    void GraphicsCamera::setViewport(int x, int y, int width, int height) {
      this->width = width;
      this->height = height;
      mainCamera->setViewport(x, y, width, height);
      if (hudCamera) {
        hudCamera->setViewport(0, 0, width, height);
      }
    }

    void GraphicsCamera::getCameraInfo(cameraStruct *s) {
      if (!s) return;
      osg::Quat q = myCameraMatrix.getRotate();
      s->rot.x() = q.x();
      s->rot.y() = q.y();
      s->rot.z() = q.z();
      s->rot.w() = q.w();

      osg::Vec3d t=myCameraMatrix.getTrans();
      s->pos[0] = t.x();
      s->pos[1] = t.y();
      s->pos[2] = t.z();

      // get the intrinsic parameters of the camera
      double fovy, aspectRatio, Zn, Zf;
      getOSGCamera()->getProjectionMatrixAsPerspective( fovy, aspectRatio, Zn, Zf );

      // center points are just the center of the image
      s->center_x = width / 2.0;
      s->center_y = height / 2.0;

      // the f_x and f_y paramters are calculated from
      // the fovy parameter we got from osg.
      // We set the scale value such that a
      // y value of tan(fovy/2) is equal to height/2.
      //
      // Note that fovy is in radians.
      double fovy_rad = fovy * M_PI / 180.0;
      s->scale_y = height / (2.0 * tan( fovy_rad / 2.0 ) );
      s->scale_x = width / (2.0 * aspectRatio * tan( fovy_rad / 2.0 ) );
    }

    void GraphicsCamera::setCameraView(cameraStruct cs) {
      myCameraMatrix.setRotate(osg::Quat(cs.rot.x() ,cs.rot.y(), cs.rot.z(), cs.rot.w()));
      myCameraMatrix.setTrans(cs.pos.x() ,cs.pos.y(), cs.pos.z());
      pivot = osg::Vec3f(cs.pos.x(), cs.pos.y(), cs.pos.z())+myCameraMatrix.getRotate()*osg::Vec3f(0, 0, -5);
    }


    /* <! --- OUTDATED FUNCTION, USE setFrustum BELOW --->
       void GraphicsCamera::setLocalSettings(local_settings *local) {
       l_settings = local;
       if(!local->id)
       mainCamera->setProjectionMatrixAsFrustum(local->left,
       local->right,
       local->bottom,
       local->top,
       local->my_near,
       local->my_far);
       }
    */


    void GraphicsCamera::setFrustumFromRad(double horizontalOpeningAngle,
                                           double verticalOpeningAngle,
                                           double near, double far){
      //assert((horizontalOpeningAngle < M_PI) && (horizontalOpeningAngle > 0));
      //assert((verticalOpeningAngle < M_PI) && (verticalOpeningAngle > 0));
      assert((near > 0) && (far > 0) && (far > near));
      double right = tan(horizontalOpeningAngle/2.0) * near;
      double left = -right;
      double top = tan(verticalOpeningAngle/2.0) * near;
      double bottom = -top;

      setFrustum(left, right, bottom, top, near, far);
    }

    void GraphicsCamera::setFrustum(double left, double right, double bottom, double top, double near, double far) {

      mainCamera->setProjectionMatrixAsFrustum(left, right,
                                               bottom, top,
                                               near, far);

      // to disable osg near and far plane caluclation:
      // mainCamera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
    }


    void GraphicsCamera::getFrustum(std::vector<double>& frustum) {

      double left, right, bottom, top, near, far;

      mainCamera->getProjectionMatrixAsFrustum(left,right,bottom,top,near,far);

      frustum.clear();

      frustum.push_back(left);
      frustum.push_back(right);
      frustum.push_back(bottom);
      frustum.push_back(top);
      frustum.push_back(near);
      frustum.push_back(far);
    }

    void GraphicsCamera::updateViewport(double rx, double ry, double tx,
                                        double ty, double tz, double rz,
                                        bool remember) {
      osg::Matrixd rot, rot2, trans;


      rot.makeIdentity();
      trans.makeIdentity();

      if (l_settings && !l_settings->id) {
        mainCamera->setProjectionMatrixAsFrustum(l_settings->left,
                                                 l_settings->right,
                                                 l_settings->bottom,
                                                 l_settings->top,
                                                 l_settings->my_near,
                                                 l_settings->my_far);
        rot.makeRotate(l_settings->rotation1[2], osg::Vec3f(0,0,1),
                       l_settings->rotation1[1], osg::Vec3f(-1,0,0),
                       l_settings->rotation1[0], osg::Vec3f(0,1,0));
        rot2.makeRotate(l_settings->rotation2[0], osg::Vec3f(0,1,0),
                        l_settings->rotation2[1], osg::Vec3f(-1,0,0),
                        l_settings->rotation2[2], osg::Vec3f(0,0,1));
        trans.makeTranslate(l_settings->pos[0],
                            l_settings->pos[1],
                            l_settings->pos[2]);
        //rot.mult(rot, rot2);
      }
      cameraRotation.makeRotate(osg::DegreesToRadians(rz),
                                osg::Vec3(0,1,0), // roll
                                osg::DegreesToRadians(ry),
                                osg::Vec3(1,0,0) , // pitch
                                osg::DegreesToRadians(rx),
                                osg::Vec3(0,0,1) ); // heading
      //cameraRotation.postMult(rot);
      cameraTrans.makeTranslate(tx, ty, tz);

      myCameraMatrix = cameraRotation*cameraTrans;
      myCameraMatrix.preMult(trans);
      myCameraMatrix.preMult(rot2);
      myCameraMatrix.preMult(rot);
      if (remember) {
        d_xp = tx;
        d_yp = ty;
        d_zp = tz;
        d_xr = rx;
        d_yr = ry;
        d_zr = rz;
        pivot = osg::Vec3f(tx, ty, tz)+osg::Vec3f(0, 0, -5)*cameraRotation;
      }
    }

    void GraphicsCamera::updateViewportQuat(double tx, double ty, double tz,
                                            double rx, double ry, double rz,
                                            double rw) {
      osg::Matrixd rot, rot2, trans;


      rot.makeIdentity();
      trans.makeIdentity();

      if (l_settings && !l_settings->id) {
        mainCamera->setProjectionMatrixAsFrustum(l_settings->left,
                                                 l_settings->right,
                                                 l_settings->bottom,
                                                 l_settings->top,
                                                 l_settings->my_near,
                                                 l_settings->my_far);
        rot.makeRotate(l_settings->rotation1[2], osg::Vec3f(0,0,1),
                       l_settings->rotation1[1], osg::Vec3f(-1,0,0),
                       l_settings->rotation1[0], osg::Vec3f(0,1,0));
        rot2.makeRotate(l_settings->rotation2[0], osg::Vec3f(0,1,0),
                        l_settings->rotation2[1], osg::Vec3f(-1,0,0),
                        l_settings->rotation2[2], osg::Vec3f(0,0,1));
        trans.makeTranslate(l_settings->pos[0],
                            l_settings->pos[1],
                            l_settings->pos[2]);
        //rot.mult(rot, rot2);
      }
      cameraRotation.makeRotate(osg::Quat(rx, ry, rz, rw));

      //cameraRotation.postMult(rot);
      cameraTrans.makeTranslate(tx, ty, tz);

      myCameraMatrix = cameraRotation*cameraTrans;
      //myCameraMatrix.preMult(trans);
      //myCameraMatrix.preMult(rot2);
      myCameraMatrix.preMult(rot);
      d_xp = tx;
      d_yp = ty;
      d_zp = tz;
    }

    void GraphicsCamera::lookAtIso(double x, double y, double z)
    {
      //set to iso camera if not set
      if(camera !=ISO_CAM){
        setCamera(ISO_CAM);
      }

      if(z > isoMinHeight && z < isoMaxHeight){d_zp = z;}

      //calculate horizontal forward vector
      osg::Vec3 dirTMP(0, 1, 0);
      dirTMP = dirTMP*cameraRotation;
      osg::Vec3 dir(dirTMP.x(), dirTMP.y(), 0);

      //calculate horizontal distance to targetPoint
      //camera angle
      double distance = d_zp * tan(osg::DegreesToRadians(30.0));
      dir.normalize();
      //go backwards
      dir *=-distance;
      d_xp = x + dir.x();
      d_yp = y + dir.y();

      osg::Quat q;
      q = cameraRotation.getRotate();
      updateViewportQuat(d_xp, d_yp, d_zp, q.x(), q.y(), q.z(), q.w());


    }

    void GraphicsCamera::getViewport(double *rx, double *ry, double *tx,
                                     double *ty, double *tz, double *rz) {
      *tx = d_xp;
      *ty = d_yp;
      *tz = d_zp;
      *rx = d_xr;
      *ry = d_yr;
      *rz = d_zr;
    }

    void GraphicsCamera::getViewportQuat(double *tx, double *ty, double *tz,
                                         double *rx, double *ry, double *rz,
                                         double *rw) {
      osg::Quat q = cameraRotation.getRotate();
      *tx = d_xp;
      *ty = d_yp;
      *tz = d_zp;
      *rx = q.x();
      *ry = q.y();
      *rz = q.z();
      *rw = q.w();
    }

    void GraphicsCamera::context_setCamPredefLeft() {
      updateViewport(180, 90, 0, 5, 0, 0, 1);
    }

    void GraphicsCamera::context_setCamPredefRight() {
      updateViewport(0, 90, 0, -5, 0, 0, 1);
    }

    void GraphicsCamera::context_setCamPredefFront() {
      updateViewport(90, 90, 5, 0, 0, 0, 1);
    }

    void GraphicsCamera::context_setCamPredefRear() {
      updateViewport(-90, 90, -5, 0, 0, 0, 1);
    }
    void GraphicsCamera::context_setCamPredefTop() {
      updateViewport(0, 0, 0, 0, 5, 0, 1);
    }

    void GraphicsCamera::context_setCamPredefBottom() {
      updateViewport(90, 180, 0, 0, -5, 0, 1);
    }

    void GraphicsCamera::setEyeSep(double value) {
      if (value < 0) {
        this->separation = value;
        calcEyeSep();
      }
      else {
        this->separation = (-1)*value;
        calcEyeSep();
      }
    }

    void GraphicsCamera::calcEyeSep(void) {
      eyeSep     = (f_focal*separation)/20;

      f_left[0]  = ((-f_ratio*f_nearPlane* tan(f_aperture))
                    -0.5*eyeSep*f_nearPlane/f_focal);
      f_right[0] = ((f_ratio*f_nearPlane*tan(f_aperture))
                    -0.5*eyeSep*f_nearPlane/f_focal);
      f_left[1]  = ((-f_ratio*f_nearPlane*tan(f_aperture))
                    +0.5*eyeSep*f_nearPlane/f_focal);
      f_right[1] = ((f_ratio*f_nearPlane*tan(f_aperture))
                    +0.5*eyeSep*f_nearPlane/f_focal);
    }

    double GraphicsCamera::getEyeSep(void) const {
      return (-1)*this->separation;
    }

    void GraphicsCamera::zoom(float speed)
    {
      if (camera == ISO_CAM) {
        //positive speed means zoom in which is moving downwards in this case
        d_zp += -speed;
        if(d_zp < isoMinHeight)d_zp = isoMinHeight;
        if(d_zp > isoMaxHeight)d_zp = isoMaxHeight;

        osg::Quat q;
        q = cameraRotation.getRotate();
        updateViewportQuat(d_xp, d_yp, d_zp, q.x(), q.y(), q.z(), q.w());
      }
    }

    void GraphicsCamera::move(bool isMoving, GraphicsCamera::Direction dir)
    {
      switch(dir){
      case FORWARD:
        isMovingForward = isMoving;
        break;
      case BACKWARD:
        isMovingBack = isMoving;
        break;
      case LEFT:
        isMovingLeft = isMoving;
        break;
      case RIGHT:
        isMovingRight = isMoving;
        break;
      }
    }

    void GraphicsCamera::moveForward(float speed)
    {
      if(camera == ISO_CAM){
        //get y vector of camera orientation which is somewhat horizontal
        //it would be better to get a completely horizontal vector so speed
        //is the same as in the other camera modes
        //move faster if cam is higher
        osg::Vec3 dir(0, speed*(d_zp/isoMaxHeight)*1.7, 0);
        dir = dir*cameraRotation;
        d_xp +=dir.x();
        d_yp += dir.y();

      }else{
        osg::Vec3 vec(0, 0, 0);
        vec = osg::Vec3(0.0,0.0,(-speed));
        vec = vec*cameraRotation;
        d_xp += vec.x();
        d_yp += vec.y();
        d_zp += vec.z();
      }
      osg::Quat q;
      q = cameraRotation.getRotate();
      updateViewportQuat(d_xp, d_yp, d_zp, q.x(), q.y(), q.z(), q.w());

    }

    void GraphicsCamera::moveRight(float speed)
    {
      if(camera == ISO_CAM){
        osg::Vec3 vec(speed*(d_zp/isoMaxHeight)*1.7, 0, 0);
        vec = vec*cameraRotation;
        d_xp += vec.x();
        d_yp += vec.y();
      }else{
        osg::Vec3 vec(0, 0, 0);
        vec = osg::Vec3(speed,0.0,0.0);
        vec = vec*cameraRotation;
        d_xp += vec.x();
        d_yp += vec.y();
        d_zp += vec.z();
      }
      osg::Quat q;
      q = cameraRotation.getRotate();
      updateViewportQuat(d_xp, d_yp, d_zp, q.x(), q.y(), q.z(), q.w());
    }

    osg::Vec3f GraphicsCamera::getClickedPoint(int x, int y)
    {
      osg::ref_ptr<osgUtil::LineSegmentIntersector> intersector = new osgUtil::LineSegmentIntersector( osgUtil::Intersector::WINDOW, x, y);

      osgUtil::IntersectionVisitor iv(intersector.get());
      iv.setTraversalMask( ~0x1 );
      mainCamera->accept( iv );


      if ( intersector->containsIntersections() )
        {
          //use first intersection
          const osgUtil::LineSegmentIntersector::Intersection& result = *(intersector->getIntersections().begin());
          osg::Vec3d worldIntersect =   result.getWorldIntersectPoint();
          return osg::Vec3f(worldIntersect.x(), worldIntersect.y(), 0);



        }else{
        //if no intersection use camera postion
        return osg::Vec3f(d_xp, d_yp, 0);
      }


    }

    void GraphicsCamera::mouseDrag(int button, unsigned int modkey, int x, int y) {
      if(abs(x-xpos) < 1 and abs(y-ypos) < 1) return;
      double xdiff = (x-xpos)*0.2;
      double ydiff = (y-ypos)*0.2;
      xpos = x;
      ypos = y;

      //mouse control for the ISO camera
      if(camera == ISO_CAM){
        //rotate the camera with middle mouse button
        if ( (button == MMB) || (button == (LMB | RMB)) ){
          osg::Matrixd final, rot;
          //mouse up and down rotates(value is arbitrary)
          float angle = osg::DegreesToRadians(3.f*(xdiff));
          rot.makeRotate(angle, osg::Vec3(0, 0, 1));
          final = cameraRotation * rot;
          osg::Quat q= final.getRotate();

          //move around the clicked point
          //if no intersection is returned move around cam center i.e. dont't move, only rotate
          osg::Vec3 rotPoint = getClickedPoint(x, y);
          osg::Vec3 rotCam = osg::Vec3(d_xp, d_yp , 0) - rotPoint;

          osg::Matrixd rotMat;
          rotMat.makeRotate(angle, osg::Vec3(0, 0, 1));
          osg::Vec3 rotRes = rotCam * rotMat;

          d_xp = rotRes.x()+rotPoint.x();
          d_yp = rotRes.y()+rotPoint.y();

          updateViewportQuat(d_xp, d_yp, d_zp, q.x(), q.y(), q.z(), q.w());

          //pan with left mouse
        }else if(button == LMB){
          moveRight(-xdiff*0.1);
          moveForward(-ydiff*0.1);
        }
        return;
      }
      osg::Matrixd tmp, tmp2;
      double td_xr = 0, td_yr = 0, td_zr = 0;
      osg::Vec3 vec(0,0,0);
      osg::Quat q;

      tmp = cameraRotation;
      tmp2.makeIdentity();

      if(camera == TRACKBALL){
        if ((button == MMB) || (button == (LMB | RMB))) {
          if (modkey & osgGA::GUIEventAdapter::MODKEY_SHIFT) {
            vec = osg::Vec3(-xdiff*0.1,-ydiff*0.1,0.0);
            vec = vec*cameraRotation;
            pivot += vec;
            d_xp += vec.x();
            d_yp += vec.y();
            d_zp += vec.z();
            q = cameraRotation.getRotate();
            updateViewportQuat(d_xp, d_yp, d_zp, q.x(), q.y(), q.z(), q.w());
          }
          else {
            double xratio = (x-width*0.5)/(width*0.5);
            double yratio = (y-height*0.5)/(height*0.5);
            double xdir = 1, ydir = 1;
            if(xratio<0) xdir=-1;
            if(yratio<0) ydir=-1;
            xratio = fabs(xratio);
            yratio = fabs(yratio);
            td_zr = xdiff * (1-yratio);
            td_yr = ydiff * (1-xratio);
            td_xr = -ydiff*xdir*xratio + xdiff*ydir*yratio;

            osg::Matrixd rotate;
            osg::Vec3 vx(1,0,0),vy(0,1,0),vz(0,0,1);
            vec = osg::Vec3(d_xp,d_yp, d_zp);

            vx = vx*cameraRotation;
            vy = vy*cameraRotation;
            vz = vz*cameraRotation;

            //if(td_xr < 0.4 && td_xr > -0.4) td_xr = 0.0;
            //if(td_yr < 0.4 && td_yr > -0.4) td_yr = 0.0;
            //if(td_zr < 0.4 && td_zr > -0.4) td_zr = 0.0;

            rotate.makeRotate(osg::DegreesToRadians(-td_zr), vy,
                              osg::DegreesToRadians(td_yr), vx,
                              osg::DegreesToRadians(td_xr), vz);
            vec = (vec-pivot)*rotate;
            rotate.preMult(cameraRotation);
            q = rotate.getRotate();

            d_xp = pivot.x()+vec.x();
            d_yp = pivot.y()+vec.y();
            d_zp = pivot.z()+vec.z();
            updateViewportQuat(d_xp, d_yp, d_zp, q.x(), q.y(), q.z(), q.w());
          }
        }
        else if (button == RMB) {
          vec = osg::Vec3(0.0,0.0,ydiff*0.1);
          vec = vec*cameraRotation;
          d_xp += vec.x();
          d_yp += vec.y();
          d_zp += vec.z();
          q = cameraRotation.getRotate();
          updateViewportQuat(d_xp, d_yp, d_zp, q.x(), q.y(), q.z(), q.w());
        }
      }
      else {
        if (button == LMB){
          td_xr = xdiff;
          td_yr = -ydiff;
        }
        else if (button == RMB){
          vec = osg::Vec3(-xdiff*0.05,0.0,ydiff*0.05);
          vec = vec*cameraRotation;
          if (camType == 2){
            vec[2] = vec[2]*0.02;
            osg::Matrix projection;
            float aspectRatio = static_cast<float>(width)/static_cast<float>(height);
            actOrtH-=ydiff*0.5;
            double w = actOrtH * aspectRatio;
            projection.makeOrtho(-w/2,w/2,-actOrtH/2,actOrtH/2,1.0f,10000.0f);
            mainCamera->setProjectionMatrix(projection);
          }
        }
        else if ( (button == MMB) || (button == (LMB | RMB)) ) {
          vec = osg::Vec3(-xdiff*0.05,-ydiff*0.05,0.0);
          vec = vec*cameraRotation;
        }

        if (l_settings && !l_settings->id) {
          // to be reimplemented via interface
          //control->gui->viewportChange(td_xr, td_yr, vec.x(), vec.y(), vec.z());
        }
        else {
          osg::Quat q;
          if (camera == MICHA_CAM && !l_settings) {
            osg::Matrixd rotate;
            osg::Vec3 vx(1,0,0),vy(0,1,0),vz(0,0,1);

            vx = vx*cameraRotation;

            //rotate.makeRotate(0.0, vy, osg::DegreesToRadians(-td_xr), vz,
            //                  osg::DegreesToRadians(-td_yr), vx);
            rotate.makeRotate(0.0, vy, osg::DegreesToRadians(-td_yr), vx,
                              osg::DegreesToRadians(-td_xr), vz);
            rotate.preMult(cameraRotation);
            q = rotate.getRotate();
            d_xp += vec.x();
            d_yp += vec.y();
            d_zp += vec.z();
          }
          else {
            osg::Matrixd rotate;
            osg::Vec3 vx(1,0,0),vy(0,1,0),vz(0,0,1);

            vx = vx*cameraRotation;

            //if(td_xr < 0.4 && td_xr > -0.4) td_xr = 0.0;
            //if(td_yr < 0.4 && td_yr > -0.4) td_yr = 0.0;

            //rotate.makeRotate(0.0, vy, osg::DegreesToRadians(td_xr), vz,
            //                  osg::DegreesToRadians(td_yr), vx);
            rotate.makeRotate(0.0, vy, osg::DegreesToRadians(td_yr), vx,
                              osg::DegreesToRadians(td_xr), vz);
            rotate.preMult(cameraRotation);
            q = rotate.getRotate();

            d_xp += vec.x();
            d_yp += vec.y();
            d_zp += vec.z();
          }
          updateViewportQuat(d_xp, d_yp, d_zp, q.x(), q.y(), q.z(), q.w());
        }
      }
    }

    void GraphicsCamera::eventStartPos(int x, int y) {
      xpos = x;
      ypos = y;
    }

    void GraphicsCamera::setKeyswitchManipulator(osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator) {
      osg::Matrixd tmp_matrix;
      tmp_matrix.makeIdentity();
      this->keyswitchManipulator = keyswitchManipulator;
      keyswitchManipulator->setByMatrix(tmp_matrix);
    }

    void GraphicsCamera::setStereoMode(bool _stereo) {
      stereo = _stereo;
    }

    void GraphicsCamera::toggleStereoMode(void) {
      stereo = !stereo;
    }

    void GraphicsCamera::setFocalLength(double value) {
      if (value < 0) {
        this->f_focal = (-1)*value;
        calcEyeSep();
      }
      else {
        this->f_focal = value;
        calcEyeSep();
      }
    }

    double GraphicsCamera::getFocalLength(void) const {
      return this->f_focal;
    }


    int GraphicsCamera::getCamera(void) const {
      return this->camera;
    }

    osg::ref_ptr<osg::Camera> GraphicsCamera::getOSGCamera() {
      return mainCamera;
    }

    int GraphicsCamera::getCameraType(void) const {
      return this->camType;
    }

    void GraphicsCamera::deactivateCam() {
      nodeMask = mainCamera->getNodeMask();
      mainCamera->setNodeMask(0);
    }

    void GraphicsCamera::activateCam() {
      if(mainCamera->getNodeMask() == 0){
          mainCamera->setNodeMask(nodeMask);
      }
    }

    void GraphicsCamera::setPivot(osg::Vec3f p) {
      osg::Vec3f c(d_xp, d_yp, d_zp);
      double l = (c-pivot).length();
      pivot = p;
      if(camera == TRACKBALL){
        osg::Quat q = cameraRotation.getRotate();
        osg::Vec3f diff = q*osg::Vec3f(0, 0, 1);
        d_xp = pivot.x()+diff.x()*l;
        d_yp = pivot.y()+diff.y()*l;
        d_zp = pivot.z()+diff.z()*l;
        updateViewportQuat(d_xp, d_yp, d_zp, q.x(), q.y(), q.z(), q.w());
      }
    }

    void GraphicsCamera::setupDistortion(osg::Texture2D *texture, osg::Image *image, osg::Group *mainScene, double factor) {
      // create the quad to visualize.
      osg::Geometry* polyGeom = new osg::Geometry();

      polyGeom->setSupportsDisplayList(false);

      osg::Vec3 origin(0.0f,0.0f,0.0f);
      osg::Vec3 xAxis(1.0f,0.0f,0.0f);
      osg::Vec3 yAxis(0.0f,1.0f,0.0f);
      float height = 1024.0f;
      float width = 1280.0f;
      int noSteps = 50;

      osg::Vec3Array* vertices = new osg::Vec3Array;
      osg::Vec2Array* texcoords = new osg::Vec2Array;
      osg::Vec4Array* colors = new osg::Vec4Array;

      osg::Vec3 bottom = origin;
      osg::Vec3 dx = xAxis*(width/((float)(noSteps-1)));
      osg::Vec3 dy = yAxis*(height/((float)(noSteps-1)));

      osg::Vec2 bottom_texcoord(0.0f,0.0f);
      osg::Vec2 dx_texcoord(1.0f/(float)(noSteps-1),0.0f);
      osg::Vec2 dy_texcoord(0.0f,1.0f/(float)(noSteps-1));

      int i,j;
      double f1 = factor*osg::PI*0.999;
      double f2 = 0.5/tan(f1*0.5);
      for(i=0;i<noSteps;++i) {
        osg::Vec3 cursor = bottom+dy*(float)i;
        osg::Vec2 texcoord = bottom_texcoord+dy_texcoord*(float)i;
        for(j=0;j<noSteps;++j) {
          vertices->push_back(cursor);
          // texcoords->push_back(osg::Vec2((sin(texcoord.x()*osg::PI-osg::PI*0.5)+1.0f)*0.5f,
          //                                (sin(texcoord.y()*osg::PI-osg::PI*0.5)+1.0f)*0.5f));
          //texcoords->push_back(osg::Vec2(texcoord.x(), texcoord.y()));
          double x = tan((texcoord.x()-0.5)*f1)*f2+0.5;
          double y = tan((texcoord.y()-0.5)*f1)*f2+0.5;
          texcoords->push_back(osg::Vec2f(x,y));
          colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));

          cursor += dx;
          texcoord += dx_texcoord;
        }
      }

      // pass the created vertex array to the points geometry object.
      polyGeom->setVertexArray(vertices);
      polyGeom->setColorArray(colors, osg::Array::BIND_PER_VERTEX);
      polyGeom->setTexCoordArray(0,texcoords);


      for(i=0;i<noSteps-1;++i) {
        osg::DrawElementsUShort* elements = new osg::DrawElementsUShort(osg::PrimitiveSet::QUAD_STRIP);
        for(j=0;j<noSteps;++j) {
          elements->push_back(j+(i+1)*noSteps);
          elements->push_back(j+(i)*noSteps);
        }
        polyGeom->addPrimitiveSet(elements);
      }

      // new we need to add the texture to the Drawable, we do so by creating a
      // StateSet to contain the Texture StateAttribute.
      osg::StateSet* stateset = polyGeom->getOrCreateStateSet();
      stateset->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
      stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

      osg::Geode* geode = new osg::Geode();
      geode->addDrawable(polyGeom);

      // set up the camera to render the textured quad
      osg::Camera* camera = new osg::Camera;

      // just inherit the main cameras view
      camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
      camera->setViewMatrix(osg::Matrix::identity());
      camera->setViewport(0, 0, width, height);
      camera->setProjectionMatrixAsOrtho2D(0,width,0,height);
      camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
      camera->setAllowEventFocus(false);
      camera->setClearColor(osg::Vec4(1, 0, 0, 0.5));
      camera->setClearMask(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

      // set the camera to render before the main camera.
      camera->setRenderOrder(osg::Camera::PRE_RENDER);
      // add subgraph to render
      camera->addChild(geode);
      camera->attach(osg::Camera::COLOR_BUFFER, image);
      mainScene->addChild(camera);
    }

    void GraphicsCamera::setTrakingTransform(osg::ref_ptr<osg::PositionAttitudeTransform> t) {
      tracking = t;
      if(tracking.valid()) {
        double x[7];
        getViewportQuat(x, x+1, x+2, x+3, x+4, x+5, x+6);
        offsetPos.x() = x[0];
        offsetPos.y() = x[1];
        offsetPos.z() = x[2];
        offsetRot.x() = x[3];
        offsetRot.y() = x[4];
        offsetRot.z() = x[5];
        offsetRot.w() = x[6];
      }
    }

    bool GraphicsCamera::isTracking() {
      return tracking.valid();
    }

    void GraphicsCamera::setTrackingLogRotation(bool b) {
      logTrackingRotation = b;
    }

    void GraphicsCamera::getOffsetQuat(double *tx, double *ty, double *tz,
                                       double *rx, double *ry, double *rz,
                                       double *rw) {
      *tx = offsetPos.x();
      *ty = offsetPos.y();
      *tz = offsetPos.z();
      *rx = offsetRot.x();
      *ry = offsetRot.y();
      *rz = offsetRot.z();
      *rw = offsetRot.w();
    }

    void GraphicsCamera::setOffsetQuat(double tx, double ty, double tz,
                                       double rx, double ry, double rz,
                                       double rw) {
      offsetPos.x() = tx;
      offsetPos.y() = ty;
      offsetPos.z() = tz;
      offsetRot.x() = rx;
      offsetRot.y() = ry;
      offsetRot.z() = rz;
      offsetRot.w() = rw;
    }

  } // end of namespace graphics
} // end of namespace mars
