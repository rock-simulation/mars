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
#include <mars/utils/Thread.h>
#include <mars/utils/mathUtils.h>
#include <mars/utils/Mutex.h>
#include <mars/interfaces/graphics/draw_structs.h>

#include <base/Pose.hpp>

namespace mars {
  namespace sim {

    class RotatingRayConfig : public interfaces::BaseConfig {
    public:
      RotatingRayConfig(){
        name = "Unknown RaySensor";
        bands=16; //number of horizontal replicates of vertical laser bands
        lasers=32; //number of lasers in vertical dimension
        pos_offset.setZero();
        ori_offset.setIdentity();
        opening_width=2*M_PI; //this means we cover the entire 360 degrees
        opening_height= (40.0/180.0)*M_PI;
        attached_node = 0;
        minDistance = 0.01;
        maxDistance = 100.0;
        draw_rays = true;
        horizontal_resolution = (1.0/180.0)*M_PI;
        transf_sensor_rot_to_sensor.setIdentity();
        horizontal_offset = 0.0;
        vertical_offset = (10.67/180.0)*M_PI;
      }

      unsigned long attached_node;
      int bands; //number of horizontal replicates of vertical laser bands
      int lasers; //number of lasers in vertical dimension
      utils::Vector pos_offset;
      utils::Quaternion ori_offset;
      double opening_width; //this means we cover the entire 360 degrees
      double opening_height;
      double horizontal_offset; // allows to shift the bands horizontally
      double vertical_offset; // allows to shift the lasers vertically
      double minDistance;
      double maxDistance;
      bool draw_rays;
      double horizontal_resolution;
      // Describes the orientation of the sensor in the unturned sensor frame. 
      // Can be used compensate the node orientation / to define the turning axis.
      // Pass the node orientation to receive an unturned sensor.
      // This transformation is only applied to the pointcloud.
      utils::Quaternion transf_sensor_rot_to_sensor;
    };

    class RotatingRaySensor :
      public interfaces::BasePolarIntersectionSensor, //->BaseArraySensor ->BaseNodeSensor->BaseSensor
      public interfaces::SensorInterface, // Stores the ControlCenter* control pointer.
      public data_broker::ReceiverInterface,
      public interfaces::DrawInterface,
      utils::Thread {

    public:
      static interfaces::BaseSensor* instanciate(interfaces::ControlCenter *control,
                                                 interfaces::BaseConfig* config);
      RotatingRaySensor(interfaces::ControlCenter *control, RotatingRayConfig config);
      ~RotatingRaySensor(void);
      
      /**
       * Returns a complete scan covering the complete defined horizontal range.
       * The pointcloud is gathered within the world frame and
       * - after a complete scan has been received - transformed into the
       * current local frame. This prevents strong distortions on slower computers.
       * If a full scan is not available an empty pointcloud will be returned.
       */
      bool getPointcloud(std::vector<utils::Vector>& pointcloud);

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
                                                 configmaps::ConfigMap *config);
      
      virtual configmaps::ConfigMap createConfig() const;

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

    protected:
      void run();

    private:
      /** Contains the normalized scan directions. */ 
      std::vector<utils::Vector> directions;
      // TODO Storing the pointcloud four times is not very effective.
      // Maybe: Integrate distortion-prevention (use current sensor pose)
      // in mlls and only create the pointcloud on demand.
      std::list<utils::Vector> pointcloud1; // TODO Replace with array with fix size.
      std::list<utils::Vector> pointcloud2; // TODO Replace with array with fix size.
      std::list<utils::Vector> *toCloud, *fromCloud; // TODO Replace with array with fix size.
      std::vector<utils::Vector> pointcloud_full; // Stores the full scan.
      bool convertPointCloud;
      int nextCloud;
      double vertical_resolution;
      bool update_available;
      bool full_scan;
      double turning_offset;
      double turning_end_fullscan; // Defines the upper border for the turning_offset. 
      utils::Quaternion orientation_offset; // Used to turn the sensor during each simulation step.
      long positionIndices[3];
      long rotationIndices[4];
      double turning_step;
      int nsamples;
      mutable mars::utils::Mutex mutex_pointcloud, poseMutex;
      Eigen::Affine3d current_pose;
      bool closeThread;
      unsigned int num_points;
    };

  } // end of namespace sim
} // end of namespace mars

#endif
