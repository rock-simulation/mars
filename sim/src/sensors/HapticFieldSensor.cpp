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
 *  HapticFieldSensor.cpp
 *  QTVersion
 *
 *  Created by Kai von Szadkowski
 *
 */

#include "HapticFieldSensor.h"
#include <mars/data_broker/DataBrokerInterface.h>
#include <mars/interfaces/sensor_bases.h>
#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>

#include <cstdio>
#include <cstdlib>

namespace mars {
  namespace sim {

    using namespace utils;
    using namespace interfaces;

    BaseSensor* HapticFieldSensor::instanciate(ControlCenter *control, BaseConfig *config) {
      HapticFieldConfig *cfg = dynamic_cast<HapticFieldConfig*>(config);
      assert(cfg);
      return new HapticFieldSensor(control, *cfg);
    }

    BaseConfig* HapticFieldSensor::parseConfig(interfaces::ControlCenter *control,
        utils::ConfigMap *config) {
      HapticFieldConfig *cfg = new HapticFieldConfig;
      cfg->parseConfig(control, config);
      return cfg;
    }

    HapticFieldSensor::HapticFieldSensor(ControlCenter *control, HapticFieldConfig config) :
        SensorInterface(control), BaseGridIntersectionSensor(config.attached_node, config.name,
            config.cols, config.rows, config.stepX, config.stepY, config.maxDistance), config(
            config) {
      attached_node = config.attached_node;
      updateRate = config.updateRate; // FIXME: is this already cared for?
      contactForce = 0.0;
      contact = false;
      contactIndex = -1;
      contactForceIndex = -1;
      fieldwidth = config.cols * config.stepX;
      fieldheight = config.rows * config.stepY;

      control->nodes->addNodeSensor(this); //register sensor with NodePhysics

      // register with DataBroker
      std::string groupName, dataName;
      control->nodes->getDataBrokerNames(attached_node, &groupName, &dataName);
      control->dataBroker->registerTimedReceiver(this, groupName, dataName, "mars_sim/simTimer",
          updateRate);

      position = control->nodes->getPosition(attached_node) + Vector(0, 0, 1);
      orientation = control->nodes->getRotation(attached_node);

      drawStruct draw;
      draw_item item;
      Vector tmp;
      Vector offset;
      haveUpdate = false;
      for (int i = 0; i < 3; ++i)
        positionIndices[i] = -1;
      for (int i = 0; i < 4; ++i)
        rotationIndices[i] = -1;
      //Drawing Stuff (taken from RaySensor)
      if (config.drawRays) {
        draw.ptr_draw = (DrawInterface*) this;
        item.id = 0;
        item.type = DRAW_LINE;
        item.draw_state = DRAW_STATE_CREATE;
        item.point_size = 1;
        item.myColor.r = 1;
        item.myColor.g = 0;
        item.myColor.b = 0;
        item.myColor.a = 1;
        item.texture = "";
        item.t_width = item.t_height = 0;
        item.get_light = 0.0;

        for (int c = 0; c < config.cols; c++) {
          for (int r = 0; r < config.rows; r++) {
            tmp = (orientation * Vector(0, 0, 1));
            offset = Vector(-0.5 * fieldwidth + c * config.stepX, -0.5 * fieldheight + r * config.stepY, 0);
            sensorpoints.push_back(tmp);
            item.start = position + offset;
            item.end = (orientation * tmp);
            item.end *= data[c + (config.cols * r)];
            draw.drawItems.push_back(item); // TEST
          }
        }

        if (control->graphics)
          control->graphics->addDrawItems(&draw);
      }
    }

    HapticFieldSensor::~HapticFieldSensor(void) {
      control->graphics->removeDrawItems((DrawInterface*) this);
      control->dataBroker->unregisterTimedReceiver(this, "*", "*", "mars_sim/simTimer");
    }

    int HapticFieldSensor::getAsciiData(char* data) const {
      sReal contact = 0;
      std::vector<double>::const_iterator iter;

      for (iter = forces.begin(); iter != forces.end(); iter++) {
        contact += *iter;
      }
      sprintf(data, " %9.3f", contact);
      return 10;
    }

    int HapticFieldSensor::getSensorData(sReal** data) const {
      sReal contact = 0;
      std::vector<double>::const_iterator iter;

      *data = (sReal*) malloc(sizeof(sReal));
      for (iter = forces.begin(); iter != forces.end(); iter++) {
        contact += *iter;
        ;
      }
      **data = contact;
      return 1;
    }

    void HapticFieldSensor::receiveData(const data_broker::DataInfo &info,
        const data_broker::DataPackage &package, int callbackParam) {
      if (contactForceIndex == -1) {
        contactForceIndex = package.getIndexByName("contactForce");
        contactIndex = package.getIndexByName("contact");
      }
      package.get(contactForceIndex, &contactForce);
      package.get(contact, &contact);
      if (contact) {
        computeForces();
      } else {
        for (std::vector<double>::iterator it = forces.begin(); it < forces.end(); ++it) {
          *it = 0;
        }
      }

      if (positionIndices[0] == -1) {
        positionIndices[0] = package.getIndexByName("position/x");
        positionIndices[1] = package.getIndexByName("position/y");
        positionIndices[2] = package.getIndexByName("position/z");
        rotationIndices[0] = package.getIndexByName("rotation/x");
        rotationIndices[1] = package.getIndexByName("rotation/y");
        rotationIndices[2] = package.getIndexByName("rotation/z");
        rotationIndices[3] = package.getIndexByName("rotation/w");
      }
      for (int i = 0; i < 3; ++i) {
        package.get(positionIndices[i], &position[i]);
      }

      package.get(rotationIndices[0], &orientation.x());
      package.get(rotationIndices[1], &orientation.y());
      package.get(rotationIndices[2], &orientation.z());
      package.get(rotationIndices[3], &orientation.w());

      haveUpdate = true;
    }

    void HapticFieldSensor::update(std::vector<draw_item>* drawItems) {
      unsigned int i;
      if (config.drawRays) {
        if (haveUpdate) {
          control->nodes->updateRay(attached_node);
          haveUpdate = false;
        }

        fprintf(stderr, "HapticFieldSensor::drawItems: %i\n", drawItems->size());
        Vector offset;
        if (!(*drawItems)[0].draw_state) {
          for (int c = 0; c < config.cols; c++) {
            for (int r = 0; r < config.rows; r++) {
              offset = Vector(-0.5 * fieldwidth + c * config.stepX, -0.5 * fieldheight + r * config.stepY, 0);
              i = c + (config.cols * r);
              (*drawItems)[i].draw_state = DRAW_STATE_UPDATE;
              (*drawItems)[i].start = position + offset;
              (*drawItems)[i].end = (orientation * sensorpoints[i]);
              (*drawItems)[i].end *= data[i];

              (*drawItems)[i].end += (*drawItems)[i].start;
            }
          }
        }
      }
      //      Vector tmp, lpos;
      //
      //      for (int i = 0; i < s_cfg.resolution; i++) {
      //        lpos = s_cfg.sensorPositions[i];
      //        //tmp = QVRotate(this->rot, lpos);
      //        tmp = (this->rot * lpos);
      //        (*drawItems)[i].draw_state = DRAW_STATE_UPDATE;
      //        (*drawItems)[i].start = Vector(
      //            tmp.x() + pos.x(),
      //            tmp.y() + pos.y(),
      //            tmp.z() + pos.z());
      //        lpos.z() = -s_cfg.data[i];
      //        //tmp = QVRotate(this->rot, lpos);
      //        tmp = (this->rot * lpos);
      //        (*drawItems)[i].end = Vector(
      //            tmp.x() + pos.x(),
      //            tmp.y() + pos.y(),
      //            tmp.z() + pos.z());
      //      }
    }

    void HapticFieldSensor::computeForces() {
      // FIXME: add mutex here?
      double distanceSum = 0;
      for (std::vector<double>::iterator it = data.begin(); it < data.end(); ++it) {
        *it = maxDistance - *it;
        distanceSum += *it;
      }
      double forceQuant = contactForce / distanceSum;
      for (int i = 0; i < forces.size(); ++i) {
        forces.at(i) = data.at(i) * forceQuant;
      }
    }

  } // end of namespace sim
} // end of namespace mars
