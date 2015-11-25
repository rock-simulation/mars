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
 *  MotorCurrrentSensor.cpp
 *  QTVersion
 *
 *  Created by Malte Römmerann
 *
 */

#include "MotorCurrentSensor.h"
#include <mars/interfaces/sim/MotorManagerInterface.h>
#include <mars/data_broker/DataBrokerInterface.h>

#include <cstdio>
#include <cstdlib>

namespace mars {
  namespace sim {

    using namespace configmaps;
    using namespace interfaces;

    BaseSensor* MotorCurrentSensor::instanciate(ControlCenter *control,
                                                BaseConfig *config ){

      IDListConfig *cfg = dynamic_cast<IDListConfig*>(config);
      assert(cfg);
      return new MotorCurrentSensor(control,*cfg);
    }

    BaseConfig* MotorCurrentSensor::parseConfig(ControlCenter *control,
                                                ConfigMap *config) {
      IDListConfig *cfg = new IDListConfig;
      cfg->parseConfig(control, config, interfaces::MAP_TYPE_MOTOR);
      return cfg;
    }

    MotorCurrentSensor::MotorCurrentSensor(ControlCenter *control,
                                           IDListConfig config):
      SensorInterface(control),
      BaseSensor(config.id, config.name),
      config(config), typeName("MotorCurrent") {

      updateRate = config.updateRate;

      countIDs = 0;
      std::vector<unsigned long>::iterator it;
      std::string groupName, dataName;

      for(it=config.ids.begin(); it!=config.ids.end(); ++it) {
        control->motors->getDataBrokerNames(*it, &groupName, &dataName);
        control->dataBroker->registerTimedReceiver(this, groupName, dataName,
                                                   "mars_sim/simTimer",
                                                   updateRate,
                                                   countIDs++);
        doubleArray.push_back(0.0);
      }
      dbCurrentIndex = -1;
    }

    MotorCurrentSensor::~MotorCurrentSensor(void) {
      control->dataBroker->unregisterTimedReceiver(this, "*", "*",
                                                   "mars_sim/simTimer");
    }

    // this function should be overwritten by the special sensor to
    int MotorCurrentSensor::getAsciiData(char* data) const {
      char *p;
      int num_char = 0;
      std::vector<double>::const_iterator iter;

      p = data;

      for(iter = doubleArray.begin(); iter != doubleArray.end(); iter++) {
        sprintf(p, " %6.3f", *iter);
        num_char += 7;
        p += 7;
      }
      return num_char;
    }

    int MotorCurrentSensor::getSensorData(sReal** data) const {
      std::vector<double>::const_iterator iter;
      int i=0;

      *data = (sReal*)calloc(doubleArray.size(), sizeof(sReal));
      for(iter = doubleArray.begin(); iter != doubleArray.end(); iter++) {
        (*data)[i++] = *iter;
      }
      return i;
    }


    void MotorCurrentSensor::receiveData(const data_broker::DataInfo &info,
                                         const data_broker::DataPackage &package,
                                         int callbackParam) {
      if(dbCurrentIndex == -1) {
        dbCurrentIndex = package.getIndexByName("current");
      }
      package.get(dbCurrentIndex, &doubleArray[callbackParam]);
    }

    ConfigMap MotorCurrentSensor::createConfig() const {
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

  } // end of namespace sim
} // end of namespace mars
