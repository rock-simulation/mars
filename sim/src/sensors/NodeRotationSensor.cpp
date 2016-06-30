/*
 *  Copyright 2011, 2012, 2016, DFKI GmbH Robotics Innovation Center
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
 *  NodeRotationSensor.cpp
 *
 *  Created by Malte Langosz
 *
 */

#include "NodeRotationSensor.h"
#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/sim/SimulatorInterface.h>
#include <mars/data_broker/DataBrokerInterface.h>
#include <mars/utils/mathUtils.h>
#include "SimNode.h"

#include <cstdio>
#include <cstdlib>

namespace mars {
  namespace sim {

    using namespace utils;
    using namespace interfaces;

    BaseSensor* NodeRotationSensor::instanciate(ControlCenter *control,
                                                BaseConfig *config ){

      IDListConfig *cfg = dynamic_cast<IDListConfig*>(config);
      assert(cfg);
      return new NodeRotationSensor(control,*cfg);
    }

    NodeRotationSensor::NodeRotationSensor(ControlCenter* control,
                                           IDListConfig config):
      NodeArraySensor(control, config, false) {

      int i = 0;
  
      for(i=0; i<countIDs; i++) {
        values.push_back((RotationData){0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0});
        // store the initial node rotation
        rotations.push_back(control->nodes->getRotation(config.ids[i]));
        std::string name = control->nodes->getSimNode(config.ids[i])->getName();
        dbPackage.add(name+"/x", 0.0);
        dbPackage.add(name+"/y", 0.0);
        dbPackage.add(name+"/z", 0.0);
        dbPackage.add(name+"/w", 0.0);
        dbPackage.add(name+"/roll", 0.0);
        dbPackage.add(name+"/pitch", 0.0);
        dbPackage.add(name+"/yaw", 0.0);
      }

      for(i = 0; i < 4; ++i)
        rotationIndices[i] = -1;

      typeName = "NodeRotation";
    }

    // this function should be overwritten by the special sensor to
    int NodeRotationSensor::getAsciiData(char* data) const {
      char *p;
      int num_char = 0;
      std::vector<RotationData>::const_iterator iter;
  
      p = data;
      for(iter = values.begin(); iter != values.end(); iter++) {
        sprintf(p, " %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f", iter->x,
                iter->y, iter->z, iter->w, iter->roll,
                iter->pitch, iter->yaw);
        p += 49;
        num_char += 49;
      }
      return num_char;
    }

    int NodeRotationSensor::getSensorData(sReal** data) const {
      std::vector<RotationData>::const_iterator iter;
  
      *data = (sReal*)calloc(7, sizeof(sReal));
      for(iter = values.begin(); iter != values.end(); iter++) {
        (*data)[0] = iter->x;
        (*data)[1] = iter->y;
        (*data)[2] = iter->z;
        (*data)[3] = iter->w;
        (*data)[4] = iter->roll;
        (*data)[5] = iter->pitch;
        (*data)[6] = iter->yaw;
      }
      return 7;
    }

    void NodeRotationSensor::produceData(const data_broker::DataInfo &info,
                                         data_broker::DataPackage *dbPackage,
                                         int callbackParam) {
      *dbPackage = this->dbPackage;
    }

    void NodeRotationSensor::receiveData(const data_broker::DataInfo &info,
                                         const data_broker::DataPackage &package,
                                         int callbackParam) {
      if(rotationIndices[0] == -1) {
        rotationIndices[0] = package.getIndexByName("rotation/x");
        rotationIndices[1] = package.getIndexByName("rotation/y");
        rotationIndices[2] = package.getIndexByName("rotation/z");
        rotationIndices[3] = package.getIndexByName("rotation/w");
      }
      Quaternion q(1,0,0,0);
      package.get(rotationIndices[0], &q.x());
      package.get(rotationIndices[1], &q.y());
      package.get(rotationIndices[2], &q.z());
      package.get(rotationIndices[3], &q.w());
      Vector axis;
      double angle = angleBetween(control->sim->getGravity(), Vector(0, 0, -1),
                                  &axis);
      q = angleAxisToQuaternion(angle, axis)*q;
      RotationData r;
      //values[callbackParam].rot = q.toEuler();
      // calculate the roll / pitch and yaw in a secure way:
      Vector tmp, tmp2;
      r.x = q.x();
      r.y = q.y();
      r.z = q.z();
      r.w = q.w();

      // get the roll
      // we assume the original front orientation is on the x axis
      tmp = q*Vector(0, 1, 0);
      tmp = rotations[callbackParam].inverse()*tmp;
      tmp2 = Vector(0.0, tmp.y(), tmp.z());
      r.roll = angleBetween(Vector(0, 1, 0), tmp2, &axis);
      if(axis.x() < 0.0) r.roll = -r.roll;

      // get the pitch
      tmp = q*Vector(1, 0, 0);
      tmp = rotations[callbackParam].inverse()*tmp;
      tmp2 = Vector(tmp.x(), 0.0, tmp.z());
      r.pitch = angleBetween(Vector(1, 0, 0), tmp2, &axis);
      if(axis.y() < 0.0) r.pitch = -r.pitch;

      // get the yaw
      tmp2 = Vector(tmp.x(), tmp.y(), 0.0);
      r.yaw = angleBetween(Vector(1, 0, 0), tmp2, &axis);
      if(axis.z() < 0.0) r.yaw = -r.yaw;

      dbPackage.set(callbackParam*7, r.x);
      dbPackage.set(callbackParam*7+1, r.y);
      dbPackage.set(callbackParam*7+2, r.z);
      dbPackage.set(callbackParam*7+3, r.w);
      dbPackage.set(callbackParam*7+4, r.roll);
      dbPackage.set(callbackParam*7+5, r.pitch);
      dbPackage.set(callbackParam*7+6, r.yaw);
      values[callbackParam] = r;
    }

  } // end of namespace sim
} // end of namespace mars
