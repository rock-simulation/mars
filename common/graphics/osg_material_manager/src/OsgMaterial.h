/*
 *  Copyright 2016, DFKI GmbH Robotics Innovation Center
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
 *  OsgMaterial.h
 *  General OsgMaterial to inherit from.
 *
 *  Created by Malte Langosz on 20.10.09.
 */

#ifndef OSG_MATERIAL_H
#define OSG_MATERIAL_H

#ifdef _PRINT_HEADER_
  #warning "OsgMaterial.h"
#endif

#include <configmaps/ConfigData.h>

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

namespace osg_material_manager {

  class MaterialNode;

  class TextureInfo {
  public:
    osg::ref_ptr<osg::Texture2D> texture;
    osg::ref_ptr<osg::Uniform> textureUniform;
    std::string name;
    int unit;
    bool enabled;
    // maybe define more attributes like filter
  };

  class OsgMaterial : public osg::Group {
  public:
    OsgMaterial(std::string resPath);
    virtual ~OsgMaterial();

    // the material struct can also contain a static texture (texture file)
    void setMaterial(const configmaps::ConfigMap &map);
    // can be used for dynamic textures
    void setTexture(osg::Texture2D *texture);

    void setNormalMap(const std::string &normalMap);
    void setBumpMap(const std::string &bumpMap);
    void updateShader(bool reload=false);
    void edit(const std::string &key, const std::string &value);

    void setMaxNumLights(int n);

    void setUseShader(bool val);
    void setNoiseImage(osg::Image *i);
    void setShadowScale(float v);
    void setShadowSamples(int v);
    void addMaterialNode(MaterialNode *d);
    void removeMaterialNode(MaterialNode *d);
    void setShadowTextureSize(int size);
    inline configmaps::ConfigMap getMaterialData() {return map;}
    void update();
    void addTexture(configmaps::ConfigMap &config, bool nearest=false);
    void disableTexture(std::string name);
    void enableTexture(std::string name);
    bool checkTexture(std::string name);

  protected:
    std::vector<osg::ref_ptr<MaterialNode> > materialNodeVector;

    osg::ref_ptr<osg::Program> lastProgram;
    osg::ref_ptr<osg::Uniform> noiseMapUniform;
    osg::ref_ptr<osg::Uniform> bumpNorFacUniform;
    osg::ref_ptr<osg::Uniform> texScaleUniform;
    osg::ref_ptr<osg::Uniform> sinUniform;
    osg::ref_ptr<osg::Uniform> cosUniform;
    osg::ref_ptr<osg::Uniform> shadowScaleUniform;

    osg::ref_ptr<osg::Uniform> shadowSamplesUniform, invShadowSamplesUniform;
    osg::ref_ptr<osg::Uniform> invShadowTextureSizeUniform;
    osg::ref_ptr<osg::Uniform> envMapSpecularUniform;
    osg::ref_ptr<osg::Uniform> envMapScaleUniform;
    osg::ref_ptr<osg::Uniform> terrainScaleZUniform;

    osg::ref_ptr<osg::Group> group;
    osg::ref_ptr<osg::Material> material;
    osg::ref_ptr<osg::Texture2D> noiseMap;

    // new implementation for generic texture handling
    std::map<std::string, TextureInfo> textures;

    bool hasShaderSources;
    bool useShader;
    int maxNumLights;
    bool getLight;
    double invShadowTextureSize;
    bool useWorldTexCoords;
    double t;
    std::string name, resPath;
    configmaps::ConfigMap map, unitMap;

    osg::Vec4 getColor(std::string key);
    void setColor(std::string color, std::string key, std::string value);
    osg::Texture2D* loadTerrainTexture(std::string filename);
}; // end of class OsgMaterial

} // end of namespace osg_material_manager

#endif /* OSG_MATERIAL_MANAGER_H */
