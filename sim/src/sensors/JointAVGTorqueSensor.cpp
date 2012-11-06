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
 *  JointAVGTorqueSensor.cpp
 *  QTVersion
 *
 *  Created by Malte Römmerann
 *
 */

#include <cstdio>
#include "JointAVGTorqueSensor.h"

namespace mars {
  namespace sim {

    using namespace utils;
    using namespace interfaces;

    BaseSensor* JointAVGTorqueSensor::instanciate(ControlCenter *control,
                                                  BaseConfig *config ){

      IDListConfig *cfg = dynamic_cast<IDListConfig*>(config);
      assert(cfg);
      return new JointAVGTorqueSensor(control,*cfg);
    }

    JointAVGTorqueSensor::JointAVGTorqueSensor(ControlCenter *control,
                                               IDListConfig config):
      JointArraySensor(control, config) {

      torqueIndices[0] = -1;
      typeName = "JointAVGTorque";
    }

    // this function should be overwritten by the special sensor to
    int JointAVGTorqueSensor::getAsciiData(char* data) const {
      char *p;
      sReal torque = 0;
      std::vector<double>::const_iterator iter;
  
      p = data;
      for(iter = doubleArray.begin(); iter != doubleArray.end(); iter++) {
        torque += *iter;
      }
      torque /= doubleArray.size();
      sprintf(p, " %6.2f", torque);
      return 7;

    }

    int JointAVGTorqueSensor::getSensorData(sReal** data) const {
      std::vector<double>::const_iterator iter;
  
      *data = (sReal*)malloc(sizeof(sReal));
      **data = 0;
      for(iter = doubleArray.begin(); iter != doubleArray.end(); iter++) {
        **data += *iter;
      }
      **data /= doubleArray.size();

      return 1;
    }


    void JointAVGTorqueSensor::receiveData(const data_broker::DataInfo &info,
                                           const data_broker::DataPackage &package,
                                           int callbackParam) {
      if(torqueIndices[0] == -1) {
        torqueIndices[0] = package.getIndexByName("axis1/torque/x");
        torqueIndices[1] = package.getIndexByName("axis1/torque/y");
        torqueIndices[2] = package.getIndexByName("axis1/torque/z");
      }
      Vector torque;
      for(int i = 0; i < 3; ++i)
        package.get(torqueIndices[i], &torque[i]);
      doubleArray[callbackParam] = torque.norm();
      //values[callbackParam].value = torque.length();
      //values[callbackParam].value = torque.norm();
    }

  } // end of namespace sim
} // end of namespace mars
