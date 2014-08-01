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

/*
 *  RotatingRaySensor.h
 *
 *  Created by Malte Langosz, Kai von Szadkowski
 *
 */

#ifndef ROTATINGRAYSENSOR_H
#define ROTATINGRAYSENSOR_H

#ifdef _PRINT_HEADER_
#warning "RotatingRaySensor.h"
#endif

#include <mars/interfaces/sim/SensorInterface.h>
#include <mars/data_broker/ReceiverInterface.h>
#include <mars/utils/Vector.h>
#include <mars/utils/Quaternion.h>
#include <mars/utils/mathUtils.h>
#include <mars/utils/Mutex.h>
#include <mars/interfaces/graphics/draw_structs.h>

namespace mars {
  namespace sim {

    class RotatingRayConfig : public interfaces::BaseConfig{
    public:
      RotatingRayConfig(){
        name = "Unknown RaySensor";
        bands=5; //number of horizontal replicates of vertical laser bands
        lasers=32; //number of lasers in vertical dimension
        increment =1; // how many lasers are skipped vertically with every horizontal step
        pos_offset.setZero();
        ori_offset.setIdentity();
        opening_width=2*M_PI; //this means we cover the entire 360 degrees
        opening_height=40.0/360.0*2.0*M_PI; //
        downtilt = 10/360*2*M_PI; //how many rads the rays of the sensor is tilted downwards
        attached_node = 0;
        maxDistance = 100.0;
        turning_speed = 1; //turning speed in Hz
        draw_rays = true;
        subresolution = 1; //factor to increase point cloud resolution through multiple scans
      }

      unsigned long attached_node;
      int bands;
      int lasers;
      utils::Vector pos_offset;
      utils::Quaternion ori_offset;
      double opening_width;
      double opening_height;
      double downtilt;
      double maxDistance;
      double turning_speed;
      int subresolution;
      int increment;
      bool draw_rays;
    };

    class RotatingRaySensor :
      public interfaces::BasePolarIntersectionSensor , 
      public interfaces::SensorInterface, 
      public data_broker::ReceiverInterface,
      public interfaces::DrawInterface {

    public:
      static interfaces::BaseSensor* instanciate(interfaces::ControlCenter *control,
                                                 interfaces::BaseConfig* config);
      RotatingRaySensor(interfaces::ControlCenter *control, RotatingRayConfig config);
      ~RotatingRaySensor(void);
  
      std::vector<double> getSensorData() const; 
      std::vector<double> getPointCloud();
      int getSensorData(double**) const; 
      virtual void receiveData(const data_broker::DataInfo &info,
                               const data_broker::DataPackage &package,
                               int callbackParam);
      virtual void update(std::vector<interfaces::draw_item>* drawItems);

      static interfaces::BaseConfig* parseConfig(interfaces::ControlCenter *control,
                                                 utils::ConfigMap *config);
      virtual utils::ConfigMap createConfig() const;

      const RotatingRayConfig& getConfig() const;

      double turn();
      int getNRays();
      RotatingRayConfig config;

    private:
      std::vector<utils::Vector> directions;
      std::list<double> pointcloud;
      bool have_update;
      bool full_scan;
      double turning_offset;
      utils::Quaternion orientation_offset;
      long positionIndices[3];
      long rotationIndices[4];
      double turning_step;
      int nsamples;
      mars::utils::Mutex mutex_data;
    };

  } // end of namespace sim
} // end of namespace mars

#endif
