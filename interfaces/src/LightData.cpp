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

#include "LightData.h"
#include "MARSDefs.h"
#include <mars/utils/mathUtils.h>

#define GET_VALUE(str, val, type)                    \
  if((it = config->find(str)) != config->end())      \
  val = it->second

#define GET_OBJECT(str, val, type)              \
  if((it = config->find(str)) != config->end())      \
    type##FromConfigItem(it->second, &val);

#define SET_VALUE(str, val)                              \
    (*config)[str] = val

#define SET_OBJECT(str, val, type)                                      \
    type##ToConfigItem((*config)[str], &val);

namespace mars {
  namespace interfaces {

    using namespace mars::utils;
    using namespace configmaps;

    bool LightData::fromConfigMap(ConfigMap *config,
                                  std::string filenamePrefix,
                                  LoadCenter *loadCenter) {
      CPP_UNUSED(filenamePrefix);
      CPP_UNUSED(loadCenter);
      ConfigMap::iterator it;
      map = *config;
      name = config->get("name", name);
      GET_OBJECT("position", pos, vector);
      GET_OBJECT("lookat", lookAt, vector);
      if((it = config->find("ambient")) != config->end())
        ambient.fromConfigItem(it->second);
      if((it = config->find("diffuse")) != config->end())
        diffuse.fromConfigItem(it->second);
      if((it = config->find("specular")) != config->end())
        specular.fromConfigItem(it->second);
      GET_VALUE("constantAttenuation", constantAttenuation, Double);
      GET_VALUE("linearAttenuation", linearAttenuation, Double);
      GET_VALUE("quadraticAttenuation", quadraticAttenuation, Double);
      GET_VALUE("type", type, Int);
      GET_VALUE("angle", angle, Double);
      GET_VALUE("exponent", exponent, Double);
      GET_VALUE("directional", directional, Bool);
      node = config->get("nodeName", node);
      drawID = 0;
      return true;
    }

    void LightData::toConfigMap(ConfigMap *config,
                                bool skipFilenamePrefix) {
      CPP_UNUSED(skipFilenamePrefix);
      LightData defaultLight;

      *config = map;
      SET_VALUE("name", name);
      SET_OBJECT("position", pos, vector);
      SET_OBJECT("lookat", lookAt, vector);

      if(ambient != defaultLight.ambient) {
        ambient.toConfigItem((*config)["ambient"]);
      }

      if(diffuse != defaultLight.diffuse) {
        diffuse.toConfigItem((*config)["diffuse"]);
      }

      if(specular != defaultLight.specular) {
        specular.toConfigItem((*config)["specular"]);
      }

      SET_VALUE("constantAttenuation", constantAttenuation);
      SET_VALUE("linearAttenuation", linearAttenuation);
      SET_VALUE("quadraticAttenuation", quadraticAttenuation);
      SET_VALUE("type", type);
      SET_VALUE("angle", angle);
      SET_VALUE("exponent", exponent);
      SET_VALUE("directional", directional);
      SET_VALUE("nodeName", node);
      drawID = 0;
    }

    void LightData::getFilesToSave(std::vector<std::string> *fileList) {
    }

  } // end of namespace interfaces
} // end of namespace mars
