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
 *  JointLoadSensor.cpp
 *  QTVersion
 *
 *  Created by Malte Römmerann
 *
 */

#include <cstdio>

#include "JointLoadSensor.h"

namespace mars {
  namespace sim {

    using namespace utils;
    using namespace interfaces;

    BaseSensor* JointLoadSensor::instanciate(ControlCenter *control,
                                             BaseConfig *config ){

      IDListConfig *cfg = dynamic_cast<IDListConfig*>(config);
      assert(cfg);
      return new JointLoadSensor(control,*cfg);
    }

    JointLoadSensor::JointLoadSensor(ControlCenter *control, IDListConfig config):
      JointArraySensor(control, config) {

      typeName = "JointLoad";
      loadIndices[0] = -1;
      dbPackage.add("id", (long)config.id);
      dbPackage.add("load", 0.0);
      char text[55];
      sprintf(text, "Sensors/Load_%05lu", config.id);
      control->dataBroker->pushData("mars_sim", text,
                                    dbPackage, NULL,
                                    data_broker::DATA_PACKAGE_READ_FLAG);
      control->dataBroker->registerTimedProducer(this, "mars_sim", text,
                                                 "mars_sim/simTimer", 0);
    }

    JointLoadSensor::~JointLoadSensor(void) {
      control->dataBroker->unregisterTimedProducer(this, "*", "*",
                                                   "mars_sim/simTimer");
    }

    // this function should be overwritten by the special sensor to
    int JointLoadSensor::getAsciiData(char* data) const {
      char *p;
      sReal load = 0;
      std::vector<double>::const_iterator iter;
  
      p = data;
      for(iter = doubleArray.begin(); iter != doubleArray.end(); iter++) {
        load += *iter;
      }
      load /= doubleArray.size();
      sprintf(p, " %6.2f", load);
      return 7;
    }

    int JointLoadSensor::getSensorData(sReal** data) const {
      std::vector<double>::const_iterator iter;

      *data = (sReal*)malloc(sizeof(sReal));
      **data = 0;
      for(iter = doubleArray.begin(); iter != doubleArray.end(); iter++) {
        **data += *iter;
      }
      **data /= doubleArray.size();
      return 1;
    }

    void JointLoadSensor::produceData(const data_broker::DataInfo &info,
                                      data_broker::DataPackage *dbPackage,
                                      int callbackParam) {
      (void)callbackParam;
      (void)info;
      sReal load = 0;
      std::vector<double>::const_iterator iter;

      if(doubleArray.size() == 0) {
        dbPackage->set(0, (long)id);
        dbPackage->set(1, load);
        return;
      }
      for(iter = doubleArray.begin(); iter != doubleArray.end(); iter++) {
        load += *iter;
      }
      load /= doubleArray.size();
      dbPackage->set(0, (long)id);
      dbPackage->set(1, load);
    }

    void JointLoadSensor::receiveData(const data_broker::DataInfo &info,
                                      const data_broker::DataPackage &package,
                                      int callbackParam) {
      if(loadIndices[0] == -1) {
        loadIndices[0] = package.getIndexByName("jointLoad/x");
        loadIndices[1] = package.getIndexByName("jointLoad/y");
        loadIndices[2] = package.getIndexByName("jointLoad/z");
      }
      Vector load;
      for(int i = 0; i < 3; ++i)
        package.get(loadIndices[0], &load[i]);

      doubleArray[callbackParam] = load.norm();
    }

  } // end of namespace sim
} // end of namespace mars
