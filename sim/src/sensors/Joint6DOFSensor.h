/*
 *  Copyright 2011, 2012, 2014, DFKI GmbH Robotics Innovation Center
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
 *  Joint6DOFSensor.h
 *  QTVersion
 *
 *  Created by Malte Roemmermann
 */

#ifndef JOINT6DOFSENSOR_H
#define JOINT6DOFSENSOR_H

#ifdef _PRINT_HEADER_
#warning "Joint6DOFSensor.h"
#endif

#include <mars/data_broker/ProducerInterface.h>
#include <mars/data_broker/ReceiverInterface.h>
#include <mars/interfaces/sim/SensorInterface.h>

namespace mars {
  namespace sim {

    class Joint6DOFConfig : public interfaces::BaseConfig{
    public:
      Joint6DOFConfig(){
        name = "Unknown 6DOFSensor";
        nodeID = 0;
        jointID = 0;
      }

      unsigned long nodeID, jointID;
    };

    typedef struct joint6DOFData {
      unsigned long body_id, joint_id;
      utils::Vector force, torque, anchor;
      utils::Quaternion body_q;
      interfaces::sReal tmp, length;
    }joint6DOFData;

    class Joint6DOFSensor : public interfaces::SensorInterface,
                            public interfaces::BaseSensor,
                            public data_broker::ProducerInterface,
                            public data_broker::ReceiverInterface{

    public:

      Joint6DOFSensor(interfaces::ControlCenter *control, Joint6DOFConfig config);
      ~Joint6DOFSensor(void);

      virtual int getAsciiData(char* data) const;
      virtual int getSensorData(interfaces::sReal **data) const;

      void getForceData(utils::Vector *force);
      void getTorqueData(utils::Vector *torque);
      void getAnchor(utils::Vector *anchor);
      void getBodyQ(utils::Quaternion* body_q);


      virtual void produceData(const data_broker::DataInfo &info,
                               data_broker::DataPackage *package,
                               int callbackParam);
      virtual void receiveData(const data_broker::DataInfo &info,
                               const data_broker::DataPackage &package,
                               int callbackParam);

      static interfaces::BaseSensor* instanciate(interfaces::ControlCenter *control,
                                           interfaces::BaseConfig* config);
      static interfaces::BaseConfig* parseConfig(interfaces::ControlCenter *control,
                                           utils::ConfigMap *config);
      virtual utils::ConfigMap createConfig() const;

    private:
      Joint6DOFConfig config;
      joint6DOFData sensor_data;
      unsigned long jointPackageId, nodePackageId;
      long nodeRotIndices[4];
      long jointForceIndices[3];
      long jointTorqueIndices[3];
      data_broker::DataPackage dbPackage;
      unsigned long dbPushId;
    };

  } // end of namespace sim
} // end of namespace mars

#endif
