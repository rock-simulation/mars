/*
 *  Copyright 2012, DFKI GmbH Robotics Innovation Center
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
 *  NodeArraySensor.h
 *  QTVersion
 *
 *  Created by Malte Roemmermann
 *
 */

#ifndef NODEARRAYSENSOR_H
#define NODEARRAYSENSOR_H

#ifdef _PRINT_HEADER_
#warning "NodeArraySensor.h"
#endif

#include <configmaps/ConfigData.h>
#include <mars/interfaces/sim/SensorInterface.h>
#include <mars/data_broker/ReceiverInterface.h>

#include "IDListConfig.h"

namespace mars {
  namespace sim {

    class NodeArraySensor : public interfaces::SensorInterface,
                            public interfaces::BaseSensor,
                            public data_broker::ReceiverInterface {

    public:
      NodeArraySensor(interfaces::ControlCenter *control,
                      IDListConfig config, bool initArray=true,
                      bool registerReceiver=true);

      virtual ~NodeArraySensor(void);
      virtual int getAsciiData(char* data) const ;
      virtual int getSensorData(interfaces::sReal **data) const ;
      virtual void receiveData(const data_broker::DataInfo &info,
                               const data_broker::DataPackage &package,
                               int callbackParam) {}

      static interfaces::BaseConfig* parseConfig(interfaces::ControlCenter *control,
                                     configmaps::ConfigMap *config);
      virtual configmaps::ConfigMap createConfig() const;

    protected:
      std::string typeName;
      int countIDs;
      std::vector<double> doubleArray;
      IDListConfig config;
    };

  } // end of namespace sim
} // end of namespace mars

#endif
