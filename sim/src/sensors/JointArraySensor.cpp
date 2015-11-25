/*
 *  Copyright 2012, DFKI GmbH Robotics Innovation Center
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
 *  JointArraySensor.cpp
 *  QTVersion
 *
 *  Created by Malte Römmerann
 *
 */

#include "JointArraySensor.h"

#include <mars/interfaces/sim/JointManagerInterface.h>
#include <mars/interfaces/sim/SimulatorInterface.h>

#include <mars/data_broker/DataBrokerInterface.h>

#include <cstdlib>
#include <cstdio>

namespace mars {
  namespace sim {

    using namespace utils;
    using namespace configmaps;
    using namespace interfaces;

    BaseConfig* JointArraySensor::parseConfig(ControlCenter *control,
                                              ConfigMap *config) {
      IDListConfig *cfg = new IDListConfig;
      cfg->parseConfig(control, config, interfaces::MAP_TYPE_JOINT);
      return cfg;
    }

    ConfigMap JointArraySensor::createConfig() const {
      std::vector<unsigned long>::const_iterator it;
      ConfigMap cfg;

      cfg["name"] = config.name;
      cfg["type"] = typeName;
      cfg["index"] = config.id;
      cfg["rate"] = config.updateRate;

      for(it=config.ids.begin(); it!= config.ids.end(); ++it) {
        cfg["id"] << *it;
      }

      return cfg;
    }


    JointArraySensor::JointArraySensor(ControlCenter *control, IDListConfig config,
                                       bool initArray):
      SensorInterface(control),
      BaseSensor(config.id, config.name),
      typeName("unknown type"), config(config) {
  
      updateRate = config.updateRate;
      countIDs = 0;
      std::vector<unsigned long>::iterator it;
      std::string groupName, dataName;

      for(it=config.ids.begin(); it!=config.ids.end(); ++it) {
        control->joints->getDataBrokerNames(*it, &groupName, &dataName);
        control->dataBroker->registerTimedReceiver(this, groupName, dataName,
                                                   "mars_sim/simTimer",
                                                   updateRate,
                                                   countIDs++);
        if(initArray) doubleArray.push_back(0.0);
      }
    }

    JointArraySensor::~JointArraySensor(void) {
      control->dataBroker->unregisterTimedReceiver(this, "*", "*",
                                                   "mars_sim/simTimer");
    }

    // this function should be overwritten by the special sensor to
    int JointArraySensor::getAsciiData(char* data) const {
      char *p;
      int num_char = 0;

      std::vector<double>::const_iterator iter;

      p = data;
      for(iter = doubleArray.begin(); iter != doubleArray.end(); iter++) {
        sprintf(p, " %10.7f", *iter);
        num_char += 11;
        p += 11;
      }
      return num_char;
    }

    int JointArraySensor::getSensorData(sReal** data) const {
      std::vector<double>::const_iterator iter;
      int i=0;

      *data = (sReal*)calloc(doubleArray.size(), sizeof(sReal));
      for(iter = doubleArray.begin(); iter != doubleArray.end(); iter++) {
        (*data)[i++] = *iter;
      }

      return i;
    }

  } // end of namespace sim
} // end of namespace mars
