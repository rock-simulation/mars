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
 *  JointPositionSensor.cpp
 *  QTVersion
 *
 *  Created by Malte Römmerann
 *
 */

#include "JointPositionSensor.h"

#include <cstdio>

namespace mars {
  namespace sim {

    using namespace utils;
    using namespace interfaces;

    BaseSensor* JointPositionSensor::instanciate(ControlCenter *control,
                                                 BaseConfig *config ){

      IDListConfig *cfg = dynamic_cast<IDListConfig*>(config);
      assert(cfg);
      return new JointPositionSensor(control,*cfg);
    }

    JointPositionSensor::JointPositionSensor(ControlCenter *control,
                                             IDListConfig config):
      JointArraySensor(control, config),
      angleIndex(-1) {

      typeName = "JointPosition";
    }

    void JointPositionSensor::receiveData(const data_broker::DataInfo &info,
                                          const data_broker::DataPackage &package,
                                          int callbackParam) {
      if(angleIndex == -1)
        angleIndex = package.getIndexByName("axis1/angle");
      package.get(angleIndex, &doubleArray[callbackParam]);
    }

  } // end of namespace sim
} // end of namespace mars
