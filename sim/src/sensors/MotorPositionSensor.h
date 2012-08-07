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
 *  MotorPositionSensor.h
 *  QTVersion
 *
 *  Created by Malte Roemmermann
 *
 */

#ifndef MOTORPOSITIONSENSOR_H
#define MOTORPOSITIONSENSOR_H

#ifdef _PRINT_HEADER_
#warning "MotorPositionSensor.h"
#endif

#include <mars/interfaces/sim/SensorInterface.h>
#include <mars/data_broker/ReceiverInterface.h>

namespace mars {
  namespace sim {

    struct motorPositionData {
      unsigned long id;
      interfaces::sReal value;
    };

    class MotorPositionSensor : public interfaces::SensorInterface,
                                public interfaces::BaseSensor,
                                public data_broker::ReceiverInterface {

    public:
      MotorPositionSensor(interfaces::ControlCenter *control, const unsigned long id,
                          const std::string &name);
      ~MotorPositionSensor(void);
      virtual int getMonsterData(char* data) const;
      virtual int getSensorData(interfaces::sReal **data) const;
      virtual void receiveData(const data_broker::DataInfo &info,
                               const data_broker::DataPackage &package,
                               int callbackParam);

    private:
      std::vector<motorPositionData> values;
      long positionIndex;
    };

  } // end of namespace sim
} // end of namespace mars

#endif
