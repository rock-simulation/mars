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
 *  NodeArraySensor.cpp
 *  QTVersion
 *
 *  Created by Malte Römmerann
 *
 */

#include "NodeArraySensor.h"

#include <mars/interfaces/sim/SimulatorInterface.h>
#include <mars/interfaces/sim/NodeManagerInterface.h>

#include <mars/data_broker/DataBrokerInterface.h>

#include <cstdlib>
#include <cstdio>

namespace mars {
  namespace sim {

    using namespace utils;
    using namespace configmaps;
    using namespace interfaces;

    BaseConfig* NodeArraySensor::parseConfig(ControlCenter *control,
                                             ConfigMap *config) {
      IDListConfig *cfg = new IDListConfig;
      cfg->parseConfig(control, config, interfaces::MAP_TYPE_NODE);
      return cfg;
    }

    ConfigMap NodeArraySensor::createConfig() const {
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


    NodeArraySensor::NodeArraySensor(ControlCenter *control, IDListConfig config,
                                     bool initArray, bool registerReceiver):
      SensorInterface(control),
      BaseSensor(config.id, config.name),
      typeName("unknown type"), config(config) {
  
      updateRate = config.updateRate;
      if(registerReceiver) {
        countIDs = 0;
        std::vector<unsigned long>::iterator it;
        std::string groupName, dataName;

        for(it=config.ids.begin(); it!=config.ids.end(); ++it) {
          control->nodes->getDataBrokerNames(*it, &groupName, &dataName);
          if(control->dataBroker) {
            control->dataBroker->registerTimedReceiver(this, groupName,
                                                       dataName,
                                                       "mars_sim/simTimer",
                                                       updateRate,
                                                       countIDs++);
          }
          if(initArray) doubleArray.push_back(0.0);
        }
      }
      else {
        countIDs = config.ids.size();
        if(initArray) {
          for(int i=0; i<countIDs; ++i) doubleArray.push_back(0.0);
        }
      }
    }

    NodeArraySensor::~NodeArraySensor(void) {
      if(control->dataBroker) {
        control->dataBroker->unregisterTimedReceiver(this, "*", "*",
                                                     "mars_sim/simTimer");
      }
    }

    // this function should be overwritten by the special sensor to
    int NodeArraySensor::getAsciiData(char* data) const {
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

    int NodeArraySensor::getSensorData(sReal** data) const {
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
