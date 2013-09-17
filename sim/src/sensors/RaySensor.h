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
 *  RaySensor.h
 *  QTVersion
 *
 *  Created by Malte Roemmermann
 *
 */

#ifndef RAYSENSOR_H
#define RAYSENSOR_H

#ifdef _PRINT_HEADER_
#warning "RaySensor.h"
#endif

#include <mars/interfaces/sim/SensorInterface.h>
#include <mars/data_broker/ReceiverInterface.h>
#include <mars/utils/Vector.h>
#include <mars/utils/Quaternion.h>
#include <mars/interfaces/graphics/draw_structs.h>
#include <mars/interfaces/graphics/GraphicsUpdateInterface.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>

namespace mars {
  namespace sim {

    class RayConfig : public interfaces::BaseConfig{
    public:
      RayConfig(){
        name = "Unknown RaySensor";
        width=1;
        height=1;
        pos_offset.setZero();
        ori_offset.setIdentity();
        opening_width=0.5*M_PI;
        opening_height=0;
        attached_node = 0;
        maxDistance = 100.0;
        draw_rays = true;
      }

      unsigned long attached_node;
      int width;
      int height;
      utils::Vector pos_offset;
      utils::Quaternion ori_offset;
      double opening_width;
      double opening_height;
      double maxDistance;
      bool draw_rays;
    };

    class RaySensor : 
      public interfaces::BaseNodeSensor,
      public interfaces::SensorInterface, 
      public data_broker::ReceiverInterface,
      public interfaces::DrawInterface,
      public interfaces::GraphicsUpdateInterface {

    public:
      static interfaces::BaseSensor* instanciate(interfaces::ControlCenter *control,
                                                 interfaces::BaseConfig* config);
      RaySensor(interfaces::ControlCenter *control, RayConfig config);
      ~RaySensor(void);
  
      std::vector<double> getSensorData() const; 
      virtual void receiveData(const data_broker::DataInfo &info,
                               const data_broker::DataPackage &package,
                               int callbackParam);
      virtual void update(std::vector<interfaces::draw_item>* drawItems);
      
      virtual void preGraphicsUpdate(void );

      static interfaces::BaseConfig* parseConfig(interfaces::ControlCenter *control,
                                                 utils::ConfigMap *config);
      virtual utils::ConfigMap createConfig() const;

      const RayConfig& getConfig() const;

    private:
	struct RaySubSensor
	{
	    long cam_window_id;
	    interfaces::GraphicsWindowInterface *gw;
	    interfaces::GraphicsCameraInterface *gc;
	    std::vector<float> depthBuffer;
	    int rttWidth;
	    int rttHeight;
	    double coveredAngle;
	    utils::Quaternion orientation;
	};
	
	std::vector<RaySubSensor> subSensors;
	RayConfig config;
	utils::Vector position;
	utils::Quaternion orientation;
	
	int overallRttWidth;
	std::vector<double> rayValues;
	
	std::vector<utils::Vector> directions;
	long positionIndices[3];
	long rotationIndices[4];
    };

  } // end of namespace sim
} // end of namespace mars

#endif
