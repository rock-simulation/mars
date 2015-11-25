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

#include "GraphicData.h"

#include <mars/utils/mathUtils.h>

#define GET_VALUE(str, val, type)                    \
  if((it = config->find(str)) != config->end())      \
    val = it->second

#define SET_VALUE(str, val)                              \
    (*config)[str] = val

namespace mars {
  namespace interfaces {

    using namespace mars::utils;
    using namespace configmaps;

    GraphicData::GraphicData() {
      clearColor.r = 0.25;
      clearColor.g = 0.27;
      clearColor.b = 0.48;
      clearColor.a = 1.0;

      fogEnabled = false;
      fogDensity = 0.35;
      fogStart = 10.0;
      fogEnd = 30.0;

      fogColor.r = 0.2;
      fogColor.g = 0.2;
      fogColor.b = 0.2;
      fogColor.a = 1.0;
    }

    bool GraphicData::fromConfigMap(ConfigMap *config,
                                    std::string filenamePrefix,
                                    LoadCenter *loadCenter) {
      CPP_UNUSED(filenamePrefix);
      CPP_UNUSED(loadCenter);
      ConfigMap::iterator it;

      if((it = config->find("clearColor")) != config->end())
        clearColor.fromConfigItem(it->second);

      GET_VALUE("fogEnabled", fogEnabled, Bool);
      GET_VALUE("fogDensity", fogDensity, Double);
      GET_VALUE("fogStart", fogStart, Double);
      GET_VALUE("fogEnd", fogEnd, Double);

      if((it = config->find("fogColor")) != config->end())
        fogColor.fromConfigItem(it->second);

      return true;
    }

    void GraphicData::toConfigMap(ConfigMap *config,
                                  bool skipFilenamePrefix) {
      CPP_UNUSED(skipFilenamePrefix);
      
      clearColor.toConfigItem((*config)["clearColor"]);
      
      SET_VALUE("fogEnabled", fogEnabled);
      SET_VALUE("fogDensity", fogDensity);
      SET_VALUE("fogStart", fogStart);
      SET_VALUE("fogEnd", fogEnd);

      fogColor.toConfigItem((*config)["fogColor"]);

    }

    void GraphicData::getFilesToSave(std::vector<std::string> *fileList) {

    }

  } // end of namespace interfaces
} // end of namespace mars
