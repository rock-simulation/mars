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
 *  NodeContactContactSensor.cpp
 *  QTVersion
 *
 *  Created by Malte Römmerann
 *
 */

#include "NodeContactForceSensor.h"
#include <mars/data_broker/DataBrokerInterface.h>

#include <cstdio>
#include <cstdlib>

namespace mars {
  namespace sim {

    using namespace utils;
    using namespace interfaces;

    BaseSensor* NodeContactForceSensor::instanciate(ControlCenter *control,
                                                    BaseConfig *config ){

      IDListConfig *cfg = dynamic_cast<IDListConfig*>(config);
      assert(cfg);
      return new NodeContactForceSensor(control,*cfg);
    }

    NodeContactForceSensor::NodeContactForceSensor(ControlCenter *control,
                                                   IDListConfig config):
      NodeArraySensor(control, config) {

      contactForceIndex = -1;
      typeName = "NodeContactForce";
    }

    // this function should be overwritten by the special sensor to
    int NodeContactForceSensor::getAsciiData(char* data) const {
      sReal contact = 0;
      std::vector<double>::const_iterator iter;
  
      for(iter = doubleArray.begin(); iter != doubleArray.end(); iter++) {
        contact += *iter;
      }
      sprintf(data, " %9.3f", contact);
      //fprintf(stderr, "\node ground contact: %s", tmp);
      return 10;
    }

    int NodeContactForceSensor::getSensorData(sReal** data) const {
      sReal contact = 0;
      std::vector<double>::const_iterator iter;
  
      *data = (sReal*)malloc(sizeof(sReal));
      for(iter = doubleArray.begin(); iter != doubleArray.end(); iter++) {
        contact += *iter;;
      }
      **data = contact;
      return 1;
    }

    void NodeContactForceSensor::receiveData(const data_broker::DataInfo &info,
                                             const data_broker::DataPackage &package,
                                             int callbackParam) {
      if(contactForceIndex == -1) {
        contactForceIndex = package.getIndexByName("contactForce");
      }
      package.get(contactForceIndex, &doubleArray[callbackParam]);
    }

  } // end of namespace sim
} // end of namespace mars
