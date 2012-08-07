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
 *  JointVelocitySensor.cpp
 *  QTVersion
 *
 *  Created by Alexander Dettmann
 *
 */

#include "JointVelocitySensor.h"

#include <cstdio>

namespace mars {
  namespace sim {

    using namespace utils;
    using namespace interfaces;

    BaseSensor* JointVelocitySensor::instanciate(ControlCenter *control,
                                                 BaseConfig *config ){

      IDListConfig *cfg = dynamic_cast<IDListConfig*>(config);
      assert(cfg);
      return new JointVelocitySensor(control,*cfg);
    }

    JointVelocitySensor::JointVelocitySensor(ControlCenter *control,
                                             IDListConfig config):
      JointArraySensor(control, config),
      speedIndex(-1) {

      typeName = "JointVelocity";
    }

    // this function should be overwritten by the special sensor to
    int JointVelocitySensor::getAsciiData(char* data) const {
      char *p;
      int num_char = 0;
      std::vector<double>::const_iterator iter;
  
      p = data;
      for(iter = doubleArray.begin(); iter != doubleArray.end(); iter++) {
        sprintf(p, " %10.5f", *iter);
        num_char += 11;
        p += 11;
      }
      return num_char;
    }

    void JointVelocitySensor::receiveData(const data_broker::DataInfo &info,
                                          const data_broker::DataPackage &package,
                                          int callbackParam) {
      if(speedIndex == -1) {
        speedIndex = package.getIndexByName("axis1/speed");
      }
      package.get(speedIndex, &doubleArray[callbackParam]);
    }

  } // end of namespace sim
} // end of namespace mars
