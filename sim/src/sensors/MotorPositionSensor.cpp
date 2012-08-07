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
 *  MotorCurrrentSensor.cpp
 *  QTVersion
 *
 *  Created by Malte Römmerann
 *
 */

#include "MotorPositionSensor.h"
#include <mars/interfaces/sim/MotorManagerInterface.h>
#include <mars/data_broker/DataBrokerInterface.h>

#include <cstdio>
#include <cstdlib>

namespace mars {
  namespace sim {

    using namespace utils;
    using namespace interfaces;

    MotorPositionSensor::MotorPositionSensor(ControlCenter *control,
                                             const unsigned long id,
                                             const std::string &name):
      SensorInterface(control),
      BaseSensor(id,name)
    {
#warning "Work needed"
#if 0
      std::vector<unsigned long>::iterator iter;
      std::string groupName, dataName;
      motorPositionData tmp = {0, 0.0};

      this->sSensor = *sSensor;
      this->control = control;
      // register for loging
      int i = 0;
      for(iter = sSensor->indices.begin(); 
          iter != sSensor->indices.end(); ++iter, ++i) {
        tmp.id = *iter;
        values.push_back(tmp);
        control->motors->getDataBrokerNames(*iter, &groupName, &dataName);
        control->dataBroker->registerTimedReceiver(this, groupName, dataName,
                                                   "mars_sim/simTimer",
                                                   sSensor->rate, i);
      }
      positionIndex = -1;
#endif
    }

    MotorPositionSensor::~MotorPositionSensor(void) {
      control->dataBroker->unregisterTimedReceiver(this, "*", "*", 
                                                   "mars_sim/simTimer");
    }

    // this function should be overwritten by the special sensor to
    int MotorPositionSensor::getMonsterData(char* data) const {
#if 0
      char *p;
      int num_char = 0;
      std::vector<motorPositionData>::const_iterator iter;
  
      p = data;

      for(iter = values.begin(); iter != values.end(); iter++) {
        sprintf(p, " %10.7f", ((*iter).value));
	num_char += 11;
	p += 11;
      }
      return num_char;
#endif
      return 0;
    }

    int MotorPositionSensor::getSensorData(sReal** data) const {
#if 0
      std::vector<motorPositionData>::const_iterator iter;
      int i=0;

      *data = (sReal*)calloc(values.size(), sizeof(sReal));
      for(iter = values.begin(); iter != values.end(); iter++) {
        (*data)[i++] = (*iter).value;
      }
      return i;
#endif
      return 0;
    }

    void MotorPositionSensor::receiveData(const data_broker::DataInfo &info,
                                          const data_broker::DataPackage &package,
                                          int callbackParam) {
      if(positionIndex == -1) {
        positionIndex = package.getIndexByName("position");
      }
      package.get(positionIndex, &values[callbackParam].value);
    }

  } // end of namespace sim
} // end of namespace mars
