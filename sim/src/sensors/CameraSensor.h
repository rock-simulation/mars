/*
 *  Copyright 2011, 2012, 2014, DFKI GmbH Robotics Innovation Center
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

#ifndef CAMERA_SENSOR_H
#define CAMERA_SENSOR_H

#ifdef _PRINT_HEADER_
#warning "CameraSensor.h"
#endif

#include <mars/data_broker/ReceiverInterface.h>

#include <mars/interfaces/sim/SensorInterface.h>
#include <mars/utils/Vector.h>
#include <mars/utils/Quaternion.h>
#include <mars/utils/Mutex.h>
#include <mars/interfaces/graphics/GraphicsWindowInterface.h>
#include <mars/interfaces/graphics/GraphicsUpdateInterface.h>
#include <mars/interfaces/graphics/GraphicsCameraInterface.h>

#include <inttypes.h>
typedef uint8_t  u_int8_t;


namespace mars {

  namespace graphics {
    class GraphicsWindowInterface;
    class GraphicsCameraInterface;
  }

  namespace sim {

      class Pixel
      {
        public:
          u_int8_t r;
          u_int8_t g;
          u_int8_t b;
          u_int8_t a;
      } __attribute__ ((packed)) ;

      typedef float DistanceMeasurement;
      
    struct CameraConfigStruct: public interfaces::BaseConfig{
      CameraConfigStruct(){
        name = "Unknown Camera";
        width=640;
        height=480;
        enabled = true;
        show_cam = false;
        hud_pos=0;
        pos_offset.setZero();
        ori_offset.setIdentity();
        opening_width=90;
        opening_height=-1;
        hud_width = 320;
        hud_height = -1;
        depthImage = false;
        frameOffset = 1;
      }

      unsigned long attached_node;
      int width;
      int height;
      bool show_cam;
      int hud_pos;
      int frameOffset;
      utils::Vector pos_offset;
      utils::Quaternion ori_offset;
      double opening_width; // deprecated: we should probably rename this to opening_angle
      double opening_height; // deprecated: we should probably rename this to opening_angle2
      int hud_width;
      int hud_height;
      bool depthImage;
      bool enabled;
    };

    class CameraSensor : public interfaces::BaseNodeSensor,
                         public interfaces::SensorInterface,
                         public data_broker::ReceiverInterface,
                         public interfaces::GraphicsUpdateInterface {
    public:
      static interfaces::BaseSensor* instanciate(interfaces::ControlCenter *control,
                                           interfaces::BaseConfig *config );
      CameraSensor(interfaces::ControlCenter *control, const CameraConfigStruct config);
      ~CameraSensor(void);

      virtual int getSensorData(interfaces::sReal** data) const;

      void getImage(std::vector<Pixel> &buffer);
      void getDepthImage(std::vector<DistanceMeasurement> &buffer);
      
      virtual void receiveData(const data_broker::DataInfo &info,
                               const data_broker::DataPackage &package,
                               int callbackParam);

      virtual void preGraphicsUpdate(void);
      void getCameraInfo( interfaces::cameraStruct *cs );

      static interfaces::BaseConfig* parseConfig(interfaces::ControlCenter *control,
                                     configmaps::ConfigMap *config);
      virtual configmaps::ConfigMap createConfig() const;
      
      const CameraConfigStruct &getConfig() const
      {
        return config;
      }

      void deactivateRendering();
      void activateRendering();
      unsigned long getWindowID() const {return cam_window_id;}

    private:
      CameraConfigStruct config;
      interfaces::BaseCameraSensor<double> depthCamera;
      interfaces::BaseCameraSensor<char*> imageCamera;
      unsigned long cam_window_id;
      interfaces::GraphicsWindowInterface *gw;
      interfaces::GraphicsCameraInterface* gc;
      long dbPosIndices[3];
      long dbRotIndices[4];
      unsigned int cam_id;
      utils::Mutex mutex;
      int renderCam;
      unsigned long draw_id;
    };

  } // end of namespace sim
} // end of namespace mars

#endif
