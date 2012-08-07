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
 *  GraphicsCamera.h
 *  General GraphicsCamera to inherit from.
 *
 *  Created by Roemmermann on 20.10.09.
 */

#ifndef MARS_GRAPHICS_CAMERA_OBJECT_H
#define MARS_GRAPHICS_CAMERA_OBJECT_H

#ifdef _PRINT_HEADER_
  #warning "GraphicsCamera.h"
#endif

#include <osg/Camera>
#include <osgGA/KeySwitchMatrixManipulator>
#include <mars/interfaces/graphics/GraphicsCameraInterface.h>

#define MY_ZNEAR    0.01
#define MY_ZFAR  1000.0

namespace mars {
  namespace graphics {

    class GraphicsCamera : public interfaces::GraphicsCameraInterface {
      //Q_OBJECT

    public:
      // note: default values for width and height come from the prior implementation
      // don't know if anyone needs it like that (jsc)
      GraphicsCamera(osg::ref_ptr<osg::Camera> _camera, int width = 720, int height = 505 );
      ~GraphicsCamera(void);

      enum Direction{FORWARD, BACKWARD, LEFT, RIGHT};
      virtual void setFrustum(double left, double right,
                              double bottom, double top,
                              double near, double far);
      virtual void setFrustumFromRad(double horizontalOpeningAngle,
                                     double verticalOpeningAngle,
                                     double near, double far);

      virtual void getFrustum(std::vector<double>& frustum);

      virtual void updateViewport(double rx, double ry, double tx,
                                  double ty, double tz, double rz = 0, bool remember = 0);
      virtual void updateViewportQuat(double tx, double ty, double tz,
                                      double rx, double ry, double rz,
                                      double rw);
      virtual void lookAtIso(double x, double y, double z = 0);
      virtual void getViewport(double *rx, double *ry, double *tx,
                               double *ty, double *tz, double *rz);

      virtual void getViewportQuat(double *tx, double *ty, double *tz,
                                   double *rx, double *ry, double *rz,
                                   double *rw);

      void setEyeSep(double value);
      double getEyeSep(void) const;
      /**\brief sets the camera type*/
      virtual void setCamera(int type);
      virtual int getCameraType(void) const;
      virtual int getCamera(void) const;
      osg::ref_ptr<osg::Camera> getOSGCamera();
      /**\brief sets the camera view */
      utils::Vector getCameraPosition();
      /* returns vector with current camera position */
      void setCameraView(interfaces::cameraStruct cs);
      /**\brief returns the cameraStruct */
      virtual void getCameraInfo(interfaces::cameraStruct *s);
      void update(void);
      void setViewport(int x, int y, int width, int height);
      void eventStartPos(int x, int y);
      void setKeyswitchManipulator(osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator);
      void mouseDrag(int button, int x, int y);
  
      //keyboard control functions
      /**sets the camera motion state */
      void move(bool isMoving, Direction dir);
  
      /**moves camera up and donwn in iso mode*/
      void zoom(float speed);
  

      //protected slots:
      /**\brief set camera type by context menu */
      virtual void changeCameraTypeToPerspective();
      virtual void changeCameraTypeToOrtho();
      void openSetCamViewport();
      void context_setCamPredefLeft();
      void context_setCamPredefRight();
      void context_setCamPredefFront();
      void context_setCamPredefRear();
      void context_setCamPredefTop();
      void context_setCamPredefBottom();
      void setStereoMode(bool _stereo);
      void toggleStereoMode(void);
      void setFocalLength(double value);
      double getFocalLength(void) const;
      void setSwitchEye(bool val) {switch_eyes = val;}
      void setLeftEye(void) {left = true;}
      void setRightEye(void) {left = false;}

    private:
      void calcEyeSep(void);
      // for vibot we need some extensions
      interfaces::local_settings *l_settings;

      osg::ref_ptr<osg::Camera> mainCamera;
      osg::ref_ptr<osg::Camera> hudCamera;
      osg::Matrixd myCameraMatrix;
      osg::Matrixd cameraRotation;
      osg::Matrixd cameraTrans;
      osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator;
      int camera;
      int camType;
      double actOrtH;
      int width, height;
      //ODE camera control global variables
      int xpos, ypos, xrot, yrot;
      double d_xp, d_yp, d_zp, d_xr, d_yr, d_zr;
      double f_nearPlane, f_farPlane, f_aperture, f_focal;
      double f_left[2], f_right[2], f_top, f_bottom;
      double f_ratio;
      double f_win_ratio;
      double separation;
      double eyeSep; // separation of the eyes (displacement of one eye)
      bool stereo;
      bool switch_eyes;
      short left;
  
      /*flags that are set to true when corresponing key is pressed 
       * and to false when it is release*/
      bool isMovingForward, isMovingBack, isMovingLeft, isMovingRight;
  
      double isoMinHeight,isoMaxHeight;

      /**moves the camera forward with positive speed and backwards with negative speed*/
      void moveForward(float speed);
      /**moves te camera left with positive speed and right with negative speed */
      void moveRight(float speed);
      osg::Vec3f getClickedPoint(int x, int y);

    }; // end of class GraphicsCamera

  } // end of namespace graphics
} // end of namespace mars

#endif  /* MARS_GRAPHICS_CAMERA_OBJECT_H */
