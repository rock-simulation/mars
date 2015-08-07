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

      if((it = config->find("exists")) != config->end())
        exists = it->second[0].getBool();

      if((it = config->find("ambientFront")) != config->end())
        ambientFront.fromConfigItem(&it->second[0]);
      if((it = config->find("ambientColor")) != config->end())
        ambientFront.fromConfigItem(&it->second[0]);
      if((it = config->find("ambientBack")) != config->end())
        ambientBack.fromConfigItem(&it->second[0]);

      if((it = config->find("diffuseFront")) != config->end())
        diffuseFront.fromConfigItem(&it->second[0]);
      if((it = config->find("diffuseColor")) != config->end())
        diffuseFront.fromConfigItem(&it->second[0]);
      if((it = config->find("diffuseBack")) != config->end())
        diffuseBack.fromConfigItem(&it->second[0]);


      if((it = config->find("specularFront")) != config->end())
        specularFront.fromConfigItem(&it->second[0]);
      if((it = config->find("specularColor")) != config->end())
        specularFront.fromConfigItem(&it->second[0]);
      if((it = config->find("specularBack")) != config->end())
        specularBack.fromConfigItem(&it->second[0]);

      if((it = config->find("emissionFront")) != config->end())
        emissionFront.fromConfigItem(&it->second[0]);
      if((it = config->find("emissionColor")) != config->end())
        emissionFront.fromConfigItem(&it->second[0]);
      if((it = config->find("emissionBack")) != config->end())
        emissionBack.fromConfigItem(&it->second[0]);

      if((it = config->find("transparency")) != config->end())
        transparency = it->second[0].getDouble();
      if((it = config->find("shininess")) != config->end())
        shininess = it->second[0].getDouble();
      if((it = config->find("texturename")) != config->end())
        texturename = trim(it->second[0].getString());
      if((it = config->find("displacementmap")) != config->end())
        bumpmap = trim(it->second[0].getString());
      if((it = config->find("bumpmap")) != config->end())
        normalmap = trim(it->second[0].getString());
      if((it = config->find("tex_scale")) != config->end())
        tex_scale = it->second[0].getDouble();
      if((it = config->find("reflect")) != config->end())
        reflect = it->second[0].getDouble();
      if((it = config->find("brightness")) != config->end())
        brightness = it->second[0].getDouble();
      if((it = config->find("getLight")) != config->end())
        getLight = it->second[0].getBool();
      if((it = config->find("cullMask")) != config->end())
        cullMask = it->second[0].getInt();
      if((it = config->find("bumpNorFac")) != config->end())
        bumpNorFac = it->second[0].getDouble();
      if((it = config->find("name")) != config->end())
        name = it->second[0].getString();
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

    void MaterialData::toConfigMap(ConfigMap *config, bool skipFilenamePrefix) {
      MaterialData defaultMaterial;
      std::string texturename_ = texturename;
      std::string bumpmap_ = bumpmap;
      std::string normalmap_ = normalmap;

      if(skipFilenamePrefix) {
        removeFilenamePrefix(&texturename_);
        removeFilenamePrefix(&bumpmap_);
        removeFilenamePrefix(&normalmap_);
      }

      if(exists != defaultMaterial.exists)
        (*config)["exists"][0] = ConfigItem(exists);

      if(ambientFront != defaultMaterial.ambientFront) {
        (*config)["ambientFront"][0] = ConfigItem(std::string());
        ambientFront.toConfigItem(&(*config)["ambientFront"][0]);
      }
      if(ambientBack != defaultMaterial.ambientBack) {
        (*config)["ambientBack"][0] = ConfigItem(std::string());
        ambientBack.toConfigItem(&(*config)["ambientBack"][0]);
      }

      if(diffuseFront != defaultMaterial.diffuseFront) {
        (*config)["diffuseFront"][0] = ConfigItem(std::string());
        diffuseFront.toConfigItem(&(*config)["diffuseFront"][0]);
      }
      if(diffuseBack != defaultMaterial.diffuseBack) {
        (*config)["diffuseBack"][0] = ConfigItem(std::string());
        diffuseBack.toConfigItem(&(*config)["diffuseBack"][0]);
      }

      if(specularFront != defaultMaterial.specularFront) {
        (*config)["specularFront"][0] = ConfigItem(std::string());
        specularFront.toConfigItem(&(*config)["specularFront"][0]);
      }
      if(specularBack != defaultMaterial.specularBack) {
        (*config)["specularBack"][0] = ConfigItem(std::string());
        specularBack.toConfigItem(&(*config)["specularBack"][0]);
      }

      if(emissionFront != defaultMaterial.emissionFront) {
        (*config)["emissionFront"][0] = ConfigItem(std::string());
        emissionFront.toConfigItem(&(*config)["emissionFront"][0]);
      }
      if(emissionBack != defaultMaterial.emissionBack) {
        (*config)["emissionBack"][0] = ConfigItem(std::string());
        emissionBack.toConfigItem(&(*config)["emissionBack"][0]);
      }

      if(transparency != defaultMaterial.transparency)
        (*config)["transparency"][0] = ConfigItem(transparency);
      if(shininess != defaultMaterial.shininess)
        (*config)["shininess"][0] = ConfigItem(shininess);
      if(texturename_ != defaultMaterial.texturename)
        (*config)["texturename"][0] = ConfigItem(texturename_);
      if(bumpmap_ != defaultMaterial.bumpmap)
        (*config)["displacementmap"][0] = ConfigItem(bumpmap_);
      if(normalmap_ != defaultMaterial.normalmap)
        (*config)["bumpmap"][0] = ConfigItem(normalmap_);
      if(tex_scale != defaultMaterial.tex_scale)
        (*config)["tex_scale"][0] = ConfigItem(tex_scale);
      if(reflect != defaultMaterial.reflect)
        (*config)["reflect"][0] = ConfigItem(reflect);
      if(brightness != defaultMaterial.brightness)
        (*config)["brightness"][0] = ConfigItem(brightness);
      if(getLight != defaultMaterial.getLight)
        (*config)["getLigth"][0] = ConfigItem(getLight);
      if(cullMask != defaultMaterial.cullMask)
        (*config)["cullMask"][0] = ConfigItem(cullMask);
      if(bumpNorFac != defaultMaterial.bumpNorFac)
        (*config)["bumpNorFac"][0] = ConfigItem(bumpNorFac);
      (*config)["name"][0] = ConfigItem(name);
    }

    void MaterialData::getFilesToSave(std::vector<std::string> *fileList) {
      if(!texturename.empty()) fileList->push_back(texturename);
      if(!bumpmap.empty()) fileList->push_back(bumpmap);
      if(!normalmap.empty()) fileList->push_back(normalmap);
    }

  } // end of namespace interfaces
} // end of namespace mars
