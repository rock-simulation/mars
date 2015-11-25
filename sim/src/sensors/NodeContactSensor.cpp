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
 *  NodeContactSensor.cpp
 *
 *  Created by Malte Römmerann
 *
 */

#include "NodeContactSensor.h"
#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/data_broker/DataBrokerInterface.h>

#include <cstdio>
#include <cstdlib>

namespace mars {
  namespace sim {

    using namespace utils;
    using namespace configmaps;
    using namespace interfaces;

    BaseSensor* NodeContactSensor::instanciate(ControlCenter *control,
                                               BaseConfig *config ){

      IDListConfig *cfg = dynamic_cast<IDListConfig*>(config);
      assert(cfg);
      return new NodeContactSensor(control,*cfg);
    }

    BaseConfig* NodeContactSensor::parseConfig(ControlCenter *control,
                                               ConfigMap *config) {
      IDListConfig *cfg = new IDListConfig;
      cfg->parseConfig(control, config, interfaces::MAP_TYPE_NODE);
      return cfg;
    }

    ConfigMap NodeContactSensor::createConfig() const {
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

    NodeContactSensor::NodeContactSensor(ControlCenter *control,
                                         IDListConfig config):
      SensorInterface(control),
      BaseNodeSensor(config.id, config.name),
      config(config) {

      std::vector<unsigned long>::iterator it;
      std::string groupName, dataName;
      groundContactIndex = -1;
      countIDs = 0;
      updateRate = config.updateRate;

      for(it=config.ids.begin(); it!=config.ids.end(); ++it) {
        control->nodes->getDataBrokerNames(*it, &groupName, &dataName);
        control->dataBroker->registerTimedReceiver(this, groupName, dataName,
                                                   "mars_sim/simTimer",
                                                   updateRate,
                                                   countIDs++);
        values.push_back(false);
      }
      typeName = "NodeContact";
    }

    NodeContactSensor::~NodeContactSensor(void) {
      control->dataBroker->unregisterTimedReceiver(this, "*", "*", 
                                                   "mars_sim/simTimer");
    }

    // this function should be overwritten by the special sensor to
    int NodeContactSensor::getAsciiData(char* data) const {
      bool contact = 0;
      std::vector<bool>::const_iterator iter;
  
      for(iter = values.begin(); iter != values.end(); iter++) {
        contact |= *iter;
      }
      sprintf(data, " %1d", contact);
      return 2;
    }

    int NodeContactSensor::getSensorData(sReal** data) const {
      bool contact = 0;
      std::vector<bool>::const_iterator iter;
  
      *data = (sReal*)malloc(sizeof(sReal));
      for(iter = values.begin(); iter != values.end(); iter++) {
        contact |= *iter;
      }
      **data = contact;
      return 1;
    }


    void NodeContactSensor::receiveData(const data_broker::DataInfo &info,
                                        const data_broker::DataPackage &package,
                                        int callbackParam) {
      if(groundContactIndex == -1) {
        groundContactIndex = package.getIndexByName("contact");
      }
      bool value;
      package.get(groundContactIndex, &value);
      values[callbackParam] = value;
    }

  } // end of namespace sim
} // end of namespace mars
