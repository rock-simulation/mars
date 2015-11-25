/*
 *  Copyright 2014, DFKI GmbH Robotics Innovation Center
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
 *  HapticFieldSensor.h
 *
 *  Created by Kai von Szadkowski
 *
 */

#ifndef HAPTICFIELDSENSOR_H
#define HAPTICFIELDSENSOR_H

#ifdef _PRINT_HEADER_
#warning "HapticFieldSensor.h"
#endif

#include <mars/interfaces/sim/SensorInterface.h>
#include <mars/data_broker/ProducerInterface.h>
#include <mars/data_broker/ReceiverInterface.h>
#include <mars/interfaces/sensor_bases.h>
#include <mars/interfaces/graphics/draw_structs.h>
#include <mars/interfaces/sim/LoadCenter.h>

namespace mars {
  namespace sim {

    class HapticFieldConfig : public interfaces::BaseConfig{
    public:
      HapticFieldConfig(){
        name = "haptic field";
        attached_node=0;
        cols=0;
        rows=0;
        stepX=0.0;
        stepY=0.0;
        maxDistance=0.0;
        drawRays=false;
      }

      void parseConfig(interfaces::ControlCenter *control, configmaps::ConfigMap *config) {
        unsigned int mapIndex = (*config)["mapIndex"];
        name = (std::string)(*config)["name"];
        updateRate = (*config)["rate"];
        cols = (*config)["cols"];
        rows = (*config)["rows"];
        stepX = (*config)["stepX"];
        stepY = (*config)["stepY"];
        maxDistance = (*config)["maxDistance"];
        drawRays = (*config)["drawRays"];
        attached_node = control->loadCenter->getMappedID((*config)["attached_node"],
            interfaces::MAP_TYPE_NODE, mapIndex);
      }

      unsigned long attached_node; // id of the node representing the base collision object
      int cols; // cols of the sensor
      int rows; // rows of the sensor
      double stepX; // distance between cols
      double stepY; // distance between rows
      double maxDistance; // length of the rays from the bottom of the node
      bool drawRays;
    };

    class HapticFieldSensor: public interfaces::SensorInterface,
        public interfaces::BaseGridIntersectionSensor,
        public data_broker::ProducerInterface,
        public data_broker::ReceiverInterface,
        public interfaces::DrawInterface {

    public:
      HapticFieldSensor(interfaces::ControlCenter *control, HapticFieldConfig config);
      ~HapticFieldSensor();

      virtual int getAsciiData(char* data) const;
      virtual int getSensorData(interfaces::sReal** data) const;
      virtual void receiveData(const data_broker::DataInfo &info,
          const data_broker::DataPackage &package, int callbackParam);
      virtual void produceData(const data_broker::DataInfo &info,
                                     data_broker::DataPackage *package,
                                     int callbackParam);

      virtual void update(std::vector<interfaces::draw_item>* drawItems);

      static interfaces::BaseConfig* parseConfig(interfaces::ControlCenter *control,
          configmaps::ConfigMap *config);
      static interfaces::BaseSensor* instanciate(interfaces::ControlCenter *control,
          interfaces::BaseConfig *config);

    private:

      void computeForces();
      //std::map<unsigned long, double> contact_forces; // <id of node in contact, force exerted by said node>
      interfaces::drawStruct draw;
      int contactForceIndex, contactIndex;
      double contactForce;
      bool contact;
      bool haveUpdate;
      long positionIndices[3];
      long rotationIndices[4];
      std::vector<utils::Vector> sensorpoints;
      utils::Vector ray;
      std::vector<double> forces;
      std::vector<double> weights;
      double fieldwidth, fieldheight;
      HapticFieldConfig config;
      data_broker::DataPackage dbPackage;
      unsigned long dbPushId;
    };

  } // end of namespace sim
} // end of namespace mars

#endif
