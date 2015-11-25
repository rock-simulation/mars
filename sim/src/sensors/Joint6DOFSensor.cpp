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
 *  Created by Malte R�mmerann
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

    using namespace configmaps;
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
      dbPackage.add("id", (long)config.id);
      dbPackage.add("fx", 0.0);
      dbPackage.add("fy", 0.0);
      dbPackage.add("fz", 0.0);
      dbPackage.add("tx", 0.0);
      dbPackage.add("ty", 0.0);
      dbPackage.add("tz", 0.0);

      char text[55];
      sprintf(text, "Sensors/FT_%05lu", config.id);
      dbPushId = control->dataBroker->pushData("mars_sim", text,
                                               dbPackage, NULL,
                                               data_broker::DATA_PACKAGE_READ_FLAG);
      control->dataBroker->registerTimedProducer(this, "mars_sim", text,
                                                 "mars_sim/simTimer", 0);

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
      control->dataBroker->unregisterTimedProducer(this, "*", "*",
                                                   "mars_sim/simTimer");
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

    void Joint6DOFSensor::getForceData(utils::Vector *force){
    	*force = (sensor_data.body_q * sensor_data.force);
    }
    void Joint6DOFSensor::getTorqueData(utils::Vector *torque){
    	*torque = (sensor_data.body_q * sensor_data.torque);
    }
    void Joint6DOFSensor::getAnchor(utils::Vector *anchor){
    	*anchor = sensor_data.anchor;
    }
    void Joint6DOFSensor::getBodyQ(utils::Quaternion* body_q){
    	*body_q = sensor_data.body_q;
    }


    void Joint6DOFSensor::produceData(const data_broker::DataInfo &info,
                                      data_broker::DataPackage *dbPackage,
                                      int callbackParam) {
      Vector tmp;
      dbPackage->set(0, (long)id);
      tmp = (sensor_data.body_q * sensor_data.force);
      dbPackage->set(1, tmp.x());
      dbPackage->set(2, tmp.y());
      dbPackage->set(3, tmp.z());

      tmp = (sensor_data.body_q * sensor_data.torque);
      dbPackage->set(4, tmp.x());
      dbPackage->set(5, tmp.y());
      dbPackage->set(6, tmp.z());
    }

    void Joint6DOFSensor::receiveData(const data_broker::DataInfo &info,
                                      const data_broker::DataPackage &package,
                                      int callbackParam) {
      if(nodePackageId == info.dataId) {
        package.get(nodeRotIndices[0], &sensor_data.body_q.x());
        package.get(nodeRotIndices[1], &sensor_data.body_q.y());
        package.get(nodeRotIndices[2], &sensor_data.body_q.z());
        package.get(nodeRotIndices[3], &sensor_data.body_q.w());
        sensor_data.body_q.x() = -sensor_data.body_q.x();
        sensor_data.body_q.y() = -sensor_data.body_q.y();
        sensor_data.body_q.z() = -sensor_data.body_q.z();
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
      unsigned int mapIndex = (*config)["mapIndex"];
      unsigned long nodeID = (*config)["nodeID"];
      if(mapIndex) {
        nodeID = control->loadCenter->getMappedID(nodeID,
                                                  interfaces::MAP_TYPE_NODE,
                                                  mapIndex);
      }
      unsigned long jointID = (*config)["jointID"];
      if(mapIndex) {
        jointID = control->loadCenter->getMappedID(jointID,
                                                   interfaces::MAP_TYPE_JOINT,
                                                   mapIndex);
      }

      cfg->nodeID = nodeID;
      cfg->jointID = jointID;
      cfg->updateRate = (*config)["rate"];
      return cfg;
    }

    ConfigMap Joint6DOFSensor::createConfig() const {
      ConfigMap cfg;
      cfg["name"] = config.name;
      cfg["index"] = config.id;
      cfg["type"] = std::string("Joint6DOF");
      cfg["rate"] = config.updateRate;
      cfg["nodeID"] = config.nodeID;
      cfg["jointID"] = config.jointID;
      return cfg;
    }

  } // end of namespace sim
} // end of namespace mars
