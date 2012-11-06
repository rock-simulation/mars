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
 *  NodeCOMSensor.cpp
 *  QTVersion
 *
 *  Created by Malte Römmerann
 *
 */

#include "NodeCOMSensor.h"
#include <mars/interfaces/sim/NodeManagerInterface.h>

#include <cstdio>
#include <cstdlib>

namespace mars {
  namespace sim {

    using namespace utils;
    using namespace interfaces;

    BaseSensor* NodeCOMSensor::instanciate(ControlCenter *control,
                                           BaseConfig *config ){
  
      IDListConfig *cfg = dynamic_cast<IDListConfig*>(config);
      assert(cfg);
      return new NodeCOMSensor(control,*cfg);
    }

    NodeCOMSensor::NodeCOMSensor(ControlCenter* control, IDListConfig config):
      NodeArraySensor(control, config, false, false) {
      typeName = "NodeCOM";
    }

    // this function should be overwritten by the special sensor to
    int NodeCOMSensor::getAsciiData(char* data) const {
      Vector center = control->nodes->getCenterOfMass(config.ids);
      sprintf(data, " %6.2f %6.2f %6.2f", center.x(), center.y(), center.z());
      return 21;
    }

    int NodeCOMSensor::getSensorData(sReal** data) const {
      Vector center = control->nodes->getCenterOfMass(config.ids);
  
      *data = (sReal*)calloc(3, sizeof(sReal));
      (*data)[0] = center.x();
      (*data)[1] = center.y();
      (*data)[2] = center.z();
      return 3;
    }

  } // end of namespace sim
} // end of namespace mars
