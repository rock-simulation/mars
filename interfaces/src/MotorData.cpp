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

#include "MotorData.h"
#include "sim/LoadCenter.h"
//#include <initializer_list>

#define GET_VALUE(str, val, type)                                        \
  if((it = config->find(str)) != config->end()) {                         \
    val = it->second;                                   \
  } else if ((it = config->find(legacynames[str])) != config->end()) {   \
    val = it->second;                                                   \
  }

#define SET_VALUE(str, val)                              \
  if(val != defaultMotor.val)                             \
    (*config)[str] = val

namespace mars {
  namespace interfaces {

    using namespace configmaps;

    std::map<std::string, std::string> MotorData::legacynames = init_legacynames();

//    C++ 11:
//     MotorData::legacynames = {{"maxEffort", "motorMaxForce"},
//        {"maxSpeed", "maximumVelocity"}};

    MotorData::MotorData(const std::string& name, MotorType type){
      init( name, type );
    }

//    MotorData::MotorData(const std::string& name, MotorType type): maximumVelocity(maxSpeed),
//        motorMaxForce(maxEffort) {
//      init( name, type );
//    }

//    MotorData& MotorData::operator=(const MotorData& other){
//      if (&other==this) {
//        return *this;
//      } else {
//          this->name = other.name;
//          this->index = other.index;
//          this->jointIndex = other.jointIndex;
//          this->jointIndex2 = other.jointIndex2;
//          this->axis = other.axis;
//          this->value = other.value;
//          this->maxSpeed = other.maxSpeed;
//          this->maxEffort = 333;//other.maxEffort;
//          this->type = type;
//          this->p = other.p;
//          this->i = other.i;
//          this->d = other.d;
//          this->maxValue = other.maxValue;
//          this->minValue = other.minValue;
//        }
//    }

    void MotorData::init(const std::string& name, MotorType type) {
      this->name = name;
      this->type = type;
      index = 0;
      jointIndex = 0;
      jointIndex2 = 0;
      axis = 1;
      maxSpeed = 0;
      maxEffort = 0;
      maxAcceleration = 0;
      p = 0;
      i = 0;
      d = 0;
      maxValue = M_PI;
      minValue = -M_PI;
      value = 0;
    }

    bool MotorData::fromConfigMap(ConfigMap *config,
                                  std::string filenamePrefix,
                                  LoadCenter *loadCenter) {
      CPP_UNUSED(filenamePrefix);
      CPP_UNUSED(loadCenter);
      ConfigMap::iterator it;
      unsigned int mapIndex;
      GET_VALUE("mapIndex", mapIndex, UInt);

      name = config->get("name", name);
      GET_VALUE("index", index, ULong);
      GET_VALUE("jointIndex", jointIndex, ULong);
      GET_VALUE("jointIndex2", jointIndex2, ULong);
      if(mapIndex && loadCenter) {
        if(jointIndex) {
          jointIndex = loadCenter->getMappedID(jointIndex, MAP_TYPE_JOINT,
                                              mapIndex);
        }
        if(jointIndex2) {
          jointIndex2 = loadCenter->getMappedID(jointIndex2, MAP_TYPE_JOINT,
                                               mapIndex);
        }
      }

      GET_VALUE("axis", axis, Int);
      GET_VALUE("maxSpeed", maxSpeed, Double);
      GET_VALUE("maxEffort", maxEffort, Double);
      GET_VALUE("maxAcceleration", maxAcceleration, Double);

      std::string tmpmotortype;
      tmpmotortype = config->get("type", tmpmotortype);
      if (tmpmotortype=="1" || tmpmotortype=="PID") {
        type = (MotorType)1ul;
      } else if (tmpmotortype=="2" || tmpmotortype=="DC") {
        type = (MotorType)2ul;
      }

      GET_VALUE("p", p, Double);
      GET_VALUE("i", i, Double);
      GET_VALUE("d", d, Double);
      GET_VALUE("position", value, Double);
      GET_VALUE("maxPosition", maxValue, Double);
      GET_VALUE("minPosition", minValue, Double);

      this->config = *config;

      return 1;
    }

    void MotorData::toConfigMap(ConfigMap *config, bool skipFilenamePrefix) {
      CPP_UNUSED(skipFilenamePrefix);
      MotorData defaultMotor;

      SET_VALUE("name", name);
      SET_VALUE("index", index);
      SET_VALUE("jointIndex", jointIndex);
      SET_VALUE("jointIndex2", jointIndex2);
      SET_VALUE("axis", axis);
      SET_VALUE("maxSpeed", maxSpeed);
      SET_VALUE("maxEffort", maxEffort);

      if(type==2) {
        (*config)["type"] = "DC";
      }
      else {
        (*config)["type"] = "PID";
      }

      SET_VALUE("p", p);
      SET_VALUE("i", i);
      SET_VALUE("d", d);
      SET_VALUE("maxPosition", maxValue);
      SET_VALUE("minPosition", minValue);
    }

    void MotorData::getFilesToSave(std::vector<std::string> *fileList) {
      CPP_UNUSED(fileList);
    }

  } // end of namespace interfaces
} // end of namespace mars
