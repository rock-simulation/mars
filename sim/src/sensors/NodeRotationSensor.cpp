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
 *  NodeRotationSensor.cpp
 *  QTVersion
 *
 *  Created by Malte Römmerann
 *
 */

#include "NodeRotationSensor.h"
#include <mars/data_broker/DataBrokerInterface.h>
#include <mars/utils/mathUtils.h>

#include <cstdio>
#include <cstdlib>

namespace mars {
  namespace sim {

    using namespace utils;
    using namespace interfaces;

    BaseSensor* NodeRotationSensor::instanciate(ControlCenter *control,
                                                BaseConfig *config ){

      IDListConfig *cfg = dynamic_cast<IDListConfig*>(config);
      assert(cfg);
      return new NodeRotationSensor(control,*cfg);
    }

    NodeRotationSensor::NodeRotationSensor(ControlCenter* control,
                                           IDListConfig config):
      NodeArraySensor(control, config, false) {

      int i = 0;
  
      for(i=0; i<countIDs; i++) values.push_back((sRotation){0.0, 0.0, 0.0});

      for(i = 0; i < 4; ++i)
        rotationIndices[i] = -1;

      typeName = "NodeRotation";
    }

    // this function should be overwritten by the special sensor to
    int NodeRotationSensor::getAsciiData(char* data) const {
      char *p;
      int num_char = 0;
      std::vector<sRotation>::const_iterator iter;
  
      p = data;
      for(iter = values.begin(); iter != values.end(); iter++) {
        sprintf(p, " %6.2f %6.2f %6.2f", iter->alpha,
                iter->beta, iter->gamma);
        p += 21;
        num_char += 21;
      }
      //fprintf(stderr, "\node rotation: %s", tmp);
      return num_char;
    }

    int NodeRotationSensor::getSensorData(sReal** data) const {
      std::vector<sRotation>::const_iterator iter;
  
      *data = (sReal*)calloc(3, sizeof(sReal));
      for(iter = values.begin(); iter != values.end(); iter++) {
        (*data)[0] = iter->alpha;
        (*data)[1] = iter->beta;
        (*data)[2] = iter->gamma;
      }
      return 3;
    }

    void NodeRotationSensor::receiveData(const data_broker::DataInfo &info,
                                         const data_broker::DataPackage &package,
                                         int callbackParam) {
      if(rotationIndices[0] == -1) {
        rotationIndices[0] = package.getIndexByName("rotation/x");
        rotationIndices[1] = package.getIndexByName("rotation/y");
        rotationIndices[2] = package.getIndexByName("rotation/z");
        rotationIndices[3] = package.getIndexByName("rotation/w");
      }
      Quaternion q(1,0,0,0);
      package.get(rotationIndices[0], &q.x());
      package.get(rotationIndices[1], &q.y());
      package.get(rotationIndices[2], &q.z());
      package.get(rotationIndices[3], &q.w());
      //values[callbackParam].rot = q.toEuler();
      values[callbackParam] = quaternionTosRotation(q);
    }

  } // end of namespace sim
} // end of namespace mars
