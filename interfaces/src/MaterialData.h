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

#ifndef MARS_INTERFACES_MATERIAL_STRUCT_H
#define MARS_INTERFACES_MATERIAL_STRUCT_H

#include <mars/utils/Color.h>
#include <configmaps/ConfigData.h>

#include <sstream>

namespace mars {
  namespace interfaces {

    /**
     * MaterialData is a struct to exchange material information of nodes
     */
    class MaterialData {
    public:
      MaterialData(){
        setZero();
      }

      void init(){
        setZero();
      }

      void setZero(){
        exists = 1;
        ambientFront.setZero();
        diffuseFront.setZero();
        specularFront.setZero();
        emissionFront.setZero();
        ambientBack.setZero();
        diffuseBack.setZero();
        specularBack.setZero();
        emissionBack.setZero();
        transparency = 0;
        shininess = 0;
        reflect = 0;
        texturename = "";
        bumpmap = "";
        normalmap = "";
        tex_scale = 1.0;
        brightness = 0.0;
        getLight = true;
        cullMask = 0xffffffff;
        bumpNorFac = 1.0;
        std::stringstream s;
        s << "material" << anonymCount++;
        name = s.str();
      }

      // todo: handle filenames
      bool fromConfigMap(configmaps::ConfigMap *config, std::string filenamePrefix);
      void toConfigMap(configmaps::ConfigMap *config,
                       bool skipFilenamePrefix = false,
                       bool exportDefault = false);
      void getFilesToSave(std::vector<std::string> *fileList);

      /** Compare with other material (usually compared with default material) */
      bool operator==(const MaterialData& other) const;

      bool exists; // used for import mesh
      utils::Color ambientFront;  // ambient color
      utils::Color diffuseFront;  // diffuse color
      utils::Color specularFront; // specular color
      utils::Color emissionFront; // emission color
      utils::Color ambientBack;   // ambient color
      utils::Color diffuseBack;   // diffuse color
      utils::Color specularBack;  // specular color
      utils::Color emissionBack;  // emission color
      double transparency;
      double shininess;
      std::string texturename; // the filename of the texture of the node
      std::string bumpmap; // the filename for a bump- / displacementmap
      std::string normalmap; // the filename for a normalmap
      double bumpNorFac;
      double tex_scale;
      bool reflect;
      double brightness;
      bool getLight;
      int cullMask;
      std::string name;

      static int anonymCount;
    }; // end of struct MaterialData

  } // end of namespace interfaces
} // end of namespace mars

#endif /* MARS_INTERFACES_MATERIAL_STRUCT_H */

