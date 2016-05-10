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
 *  JointLoadSensor.h
 *  QTVersion
 *
 *  Created by Malte Roemmermann
 *
 */

#ifndef JOINTLOADSENSOR_H
#define JOINTLOADSENSOR_H

#ifdef _PRINT_HEADER_
#warning "JointLoadSensor.h"
#endif

#include "JointArraySensor.h"
#include <mars/data_broker/ProducerInterface.h>
#include <mars/data_broker/DataPackage.h>

namespace mars {
  namespace sim {

    class JointLoadSensor : public JointArraySensor,
                            public data_broker::ProducerInterface {

    public:
      JointLoadSensor(interfaces::ControlCenter *control, IDListConfig config);
      ~JointLoadSensor(void);

      virtual int getAsciiData(char* data) const ;
      virtual int getSensorData(interfaces::sReal **data) const ;
      virtual void produceData(const data_broker::DataInfo &info,
                               data_broker::DataPackage *package,
                               int callbackParam);
      virtual void receiveData(const data_broker::DataInfo &info,
                               const data_broker::DataPackage &package,
                               int callbackParam);

      static interfaces::BaseSensor* instanciate(interfaces::ControlCenter *control,
                                           interfaces::BaseConfig *config);

    private:
      long loadIndices[3];
      data_broker::DataPackage dbPackage;
    };

  } // end of namespace sim
} // end of namespace mars

#endif
