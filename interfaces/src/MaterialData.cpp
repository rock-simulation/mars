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

#include "MaterialData.h"
//#include "utils.h"

#include <mars/utils/misc.h>

namespace mars {
  namespace interfaces {

    using namespace configmaps;
    using namespace mars::utils;
    int MaterialData::anonymCount = 1;
    bool MaterialData::operator==(const MaterialData& other) const {
      return (exists == other.exists) &&
        (ambientFront == other.ambientFront) &&
        (diffuseFront == other.diffuseFront) &&
        (specularFront == other.specularFront) &&
        (emissionFront == other.emissionFront) &&
        (ambientBack == other.ambientBack) &&
        (diffuseBack == other.diffuseBack) &&
        (specularBack == other.specularBack) &&
        (emissionBack == other.emissionBack) &&
        (transparency == other.transparency) &&
        (shininess == other.shininess) &&
        (texturename == other.texturename);
    }

    bool MaterialData::fromConfigMap(ConfigMap *config,
                                     std::string filenamePrefix) {
      ConfigMap::iterator it;

      exists = config->get("exists", exists);

      if((it = config->find("ambientFront")) != config->end())
        ambientFront.fromConfigItem(it->second);
      if((it = config->find("ambientColor")) != config->end()) // new SMURF naming
        ambientFront.fromConfigItem(it->second);
      if((it = config->find("ambientBack")) != config->end())
        ambientBack.fromConfigItem(it->second);

      if((it = config->find("diffuseFront")) != config->end())
        diffuseFront.fromConfigItem(it->second);
      if((it = config->find("diffuseColor")) != config->end()) // new SMURF naming
        diffuseFront.fromConfigItem(it->second);
      if((it = config->find("diffuseBack")) != config->end())
        diffuseBack.fromConfigItem(it->second);


      if((it = config->find("specularFront")) != config->end())
        specularFront.fromConfigItem(it->second);
      if((it = config->find("specularColor")) != config->end()) // new SMURF naming
        specularFront.fromConfigItem(it->second);
      if((it = config->find("specularBack")) != config->end())
        specularBack.fromConfigItem(it->second);

      if((it = config->find("emissionFront")) != config->end())
        emissionFront.fromConfigItem(it->second);
      if((it = config->find("emissionColor")) != config->end()) // new SMURF naming
        emissionFront.fromConfigItem(it->second);
      if((it = config->find("emissionBack")) != config->end())
        emissionBack.fromConfigItem(it->second);

      transparency = config->get("transparency", transparency);
      shininess = config->get("shininess", shininess);
      texturename = trim(config->get("texturename", texturename));
      texturename = trim(config->get("diffuseTexture", texturename)); // new SMURF naming
      bumpmap = trim(config->get("displacementmap", bumpmap));
      bumpmap = config->get("displacementTexture", bumpmap); // new SMURF naming
      normalmap = config->get("bumpmap", normalmap);
      normalmap = config->get("normalTexture", normalmap); // new SMURF naming
      tex_scale = config->get("tex_scale", tex_scale);
      reflect = config->get("reflect", reflect);
      brightness = config->get("brightness", brightness);
      getLight = config->get("getLight", getLight);
      cullMask = config->get("cullMask", cullMask);
      bumpNorFac = config->get("bumpNorFac", bumpNorFac);

      if((it = config->find("name")) != config->end())
        name = (std::string)it->second;
      else {
        std::stringstream s;
        s << "material" << anonymCount++ << std::endl;
        name = s.str();
      }

      if(!filenamePrefix.empty()) {
        handleFilenamePrefix(&texturename, filenamePrefix);
        handleFilenamePrefix(&bumpmap, filenamePrefix);
        handleFilenamePrefix(&normalmap, filenamePrefix);
      }

      return true;
    }

    void MaterialData::toConfigMap(ConfigMap *config, bool skipFilenamePrefix,
                                   bool exportDefault) {
      MaterialData defaultMaterial;
      std::string texturename_ = texturename;
      std::string bumpmap_ = bumpmap;
      std::string normalmap_ = normalmap;

      if(skipFilenamePrefix) {
        removeFilenamePrefix(&texturename_);
        removeFilenamePrefix(&bumpmap_);
        removeFilenamePrefix(&normalmap_);
      }

      (*config)["name"] = name;

      if(exportDefault || ambientFront != defaultMaterial.ambientFront) {
        ambientFront.toConfigItem((*config)["ambientColor"]);
      }
      if(ambientBack != defaultMaterial.ambientBack) {
        ambientBack.toConfigItem((*config)["ambientBack"]);
      }

      if(exportDefault || diffuseFront != defaultMaterial.diffuseFront) {
        diffuseFront.toConfigItem((*config)["diffuseColor"]);
      }
      if(diffuseBack != defaultMaterial.diffuseBack) {
        diffuseBack.toConfigItem((*config)["diffuseBack"]);
      }

      if(exportDefault || specularFront != defaultMaterial.specularFront) {
        specularFront.toConfigItem((*config)["specularColor"]);
      }
      if(specularBack != defaultMaterial.specularBack) {
        specularBack.toConfigItem((*config)["specularBack"]);
      }

      if(exportDefault || emissionFront != defaultMaterial.emissionFront) {
        emissionFront.toConfigItem((*config)["emissionColor"]);
      }
      if(emissionBack != defaultMaterial.emissionBack) {
        emissionBack.toConfigItem((*config)["emissionBack"]);
      }

      if(exportDefault || shininess != defaultMaterial.shininess)
        (*config)["shininess"] = shininess;
      if(exportDefault || transparency != defaultMaterial.transparency)
        (*config)["transparency"] = transparency;
      if(exportDefault || texturename_ != defaultMaterial.texturename)
        (*config)["diffuseTexture"] = texturename_;
      if(exportDefault || normalmap_ != defaultMaterial.normalmap)
        (*config)["normalTexture"] = normalmap_;
      // no default export because testing is needed
      if( bumpmap_ != defaultMaterial.bumpmap)
        (*config)["displacementTexture"] = bumpmap_;
      if(exportDefault || tex_scale != defaultMaterial.tex_scale)
        (*config)["tex_scale"] = tex_scale;
      // feature is currently not working (no default export)
      if(reflect != defaultMaterial.reflect)
        (*config)["reflect"] = reflect;
      if(exportDefault || brightness != defaultMaterial.brightness)
        (*config)["brightness"] = brightness;
      if(exportDefault || getLight != defaultMaterial.getLight)
        (*config)["getLigth"] = getLight;
      if(exportDefault || cullMask != defaultMaterial.cullMask)
        (*config)["cullMask"] = cullMask;
      if(exportDefault || bumpNorFac != defaultMaterial.bumpNorFac)
        (*config)["bumpNorFac"] = bumpNorFac;
      // can not be changed online currently
      if(exists != defaultMaterial.exists)
        (*config)["exists"] = exists;
    }

    void MaterialData::getFilesToSave(std::vector<std::string> *fileList) {
      if(!texturename.empty()) fileList->push_back(texturename);
      if(!bumpmap.empty()) fileList->push_back(bumpmap);
      if(!normalmap.empty()) fileList->push_back(normalmap);
    }

  } // end of namespace interfaces
} // end of namespace mars
