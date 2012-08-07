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

#include "NodeVelocitySensor.h"
#include <cstdio>
#include <cstdlib>

namespace mars {
  namespace sim {

    using namespace utils;
    using namespace interfaces;

    BaseSensor* NodeVelocitySensor::instanciate(ControlCenter *control,
                                                BaseConfig *config ){

      IDListConfig *cfg = dynamic_cast<IDListConfig*>(config);
      assert(cfg);
      return new NodeVelocitySensor(control,*cfg);
    }

    NodeVelocitySensor::NodeVelocitySensor(ControlCenter *control,
                                           IDListConfig config):
      NodeArraySensor(control, config, false) {

      int i;
      for(i=0; i<countIDs; i++) values.push_back(Vector(0.0, 0.0, 0.0));

      for(i = 0; i < 3; ++i) 
        velocityIndices[i] = -1;

      typeName = "NodeVelocity";
    }

    // this function should be overwritten by the special sensor to
    int NodeVelocitySensor::getAsciiData(char* data) const {
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

    int NodeVelocitySensor::getSensorData(sReal** data) const {
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

    void NodeVelocitySensor::receiveData(const data_broker::DataInfo &info,
                                         const data_broker::DataPackage &package,
                                         int callbackParam) {
      if(velocityIndices[0] == -1) {
        velocityIndices[0] = package.getIndexByName("linearVelocity/x");
        velocityIndices[1] = package.getIndexByName("linearVelocity/y");
        velocityIndices[2] = package.getIndexByName("linearVelocity/z");
      }

      package.get(velocityIndices[0], &values[callbackParam].x());
      package.get(velocityIndices[1], &values[callbackParam].y());
      package.get(velocityIndices[2], &values[callbackParam].z());
    }

  } // end of namespace sim
} // end of namespace mars
