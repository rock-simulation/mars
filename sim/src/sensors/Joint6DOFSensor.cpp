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
 *  Joint6DOFSensor.cpp
 *  QTVersion
 *
 *  Created by Malte Römmerann
 *
 */

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "Joint6DOFSensor.h"

#include <mars/interfaces/sim/SimulatorInterface.h>
#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/sim/JointManagerInterface.h>
#include <mars/interfaces/sim/LoadCenter.h>

#include <mars/data_broker/DataBrokerInterface.h>

namespace mars {
  namespace sim {

    using namespace utils;
    using namespace interfaces;

    BaseSensor* Joint6DOFSensor::instanciate(ControlCenter *control,
                                             BaseConfig *config ){
      Joint6DOFConfig *cfg = dynamic_cast<Joint6DOFConfig*>(config);
      assert(cfg);
      return new Joint6DOFSensor(control,*cfg);
    }


    enum { CALLBACK_NODE, CALLBACK_JOINT };

    Joint6DOFSensor::Joint6DOFSensor(ControlCenter *control,
                                     Joint6DOFConfig config) :
      SensorInterface(control),
      BaseSensor(config.id, config.name),
      config(config) {

      std::vector<unsigned long>::iterator iter;
      jointPackageId = nodePackageId = 0;

      sensor_data.body_id = config.nodeID;
      sensor_data.joint_id = config.jointID;
  
      std::string groupName, dataName;

      control->nodes->getDataBrokerNames(sensor_data.body_id,
                                         &groupName, &dataName);
      control->dataBroker->registerTimedReceiver(this, groupName, 
                                                 dataName,
                                                 "mars_sim/simTimer", 
                                                 config.updateRate,
                                                 CALLBACK_NODE);
      control->joints->getDataBrokerNames(sensor_data.joint_id,
                                          &groupName, &dataName);
      control->dataBroker->registerTimedReceiver(this, groupName, 
                                                 dataName,
                                                 "mars_sim/simTimer",
                                                 config.updateRate,
                                                 CALLBACK_JOINT);
      NodeData theNode = control->nodes->getFullNode(sensor_data.body_id);
      JointData theJoint = control->joints->getFullJoint(sensor_data.joint_id);
      sensor_data.anchor = theJoint.anchor;
      sensor_data.anchor -= theNode.pos;
      sensor_data.tmp = 1 / (sensor_data.anchor.x()*sensor_data.anchor.x() +
                             sensor_data.anchor.y()*sensor_data.anchor.y() +
                             sensor_data.anchor.z()*sensor_data.anchor.z());

      sensor_data.length = sensor_data.anchor.norm();

    }

    Joint6DOFSensor::~Joint6DOFSensor(void) {
      control->dataBroker->unregisterTimedReceiver(this, "*", "*", 
                                                   "mars_sim/simTimer");
    }

    // this function should be overwritten by the special sensor to
    int Joint6DOFSensor::getAsciiData(char* data) const {
      char *p;
      Vector force, torque;
      Vector pro;
 
      force =  (sensor_data.body_q * sensor_data.force);
      torque = (sensor_data.body_q * sensor_data.torque);
      p = data;
      sprintf(p, " %10.6g %10.6g %10.6g %10.6g %10.6g %10.6g",
              force.x(), force.y(), force.z(), torque.x(), torque.y(), torque.z());

      return strlen(p);
    }


    int Joint6DOFSensor::getSensorData(sReal** data) const {
      Vector tmp;
  
      *data = (sReal*)malloc(sizeof(sReal)*6);
      tmp = (sensor_data.body_q * sensor_data.force);
      (*data)[0] = tmp.x();
      (*data)[1] = tmp.y();
      (*data)[2] = tmp.z();
      tmp = (sensor_data.body_q * sensor_data.torque);
      (*data)[3] = tmp.x();
      (*data)[4] = tmp.y();
      (*data)[5] = tmp.z();
      return 6;
    }


    void Joint6DOFSensor::receiveData(const data_broker::DataInfo &info,
                                      const data_broker::DataPackage &package,
                                      int callbackParam) {
      if(nodePackageId == info.dataId) {
        package.get(nodeRotIndices[0], &sensor_data.body_q.x());
        package.get(nodeRotIndices[1], &sensor_data.body_q.y());
        package.get(nodeRotIndices[2], &sensor_data.body_q.z());
        package.get(nodeRotIndices[3], &sensor_data.body_q.w());
      } else if(jointPackageId == info.dataId) {
        for(int i = 0; i < 3; ++i) {
          package.get(jointForceIndices[i], &sensor_data.force[i]);
          package.get(jointTorqueIndices[i], &sensor_data.torque[i]);
        }
      } else {
        // we don't know the dataId yet, so assign the ids...
        switch(callbackParam) {
        case CALLBACK_NODE:
          nodePackageId = info.dataId;
          nodeRotIndices[0] = package.getIndexByName("rotation/x");
          nodeRotIndices[1] = package.getIndexByName("rotation/y");
          nodeRotIndices[2] = package.getIndexByName("rotation/z");
          nodeRotIndices[3] = package.getIndexByName("rotation/w");
          break;
        case CALLBACK_JOINT:
          jointPackageId = info.dataId;
          jointForceIndices[0] = package.getIndexByName("force1/x");
          jointForceIndices[1] = package.getIndexByName("force1/y");
          jointForceIndices[2] = package.getIndexByName("force1/z");
          jointTorqueIndices[0] = package.getIndexByName("torque1/x");
          jointTorqueIndices[1] = package.getIndexByName("torque1/y");
          jointTorqueIndices[2] = package.getIndexByName("torque1/z");
          break;
        default:
          // Error: We are not interested in this package. Why did we get it?
          return;
        }
        // ...and call this method again.
        receiveData(info, package, callbackParam);
      }
    }

    BaseConfig* Joint6DOFSensor::parseConfig(ControlCenter *control,
                                             ConfigMap *config) {
      Joint6DOFConfig *cfg = new Joint6DOFConfig;
      unsigned int mapIndex = (*config)["mapIndex"][0].getUInt();
      unsigned long nodeID = (*config)["nodeID"][0].getULong();
      if(mapIndex) {
        nodeID = control->loadCenter->getMappedID(nodeID,
                                                  interfaces::MAP_TYPE_NODE,
                                                  mapIndex);
      }
      unsigned long jointID = (*config)["jointID"][0].getULong();
      if(mapIndex) {
        jointID = control->loadCenter->getMappedID(jointID,
                                                   interfaces::MAP_TYPE_JOINT,
                                                   mapIndex);
      }

      cfg->nodeID = nodeID;
      cfg->jointID = jointID;
      cfg->updateRate = (*config)["rate"][0].getULong();
      return cfg;
    }

    ConfigMap Joint6DOFSensor::createConfig() const {
      ConfigMap cfg;
      cfg["name"][0] = ConfigItem(config.name);
      cfg["index"][0] = ConfigItem(config.id);
      cfg["type"][0] = ConfigItem(std::string("Joint6DOF"));
      cfg["rate"][0] = ConfigItem(config.updateRate);
      cfg["nodeID"][0] = ConfigItem(config.nodeID);
      cfg["jointID"][0] = ConfigItem(config.jointID);
      return cfg;
    }

  } // end of namespace sim
} // end of namespace mars
