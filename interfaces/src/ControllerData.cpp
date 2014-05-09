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

#include "ControllerData.h"
#include "sim/LoadCenter.h"

#define GET_VALUE(str, val, type)                    \
  if((it = config->find(str)) != config->end())      \
  val = it->second[0].get##type()


#define SET_VALUE(str, val)                              \
  (*config)[str][0] = ConfigItem(val)

namespace mars {
  namespace interfaces {

    using namespace mars::utils;

    ControllerData::ControllerData() {
      rate = 20;
    }

    bool ControllerData::fromConfigMap(utils::ConfigMap *config,
                                       std::string filenamePrefix,
                                       LoadCenter *loadCenter) {
      CPP_UNUSED(filenamePrefix);
      CPP_UNUSED(loadCenter);
      ConfigMap::iterator it;
      std::vector<ConfigItem>::iterator it2;
      unsigned int mapIndex = (*config)["mapIndex"][0].getUInt();
      unsigned long _id;

      GET_VALUE("index", id, ULong);
      GET_VALUE("rate", rate, Double);
      GET_VALUE("dylib_path", dylib_path, String);

      if((it = config->find("sensorid")) != config->end()) {
        ConfigVector _ids = (*config)["sensorid"];
        for(it2=_ids.begin(); it2!=_ids.end(); ++it2) {
          if((_id = it2->getULong())){
            if(mapIndex) {
              _id = loadCenter->getMappedID(_id, MAP_TYPE_SENSOR, mapIndex);
            }
            sensors.push_back(_id);
          }
        }
      }

      if((it = config->find("motorid")) != config->end()) {
        ConfigVector _ids = (*config)["motorid"];
        for(it2=_ids.begin(); it2!=_ids.end(); ++it2) {
          if((_id = it2->getULong())){
            if(mapIndex) {
              _id = loadCenter->getMappedID(_id, MAP_TYPE_MOTOR, mapIndex);
            }
            motors.push_back(_id);
          }
        }
      }

      if((it = config->find("nodeid")) != config->end()) {
        ConfigVector _ids = (*config)["nodeid"];
        for(it2=_ids.begin(); it2!=_ids.end(); ++it2) {
          if((_id = it2->getULong())){
            if(mapIndex) {
              _id = loadCenter->getMappedID(_id, MAP_TYPE_NODE, mapIndex);
            }
            sNodes.push_back(_id);
          }
        }
      }

      return true;
    }

    void ControllerData::toConfigMap(utils::ConfigMap *config,
                                     bool skipFilenamePrefix) {
      CPP_UNUSED(skipFilenamePrefix);
      std::vector<unsigned long>::iterator it;

      SET_VALUE("index", id);
      SET_VALUE("rate", rate);
      SET_VALUE("dylib_path", dylib_path);

      for(it=sensors.begin(); it!=sensors.end(); ++it) {
        (*config)["sensorid"].push_back(ConfigItem(*it));
      }
      for(it=motors.begin(); it!=motors.end(); ++it) {
        (*config)["motorid"].push_back(ConfigItem(*it));
      }
      for(it=sNodes.begin(); it!=sNodes.end(); ++it) {
        (*config)["nodeid"].push_back(ConfigItem(*it));
      }
    }

    void ControllerData::getFilesToSave(std::vector<std::string> *fileList) {

    }

  } // end of namespace interfaces
} // end of namespace mars
