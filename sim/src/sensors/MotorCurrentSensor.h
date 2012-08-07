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
 *  MotorCurrentSensor.h
 *  QTVersion
 *
 *  Created by Malte Roemmermann
 *
 */

#ifndef MOTORCURRENTSENSOR_H
#define MOTORCURRENTSENSOR_H

#ifdef _PRINT_HEADER_
#warning "MotorCurrentSensor.h"
#endif

#include <mars/utils/ConfigData.h>
#include <mars/interfaces/sim/SensorInterface.h>
#include <mars/data_broker/ReceiverInterface.h>

#include "IDListConfig.h"

namespace mars {
  namespace sim {

    class MotorCurrentSensor : public interfaces::SensorInterface,
                               public interfaces::BaseSensor,
                               public data_broker::ReceiverInterface {

    public:
      MotorCurrentSensor(interfaces::ControlCenter *control, IDListConfig config);
      ~MotorCurrentSensor(void);

      virtual int getAsciiData(char* data) const;
      virtual int getSensorData(interfaces::sReal **data) const;

      virtual void receiveData(const data_broker::DataInfo &info,
                               const data_broker::DataPackage &package,
                               int callbackParam);

      static interfaces::BaseSensor* instanciate(interfaces::ControlCenter *control,
                                                 interfaces::BaseConfig *config);
      static interfaces::BaseConfig* parseConfig(interfaces::ControlCenter *control,
                                                 utils::ConfigMap *config);

      virtual utils::ConfigMap createConfig() const;

    private:
      IDListConfig config;
      std::vector<double> doubleArray;
      std::string typeName;
      int countIDs;
      int dbCurrentIndex;

    };

  } // end of namespace sim
} // end of namespace mars

#endif
