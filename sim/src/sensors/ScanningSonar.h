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

#ifndef __SCANNING_SONAR_H__
#define __SCANNING_SONAR_H__

#ifdef _PRINT_HEADER_
#warning "ScanningSonar.h"
#endif

#include <mars/data_broker/ReceiverInterface.h>

#include <mars/utils/Vector.h>
#include <mars/utils/Quaternion.h>
#include <mars/utils/mathUtils.h>
#include <mars/interfaces/sim/SensorInterface.h>
#include <mars/interfaces/graphics/GraphicsWindowInterface.h>
#include <mars/interfaces/graphics/GraphicsUpdateInterface.h>

namespace mars {

  namespace graphics {
    class GraphicsWindowInterface;
    class GraphicsCameraInterface;
  }

  namespace sim {
 
    class RaySensor;
 

    struct ScanningSonarConfig : public interfaces::BaseConfig{
      ScanningSonarConfig(){
        name = "Unkown Scanning Sonar";
        width= 64;
        height= 512;
        resolution = 0.1; //10cm
        maxDist = 100; //100meter
        hud_pos=0;
        updateRate = 10;
        gain = 1;
        show_cam = false;
        only_ray=false;
        extension = utils::Vector(0.010000, 0.004000, 0.004000);
        ori_offset = utils::eulerToQuaternion(utils::Vector(90, 0, -90)); //All elements should be X Forwart looging to meet rock-convention, so i add this offset for all setting
        pos_offset = utils::Vector(0.02, 0, 0);
        position.setZero();
        orientation.setIdentity();
        attached_node = 0;
        left_limit = M_PI;
        right_limit = -M_PI;
        ping_pong_mode = false;
      }
      unsigned int updateRate;
      unsigned int width;
      unsigned int height;
      unsigned int attached_node;
      utils::Vector pos_offset;
      utils::Vector extension;
      double resolution, maxDist;
      int hud_pos;
      bool show_cam;
      utils::Quaternion ori_offset;
      double gain;
      bool only_ray;
      utils::Vector position;
      utils::Quaternion orientation;
      float left_limit;
      float right_limit;
      bool ping_pong_mode;
    };

    class ScanningSonar : public interfaces::BaseCameraSensor<double>,
                          public interfaces::BaseNodeSensor,
                          public interfaces::SensorInterface,
                          public data_broker::ReceiverInterface,
                          public interfaces::GraphicsUpdateInterface {
    public:
      static interfaces::BaseSensor* instanciate(interfaces::ControlCenter *control,
                                           interfaces::BaseConfig *config );

      ScanningSonar(interfaces::ControlCenter *control, ScanningSonarConfig _config);
      ~ScanningSonar(void);

      virtual int getSensorData(double** data) const;
      virtual void receiveData(const data_broker::DataInfo &info,
                               const data_broker::DataPackage &package,
                               int callbackParam);

      virtual void preGraphicsUpdate(void);

      static interfaces::BaseConfig* parseConfig(interfaces::ControlCenter *control,
                                           utils::ConfigMap *config);
      virtual utils::ConfigMap createConfig();
  
      void setPingPongConfig(bool ping_pong_mode, float left_limit,
                             float right_limit);

    private:
      ScanningSonarConfig config;
      interfaces::GraphicsWindowInterface *gw;
      interfaces::GraphicsCameraInterface* gc;
      long dbPosIndices[3];
      long dbRotIndices[4];
      unsigned long nodeID[2];
      unsigned long jointID[2];
      unsigned long motorID;
      unsigned long rayID;
      unsigned long cam_window_id;
      bool switch_motor_direction;

      utils::Quaternion head_orientation;
      utils::Vector head_position;
      unsigned int attached_motor;
      RaySensor *raySensor;
    };

  } // end of namespace sim
} // end of namespace mars

#endif
