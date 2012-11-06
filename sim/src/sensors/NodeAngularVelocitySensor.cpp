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

#include "NodeAngularVelocitySensor.h"

#include <cstdio>
#include <cstdlib>

namespace mars {
  namespace sim {

    using namespace utils;
    using namespace interfaces;

    BaseSensor* NodeAngularVelocitySensor::instanciate(ControlCenter *control,
                                                       BaseConfig *config ){

      IDListConfig *cfg = dynamic_cast<IDListConfig*>(config);
      assert(cfg);
      return new NodeAngularVelocitySensor(control,*cfg);
    }

    NodeAngularVelocitySensor::NodeAngularVelocitySensor(ControlCenter* control,
                                                         IDListConfig config):
      NodeArraySensor(control, config, false) {

      int i;
      for(i=0; i<countIDs; i++) values.push_back(Vector(0.0, 0.0, 0.0));

      for(i = 0; i < 3; ++i) 
        angularVelocityIndices[i] = -1;

      typeName = "NodeAngularVelocity";
    }

    // this function should be overwritten by the special sensor to
    int NodeAngularVelocitySensor::getAsciiData(char* data) const {
      char *p;
      int num_char = 0;
      std::vector<Vector>::const_iterator iter;
  
      p = data;
      for(iter = values.begin(); iter != values.end(); iter++) {
        sprintf(p, " %10.4f %10.4f %10.4f", iter->x(),
                iter->y(), iter->z());
        num_char += 33;
        p += num_char;

      }
      return num_char;
    }

    int NodeAngularVelocitySensor::getSensorData(sReal** data) const {
      std::vector<Vector>::const_iterator iter;
      int i=0;

      *data = (sReal*)calloc(3*values.size(), sizeof(sReal));
      for(iter = values.begin(); iter != values.end(); iter++) {
        (*data)[i++] = iter->x();
        (*data)[i++] = iter->y();
        (*data)[i++] = iter->z();
      }
      return i;
    }

    void NodeAngularVelocitySensor::receiveData(const data_broker::DataInfo &info,
                                                const data_broker::DataPackage &package,
                                                int callbackParam) {
      if(angularVelocityIndices[0] == -1) {
        angularVelocityIndices[0] = package.getIndexByName("angularVelocity/x");
        angularVelocityIndices[1] = package.getIndexByName("angularVelocity/y");
        angularVelocityIndices[2] = package.getIndexByName("angularVelocity/z");
      }

      package.get(angularVelocityIndices[0], &values[callbackParam].x());
      package.get(angularVelocityIndices[1], &values[callbackParam].y());
      package.get(angularVelocityIndices[2], &values[callbackParam].z());
    }

  } // end of namespace sim
} // end of namespace mars
