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
 *  JointArraySensor.h
 *  QTVersion
 *
 *  Created by Malte Roemmermann
 *
 */

#ifndef JOINTARRAYSENSOR_H
#define JOINTARRAYSENSOR_H

#ifdef _PRINT_HEADER_
#warning "JointArraySensor.h"
#endif

#include <mars/utils/ConfigData.h>
#include <mars/interfaces/sim/SensorInterface.h>
#include <mars/data_broker/ReceiverInterface.h>
#include <mars/data_broker/DataPackage.h>

#include "IDListConfig.h"

namespace mars {
  namespace sim {

    class JointArraySensor : public interfaces::SensorInterface,
                             public interfaces::BaseSensor,
                             public data_broker::ReceiverInterface {

    public:
      JointArraySensor(interfaces::ControlCenter *control,
                       IDListConfig config, bool initArray=true);
      virtual ~JointArraySensor(void);
      virtual int getAsciiData(char* data) const ;
      virtual int getSensorData(interfaces::sReal **data) const ;
      virtual void receiveData(const data_broker::DataInfo &info,
                               const data_broker::DataPackage &package,
                               int callbackParam) {}

      static interfaces::BaseConfig* parseConfig(interfaces::ControlCenter *control,
                                                 utils::ConfigMap *config);
      virtual utils::ConfigMap createConfig() const;

    protected:
      std::string typeName;
      int countIDs;
      std::vector<double> doubleArray;

    private:
      IDListConfig config;

    };

  } // end of namespace sim
} // end of namespace mars

#endif
