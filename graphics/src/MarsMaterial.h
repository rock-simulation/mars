/*
 *  Copyright 20115, DFKI GmbH Robotics Innovation Center
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
 *  MarsMaterial.h
 *  General MarsMaterial to inherit from.
 *
 *  Created by Malte Langosz on 20.10.09.
 */

#ifndef MARS_GRAPHICS_MATERIAL_H
#define MARS_GRAPHICS_MATERIAL_H

#ifdef _PRINT_HEADER_
  #warning "MarsMaterial.h"
#endif

#include "3d_objects/DrawObject.h"

#include <mars/interfaces/MARSDefs.h>
#include <mars/utils/Vector.h>
#include <mars/utils/Quaternion.h>
#include <mars/interfaces/LightData.h>
#include <mars/interfaces/MaterialData.h>

#include <string>
#include <vector>
#include <list>
#include <map>

#include <osg/Material>
#include <osg/Group>
#include <osg/Uniform>
#include <osg/Texture2D>

#define COLOR_MAP_UNIT 0
#define NORMAL_MAP_UNIT 1
#define SHADOW_MAP_UNIT 2
#define BUMP_MAP_UNIT 3
#define NOISE_MAP_UNIT 4
#define TANGENT_UNIT 7
#define DEFAULT_UV_UNIT 0

#define SHADER_LIGHT_IS_SET                1 << 0
#define SHADER_LIGHT_IS_DIRECTIONAL        1 << 1
#define SHADER_LIGHT_IS_SPOT               1 << 2
#define SHADER_USE_SHADOW                  1 << 3
#define SHADER_USE_FOG                     1 << 4
#define SHADER_USE_NOISE                   1 << 5
#define SHADER_DRAW_LINE_LASER             1 << 6

#define SHADOW_SAMPLES 1
#define SHADOW_SAMPLES2 (SHADOW_SAMPLES*SHADOW_SAMPLES)

namespace mars {
  namespace graphics {

    class MarsMaterial {
    public:
      MarsMaterial(std::string resPath, int shadowTextureSize);
      virtual ~MarsMaterial();

      // the material struct can also contain a static texture (texture file)
      virtual void setMaterial(const mars::interfaces::MaterialData &mStruct);
      // can be used for dynamic textures
      virtual void setTexture(osg::Texture2D *texture);
      virtual void setNormalMap(const std::string &normalMap);
      virtual void setBumpMap(const std::string &bumpMap);
      typedef std::map<mars::interfaces::ShaderType, std::string> foo;
      void updateShader(bool reload=false);
      void edit(std::string key, std::string value);
      osg::StateSet* getStateSet() {
        return group_->getOrCreateStateSet();
      }

      osg::Group* getGroup() {
        return group_.get();
      }

      osg::ref_ptr<osg::Material> getMaterial() {
        return material_;
      }

      void setMaxNumLights(int n) {maxNumLights = n;}

      void setUseMARSShader(bool val);
      void setNoiseImage(osg::Image *i);
      void setShadowScale(float v);
      void setShadowSamples(int v);
      void addDrawObject(unsigned long id, DrawObject *d) {drawObjectMap[id] = d;}
      void removeDrawObject(unsigned long id) {drawObjectMap.erase(id);}
      inline const interfaces::MaterialData& getMaterialData() {return materialData;}

    protected:
      std::map<unsigned long, DrawObject*> drawObjectMap;
      osg::ref_ptr<osg::Program> lastProgram;
      osg::ref_ptr<osg::Uniform> normalMapUniform, bumpMapUniform;
      osg::ref_ptr<osg::Uniform> baseImageUniform, noiseMapUniform;
      osg::ref_ptr<osg::Uniform> bumpNorFacUniform;
      osg::ref_ptr<osg::Uniform> texScaleUniform;
      osg::ref_ptr<osg::Uniform> shadowScaleUniform;

      osg::ref_ptr<osg::Uniform> shadowSamplesUniform, invShadowSamplesUniform;
      osg::ref_ptr<osg::Uniform> invShadowTextureSizeUniform;

      osg::ref_ptr<osg::Group> group_;
      osg::ref_ptr<osg::Material> material_;
      osg::ref_ptr<osg::Texture2D> colorMap_;
      osg::ref_ptr<osg::Texture2D> normalMap_;
      osg::ref_ptr<osg::Texture2D> bumpMap_;
      osg::ref_ptr<osg::Texture2D> noiseMap_;

      bool hasShaderSources;
      bool useMARSShader;
      int maxNumLights;
      bool getLight;
      double invShadowTextureSize;
      std::string name, resPath;
      bool exists;
      interfaces::MaterialData materialData;
    }; // end of class DrawObject

  } // end of namespace graphics
} // end of namespace mars

#endif /* MARS_GRAPHICS_DRAW_OBJECT_H */
