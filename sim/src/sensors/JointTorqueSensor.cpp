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
 *  JointTorqueSensor.cpp
 *  QTVersion
 *
 *  Created by Malte Römmerann
 *
 */

#include <cstdio>

#include "JointTorqueSensor.h"

namespace mars {
  namespace sim {

    using namespace utils;
    using namespace interfaces;

    BaseSensor* JointTorqueSensor::instanciate(ControlCenter *control,
                                               BaseConfig *config ){

      IDListConfig *cfg = dynamic_cast<IDListConfig*>(config);
      assert(cfg);
      return new JointTorqueSensor(control,*cfg);
    }

    JointTorqueSensor::JointTorqueSensor(ControlCenter *control,
                                         IDListConfig config):
      JointArraySensor(control, config),
      motorTorqueIndex(-1) {

      typeName = "JointTorque";
    }

    // this function should be overwritten by the special sensor to
    int JointTorqueSensor::getAsciiData(char* data) const {
      char *p;
      std::vector<double>::const_iterator iter;
  
      p = data;
      for(iter = doubleArray.begin(); iter != doubleArray.end(); iter++) {
        sprintf(p, " %6.2f", *iter);
        p += 7;
      }

      return 7*doubleArray.size();
    }

    void JointTorqueSensor::receiveData(const data_broker::DataInfo &info,
                                        const data_broker::DataPackage &package,
                                        int callbackParam) {
      if(motorTorqueIndex == -1) {
        motorTorqueIndex = package.getIndexByName("motorTorque");
      }
      package.get(motorTorqueIndex, &doubleArray[callbackParam]);
    }

  } // end of namespace sim
} // end of namespace mars
