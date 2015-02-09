/*
 *  Copyright 2011, 2012,  DFKI GmbH Robotics Innovation Center
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
 *  NodeContactSensor.h
 *
 *  Created by Malte Roemmermann
 *
 */

#ifndef NODECONTACTSENSOR_H
#define NODECONTACTSENSOR_H

#ifdef _PRINT_HEADER_
#warning "NodeContactSensor.h"
#endif

#include <configmaps/ConfigData.h>
#include <mars/interfaces/sim/SensorInterface.h>
#include <mars/data_broker/ReceiverInterface.h>

#include "IDListConfig.h"

namespace mars {
  namespace sim {

    class NodeContactSensor : public interfaces::SensorInterface,
                              public interfaces::BaseNodeSensor,
                              public data_broker::ReceiverInterface {

    public:
      NodeContactSensor(interfaces::ControlCenter *control, IDListConfig config);
      ~NodeContactSensor(void);
      virtual int getAsciiData(char* data) const ;
      virtual int getSensorData(interfaces::sReal** data) const;
      virtual void receiveData(const data_broker::DataInfo &info,
                               const data_broker::DataPackage &package,
                               int callbackParam);

      virtual configmaps::ConfigMap createConfig() const;

      static interfaces::BaseConfig* parseConfig(interfaces::ControlCenter *control,
                                           configmaps::ConfigMap *config);
      static interfaces::BaseSensor* instanciate(interfaces::ControlCenter *control,
                                           interfaces::BaseConfig *config);

    private:
      std::string typeName;
      IDListConfig config;
      std::vector<bool> values;
      long contactIndex;
      int countIDs;
      int groundContactIndex;
    };

  } // end of namespace sim
} // end of namespace mars

#endif
