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

      camera = ODE_CAM;
      camType = 1;
      switch_eyes = true;

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
      if(isMovingForward) moveForward(0.1f);
      if(isMovingBack) moveForward(-0.1f);
      if(isMovingLeft) moveRight(-0.1f);
      if(isMovingRight) moveRight(0.1f);
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
      // y value of sin(fovy/2) is equal to height/2.
      //
      // Note that fovy is in radians.
      double fovy_rad = fovy * M_PI / 180.0;
      s->scale_y = height / (2.0 * sin( fovy_rad / 2.0 ) );
      s->scale_x = s->scale_y;
    }

    void GraphicsCamera::setCameraView(cameraStruct cs) {
      myCameraMatrix.setRotate(osg::Quat(cs.rot.x() ,cs.rot.y(), cs.rot.z(), cs.rot.w()));
      myCameraMatrix.setTrans(cs.pos.x() ,cs.pos.y(), cs.pos.z());
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
      assert((horizontalOpeningAngle < M_PI) && (horizontalOpeningAngle > 0));
      assert((verticalOpeningAngle < M_PI) && (verticalOpeningAngle > 0));
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

    void GraphicsCamera::mouseDrag(int button, int x, int y) {
      //mouse control for the ISO camera
      if(camera == ISO_CAM){
        //rotate the camera with middle mouse button
        if ( (button == MMB) || (button == (LMB | RMB)) ){
          osg::Matrixd final, rot;
          //mouse up and down rotates(value is arbitrary)
          float angle = osg::DegreesToRadians(3.f*(x-xpos));
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
          moveRight((xpos - x)*0.1);
          moveForward((ypos - y)*0.1);
        }
        xpos = x;
        ypos = y;
        return;
      }
      osg::Matrixd tmp, tmp2;
      double td_xr = 0, td_yr = 0;
      osg::Vec3 vec(0,0,0);

      tmp = cameraRotation;
      tmp2.makeIdentity();

      if (button == LMB){
        td_xr = (x - xpos)*0.5;
        td_yr = -(y - ypos)*0.5;
      }
      else if (button == RMB){
        vec = osg::Vec3(-(x-xpos)*0.01,0.0,(y-ypos)*0.01);
        vec = vec*cameraRotation;
        if (camType == 2){
          vec[2] = vec[2]*0.02;
          osg::Matrix projection;
          float aspectRatio = static_cast<float>(width)/static_cast<float>(height);
          actOrtH-=(y-ypos)*0.1;
          double w = actOrtH * aspectRatio;
          projection.makeOrtho(-w/2,w/2,-actOrtH/2,actOrtH/2,1.0f,10000.0f);
          mainCamera->setProjectionMatrix(projection);
        }
      }
      else if ( (button == MMB) || (button == (LMB | RMB)) ) {
        vec = osg::Vec3(-(x-xpos)*0.01,-(y-ypos)*0.01,0.0);
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
      
          if(td_xr < 0.4 && td_xr > -0.4) td_xr = 0.0;
          if(td_yr < 0.4 && td_yr > -0.4) td_yr = 0.0;

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
      xpos = x;
      ypos = y;
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

  } // end of namespace graphics
} // end of namespace mars
