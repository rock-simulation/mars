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
 *  Created by Stefan Haase, Kai von Szadkowski, Malte Langosz
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

#include <base/Time.hpp>

namespace mars {
  namespace sim {

    class RotatingRayConfig : public interfaces::BaseConfig{
    public:
      RotatingRayConfig(){
        name = "Unknown RaySensor";
        bands=16; //number of horizontal replicates of vertical laser bands
        lasers=32; //number of lasers in vertical dimension
        pos_offset.setZero();
        ori_offset.setIdentity();
        opening_width=2*M_PI; //this means we cover the entire 360 degrees
        opening_height= (40.0/180.0)*M_PI; //
        downtilt = (10.67/180.0)*M_PI; //how many rads the rays of the sensor is tilted downwards
        attached_node = 0;
        maxDistance = 100.0;
        draw_rays = true;
        horizontal_resolution = (1.0/180.0)*M_PI;
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
      bool draw_rays;
      double horizontal_resolution;
    };

    class RotatingRaySensor :
      public interfaces::BasePolarIntersectionSensor, //->BaseArraySensor ->BaseNodeSensor->BaseSensor
      public interfaces::SensorInterface, // Stores the ControlCenter* control pointer.
      public data_broker::ReceiverInterface,
      public interfaces::DrawInterface {

    public:
      static interfaces::BaseSensor* instanciate(interfaces::ControlCenter *control,
                                                 interfaces::BaseConfig* config);
      RotatingRaySensor(interfaces::ControlCenter *control, RotatingRayConfig config);
      ~RotatingRaySensor(void);
      
      /**
       * Returns a complete 360 degree scan and clears the pointcloud afterwards.
       */
      std::vector<utils::Vector> getPointcloud();
      
      /** 
       * Copies the current full pointcloud to a double array with (x,y,z).
       * \warning Memory has to be freed manually!
       * Inherited from BaseSensor, implemented from BasePolarIntersectionSensor.
       */
      int getSensorData(double**) const; 
      
      /**
       * Receives the measured distances, calculates the vectors in the local
       * sensor frame and transfers them into the world to compensate the
       * movement during pointcloud gathering.
       * The points are transformed back to the current node pose
       * when the pointcloud is requested.
       * Inherited from ReceiverInterface. Method is called by the DataBroker
       * as soon as the registered event occurs.
       */
      virtual void receiveData(const data_broker::DataInfo &info,
                               const data_broker::DataPackage &package,
                               int callbackParam);
      
      /**
       * Uses the current node pose and the current distances to draw 
       * the laser rays.
       * Inherited from DrawInterface.
       */
      virtual void update(std::vector<interfaces::draw_item>* drawItems);
      
      /**
       * Config methods all part of BaseSensor.
       */
      static interfaces::BaseConfig* parseConfig(interfaces::ControlCenter *control,
                                                 utils::ConfigMap *config);
      virtual utils::ConfigMap createConfig() const;

      const RotatingRayConfig& getConfig() const;

      /**
       * Turns the sensor during each simulation step.
       * As soon as a full scan has been done (depends on the number of bands)
       * the pointcloud is copied to pointcloud_full and a new scan
       * is initiated. Runs in the same thread than receiveData, so only the use of 
       * full_pointcloud (turn(), getPointcloud() and getSensorData()) has to be 
       * synchronized.
       */
      utils::Quaternion turn();
      
      /** Number of lasers * number of bands. */
      int getNumberRays();
      
      /**
       * Returns all the ray directions as normalized vectors.
       */
      inline std::vector<utils::Vector>& getDirections() {
        return directions;
      }
      
      RotatingRayConfig config;

    private:
      /** Contains the normalized scan directions. */ 
      std::vector<utils::Vector> directions;
      std::list<utils::Vector> pointcloud; // TODO Replace with array with fix size.
      std::vector<utils::Vector> pointcloud_full; // Stores the full scan.
      bool have_update;
      bool full_scan;
      double turning_offset;
      double turning_start_fullscan; // Defines the start of the next full scan.
      double turning_end_fullscan; // Defines the end of the next full scan.  
      utils::Quaternion orientation_offset;
      long positionIndices[3];
      long rotationIndices[4];
      double turning_step;
      int nsamples;
      mutable mars::utils::Mutex mutex_pointcloud;
      Eigen::Affine3d current_pose;
      
      unsigned int num_points;
      base::Time time_start;
    };

  } // end of namespace sim
} // end of namespace mars

#endif