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

#define GET_VALUE(str, val, type)                    \
  if((it = config->find(str)) != config->end())      \
    val = it->second[0].get##type()

#define SET_VALUE(str, val)                              \
  if(val != defaultMotor.val)                             \
    (*config)[str][0] = ConfigItem(val)

namespace mars {
  namespace interfaces {

    using namespace mars::utils;

    MotorData::MotorData(const std::string& name, MotorType type) {
      init( name, type );
    }

    void MotorData::init(const std::string& name, MotorType type) {
      this->name = name;
      index = 0;
      jointIndex = 0;
      jointIndex2 = 0;
      axis = 0;
      value = 0;
      maximumVelocity = 0;
      motorMaxForce = 0;
      this->type = type;
      p = 0;
      i = 0;
      d = 0;
      Km = 0;
      Kn = 0;
      Ra = 0;
      Lm = 0;
      Jm = 0;
      Rm = 0;
      U = 0;
      gear = 0;
      max_current = 0;
      r_current = 0;
      max_val = M_PI;
      min_val = -M_PI;
    }

    bool MotorData::fromConfigMap(ConfigMap *config,
                                  std::string filenamePrefix,
                                  LoadCenter *loadCenter) {
      CPP_UNUSED(filenamePrefix);
      CPP_UNUSED(loadCenter);
      ConfigMap::iterator it;
      unsigned int mapIndex;
      GET_VALUE("mapIndex", mapIndex, UInt);

      GET_VALUE("name", name, String);
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
      GET_VALUE("maximumVelocity", maximumVelocity, Double);
      GET_VALUE("motorMaxForce", motorMaxForce, Double);

      int tmp;
      GET_VALUE("type", tmp, Int);
      type = (MotorType)tmp;

      GET_VALUE("p", p, Double);
      GET_VALUE("i", i, Double);
      GET_VALUE("d", d, Double);
      GET_VALUE("value", value, Double);
      GET_VALUE("Km", Km, Double);
      GET_VALUE("Kn", Kn, Double);
      GET_VALUE("Ra", Ra, Double);
      GET_VALUE("Lm", Lm, Double);
      GET_VALUE("Jm", Jm, Double);
      GET_VALUE("U", U, Double);
      GET_VALUE("gear", gear, Double);
      GET_VALUE("max_current", max_current, Double);
      GET_VALUE("r_current", r_current, Double);
      GET_VALUE("Rm", Rm, Double);
      GET_VALUE("max_val", max_val, Double);
      GET_VALUE("min_val", min_val, Double);

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
      SET_VALUE("maximumVelocity", maximumVelocity);
      SET_VALUE("motorMaxForce", motorMaxForce);

      (*config)["type"][0] = ConfigItem((int)type);

      SET_VALUE("p", p);
      SET_VALUE("i", i);
      SET_VALUE("d", d);
      SET_VALUE("value", value);
      SET_VALUE("Km", Km);
      SET_VALUE("Kn", Kn);
      SET_VALUE("Ra", Ra);
      SET_VALUE("Lm", Lm);
      SET_VALUE("Jm", Jm);
      SET_VALUE("U", U);
      SET_VALUE("gear", gear);
      SET_VALUE("max_current", max_current);
      SET_VALUE("r_current", r_current);
      SET_VALUE("Rm", Rm);
      SET_VALUE("max_val", max_val);
      SET_VALUE("min_val", min_val);
    }

    void MotorData::getFilesToSave(std::vector<std::string> *fileList) {
      CPP_UNUSED(fileList);
    }

  } // end of namespace interfaces
} // end of namespace mars
