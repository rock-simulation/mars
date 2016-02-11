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
 *  OsgMaterial.cpp
 *  General OsgMaterial to inherit from.
 *
 *  Created by Langosz on 2016
 */

#include <mars/utils/misc.h>
#include "OsgMaterial.h"
#include "OsgMaterialManager.h"
#include "MaterialNode.h"

#include "shader/shader-generator.h"
#include "shader/shader-function.h"
#include "shader/bumpmapping.h"
#include "shader/pixellight.h"

#include <osg/TexMat>
#include <osg/CullFace>

namespace osg_material_manager {

  using namespace std;
  using namespace mars::utils;

  OsgMaterial::OsgMaterial(std::string resPath)
    : material(0),
      colorMap(0),
      normalMap(0),
      bumpMap(0),
      hasShaderSources(false),
      useShader(true),
      maxNumLights(1),
      resPath(resPath),
      invShadowTextureSize(1./1024) {
    baseImageUniform = new osg::Uniform("BaseImage", COLOR_MAP_UNIT);
    normalMapUniform = new osg::Uniform("NormalMap", NORMAL_MAP_UNIT);
    bumpMapUniform = new osg::Uniform("BumpMap", BUMP_MAP_UNIT);
    noiseMapUniform = new osg::Uniform("NoiseMap", NOISE_MAP_UNIT);
    texScaleUniform = new osg::Uniform("texScale", 1.0f);
    shadowScaleUniform = new osg::Uniform("shadowScale", 0.5f);
    bumpNorFacUniform = new osg::Uniform("bumpNorFac", 1.0f);
    shadowSamplesUniform = new osg::Uniform("shadowSamples", 1);
    invShadowSamplesUniform = new osg::Uniform("invShadowSamples",
                                               1.f/1);
    invShadowTextureSizeUniform = new osg::Uniform("invShadowTextureSize",
                                                   (float)(invShadowTextureSize));

    noiseMap = new osg::Texture2D();
    noiseMap->setDataVariance(osg::Object::DYNAMIC);
    noiseMap->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
    noiseMap->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
    noiseMap->setWrap(osg::Texture::WRAP_R, osg::Texture::REPEAT);
    noiseMap->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
    noiseMap->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
  }

  OsgMaterial::~OsgMaterial() {
  }

  osg::Vec4 OsgMaterial::getColor(string key) {
    osg::Vec4 c(0, 0, 0, 1);
    if(map.hasKey(key)) {
      configmaps::ConfigMap &m = map[key];
      c[0] = m.get("r", 0.0);
      c[1] = m.get("g", 0.0);
      c[2] = m.get("b", 0.0);
      c[3] = m.get("a", 1.0);
    }
    return c;
  }
  // the material struct can also contain a static texture (texture file)
  void OsgMaterial::setMaterial(const configmaps::ConfigMap &map_) {
    //return;
    map = map_;
    name << map["name"];
    getLight = map.get("getLight", true);

    // create the osg::Material
    material = new osg::Material();
    material->setColorMode(osg::Material::OFF);
    
    material->setAmbient(osg::Material::FRONT_AND_BACK, getColor("ambientColor"));
    material->setSpecular(osg::Material::FRONT_AND_BACK, getColor("specularColor"));
    material->setDiffuse(osg::Material::FRONT_AND_BACK, getColor("diffuseColor"));
    material->setEmission(osg::Material::FRONT_AND_BACK, getColor("emissionColor"));
    material->setShininess(osg::Material::FRONT_AND_BACK, map.get("shininess", 0.0));
    material->setTransparency(osg::Material::FRONT_AND_BACK, map.get("transparency", 0.0));

    // get the StateSet of the Object
    osg::StateSet *state = getOrCreateStateSet();

    // set the material
    state->setAttributeAndModes(material.get(), osg::StateAttribute::ON);

    if(!getLight) {
      osg::ref_ptr<osg::CullFace> cull = new osg::CullFace();
      cull->setMode(osg::CullFace::BACK);
      state->setAttributeAndModes(cull.get(), osg::StateAttribute::OFF);
      state->setMode(GL_LIGHTING,
                     osg::StateAttribute::OFF);
      state->setMode(GL_FOG, osg::StateAttribute::OFF);
    }

    string texturename = map.get("diffuseTexture", std::string());
    double tex_scale = map.get("tex_scale", 1.0);
    texScaleUniform->set((float)tex_scale);
    if (!texturename.empty()) {
      colorMap = OsgMaterialManager::loadTexture(texturename);
      state->setTextureAttributeAndModes(COLOR_MAP_UNIT, colorMap,
                                         osg::StateAttribute::ON);
      
      if(tex_scale != 1.0) {
        osg::ref_ptr<osg::TexMat> scaleTexture = new osg::TexMat();
        scaleTexture->setMatrix(osg::Matrix::scale(tex_scale, tex_scale, tex_scale));
        state->setTextureAttributeAndModes(COLOR_MAP_UNIT, scaleTexture.get(),
                                           osg::StateAttribute::ON);
      }
    }
    else {
      if(colorMap.valid()) {
        state->setTextureAttributeAndModes(COLOR_MAP_UNIT, colorMap,
                                           osg::StateAttribute::OFF);
        colorMap = 0;
      }
    }
    bool generateTangents = false;
    texturename = map.get("normalTexture", std::string());
    if (!texturename.empty()) {
      generateTangents = true;
      setNormalMap(texturename);
      texturename = map.get("displacementTexture", std::string());
      if(!texturename.empty()) {
        setBumpMap(texturename);
      }
      else if(bumpMap.valid()) {
        bumpMap = 0;
      }
      bumpNorFacUniform->set((float)map.get("bumpNorFac", 1.0));
    }
    else {
      state->setTextureAttributeAndModes(NORMAL_MAP_UNIT, normalMap,
                                         osg::StateAttribute::OFF);
      normalMap = 0;
    }
    updateShader(true);

    float transparency = (float)map.get("transparency", 0.0);
    std::vector<osg::ref_ptr<MaterialNode> >::iterator it = materialNodeVector.begin();
    for(; it!=materialNodeVector.end(); ++it) {
      if(generateTangents) {
        (*it)->setNeedTangents(true);
        (*it)->generateTangents();
      }
      (*it)->setTransparency(transparency);
    }
  }
  void OsgMaterial::setColor(string color, string key, string value) {
    double v = atof(value.c_str());
    if(key[key.size()-1] == 'a') map[color]["a"] = v;
    else if(key[key.size()-1] == 'r') map[color]["r"] = v;
    else if(key[key.size()-1] == 'g') map[color]["g"] = v;
    else if(key[key.size()-1] == 'b') map[color]["b"] = v;
    setMaterial(map);
  }
  
  void OsgMaterial::edit(const std::string &key, const std::string &value) {
    if(matchPattern("*/ambientColor/*", key) ||
       matchPattern("*/ambientFront/*", key)) {
      setColor("ambientColor", key, value);
    }
    else if(matchPattern("*/diffuseColor/*", key) ||
            matchPattern("*/diffuseFront/*", key)) {
      setColor("diffuseColor", key, value);
    }
    if(matchPattern("*/specularColor/*", key) ||
       matchPattern("*/specularFront/*", key)) {
      setColor("specularColor", key, value);
    }
    if(matchPattern("*/emissionColor/*", key) ||
       matchPattern("*/emissionFront/*", key)) {
      setColor("emissionColor", key, value);
    }

    if(matchPattern("*/diffuseTexture", key) ||
       matchPattern("*/texturename", key)) {
      if(value == "") {
        map["diffuseTexture"] = string();
        fprintf(stderr, "edit material: %s\n", map["diffuseTexture"].c_str());
      }
      else if(pathExists(value)) {
        map["diffuseTexture"] = value;
        fprintf(stderr, "edit material: %s\n", map["diffuseTexture"].c_str());
      }
      setMaterial(map);
    }
    if(matchPattern("*/normalTexture", key) ||
       matchPattern("*/bumpmap", key)) {
      if(value == "") {
        map["normalTexture"] = string();
        fprintf(stderr, "edit material: %s\n", map["normalTexture"].c_str());
      }
      else if(pathExists(value)) {
        map["normalTexture"] = value;
        fprintf(stderr, "edit material: %s\n", map["normalTexture"].c_str());
      }
      setMaterial(map);
    }
    if(matchPattern("*/displacementTexture", key) ||
       matchPattern("*/displacementmap", key)) {
      if(value == "") {
        map["displacementTexture"] = string();
      }
      else if(pathExists(value)) {
        map["displacementTexture"] = value;
      }
      setMaterial(map);
    }
    if(matchPattern("*/bumpNorFac", key)) {
      map["bumpNorFac"] = atof(value.c_str());
      setMaterial(map);
    }
    if(matchPattern("*/shininess", key)) {
      map["shininess"] = atof(value.c_str());
      setMaterial(map);
    }
    if(matchPattern("*/transparency", key)) {
      map["transparency"] = atof(value.c_str());
      setMaterial(map);
    }
    if(matchPattern("*/tex_scale", key)) {
      map["tex_scale"] = atof(value.c_str());
      setMaterial(map);
    }
  }

  void OsgMaterial::setTexture(osg::Texture2D *texture) {
    colorMap = texture;
    osg::StateSet *state = getOrCreateStateSet();
    state->setTextureAttributeAndModes(COLOR_MAP_UNIT, colorMap,
                                       osg::StateAttribute::ON);
  }

  void OsgMaterial::setBumpMap(const std::string &filename) {
    bumpMap = OsgMaterialManager::loadTexture( filename.c_str() );
    bumpMap->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR );
    bumpMap->setFilter( osg::Texture::MAG_FILTER, osg::Texture::LINEAR );
    bumpMap->setWrap( osg::Texture::WRAP_S, osg::Texture::REPEAT );
    bumpMap->setWrap( osg::Texture::WRAP_T, osg::Texture::REPEAT );
    bumpMap->setMaxAnisotropy(8);

    osg::StateSet *state = getOrCreateStateSet();
    state->setTextureAttributeAndModes(BUMP_MAP_UNIT, bumpMap,
                                       osg::StateAttribute::ON);
  }

  void OsgMaterial::setNormalMap(const std::string &filename) {
    normalMap = OsgMaterialManager::loadTexture( filename.c_str() );
    normalMap->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR );
    normalMap->setFilter( osg::Texture::MAG_FILTER, osg::Texture::LINEAR );
    normalMap->setWrap( osg::Texture::WRAP_S, osg::Texture::REPEAT );
    normalMap->setWrap( osg::Texture::WRAP_T, osg::Texture::REPEAT );
    normalMap->setMaxAnisotropy(8);

    osg::StateSet *state = getOrCreateStateSet();
    state->setTextureAttributeAndModes(NORMAL_MAP_UNIT, normalMap,
                                       osg::StateAttribute::ON);
  }

  void OsgMaterial::setUseShader(bool val) {
    fprintf(stderr, "use shader: %d %d\n", useShader, val);
    if(useShader != val) {
      useShader = val;
      updateShader(true);
    }
  }

  void OsgMaterial::setShadowScale(float v) {
    shadowScaleUniform->set(1.f/(v*v));
  }

  void OsgMaterial::updateShader(bool reload) {
    osg::StateSet* stateSet = getOrCreateStateSet();

    //return;
    if(!useShader || !getLight) {
      if(lastProgram.valid()) {
        stateSet->removeAttribute(lastProgram.get());
        lastProgram = NULL;
      }
      if(normalMap.valid()) {
        stateSet->setTextureAttributeAndModes(NORMAL_MAP_UNIT, normalMap,
                                              osg::StateAttribute::OFF);
      }
      if(bumpMap.valid()) {
        stateSet->setTextureAttributeAndModes(BUMP_MAP_UNIT, bumpMap,
                                              osg::StateAttribute::OFF);
      }
      stateSet->setTextureAttributeAndModes(NOISE_MAP_UNIT, noiseMap,
                                            osg::StateAttribute::OFF);
      return;
    }
    if(normalMap.valid()) {
      stateSet->setTextureAttributeAndModes(NORMAL_MAP_UNIT, normalMap,
                                            osg::StateAttribute::ON);
    }
    if(bumpMap.valid()) {
      stateSet->setTextureAttributeAndModes(BUMP_MAP_UNIT, bumpMap,
                                            osg::StateAttribute::ON);
    }

    if(!reload && hasShaderSources) {
      // no need to regenerate, shader source did not changed
      return;
    }
    hasShaderSources = true;
    stateSet->setTextureAttributeAndModes(NOISE_MAP_UNIT, noiseMap,
                                          osg::StateAttribute::ON);
    ShaderGenerator shaderGenerator;
    vector<string> args;

    bool hasTexture = (colorMap.valid() || normalMap.valid());
    {
      ShaderFunc *vertexShader = new ShaderFunc;
      vertexShader->addExport( (GLSLExport)
                               {"gl_Position", "gl_ModelViewProjectionMatrix * gl_Vertex"} );
      vertexShader->addExport( (GLSLExport) {"gl_ClipVertex", "v1"} );
      if(hasTexture) {
        vertexShader->addExport( (GLSLExport)
                                 { "gl_TexCoord[0].xy", "gl_MultiTexCoord0.xy" });
      }
      shaderGenerator.addShaderFunction(vertexShader, SHADER_TYPE_VERTEX);
    }

    {
      ShaderFunc *fragmentShader = new ShaderFunc;
      if(hasTexture) {
        fragmentShader->addUniform( (GLSLUniform)
                                    { "float", "texScale" } );
        if(bumpMap.valid()) {
          fragmentShader->addUniform( (GLSLUniform)
                                      { "sampler2D", "BumpMap" } );
          fragmentShader->addMainVar( (GLSLVariable)
                                      { "vec2", "texCoord", "gl_TexCoord[0].xy*texScale + texture2D(BumpMap, gl_TexCoord[0].xy*texScale).x*0.01*eyeVec.xy" });

        }
        else {
          fragmentShader->addMainVar( (GLSLVariable)
                                      { "vec2", "texCoord", "gl_TexCoord[0].xy*texScale" });
        }
      }
      if(colorMap.valid()) {
        fragmentShader->addUniform( (GLSLUniform)
                                    { "sampler2D", "BaseImage" } );
        fragmentShader->addMainVar( (GLSLVariable)
                                    { "vec4", "col", "texture2D(BaseImage, texCoord)" });
      }
      else {
        fragmentShader->addMainVar( (GLSLVariable)
                                    { "vec4", "col", "vec4(1.0)" });
      }
      fragmentShader->addExport( (GLSLExport) {"gl_FragColor", "col"} );
      shaderGenerator.addShaderFunction(fragmentShader, SHADER_TYPE_FRAGMENT);
    }

    args.clear();
    args.push_back("v");
    PixelLightVert *plightVert = new PixelLightVert(args, maxNumLights,
                                                    resPath);
    plightVert->addMainVar( (GLSLVariable)
                            { "vec4", "v1", "gl_ModelViewMatrix * gl_Vertex" } );
    plightVert->addMainVar( (GLSLVariable)
                            { "vec4", "v", "osg_ViewMatrixInverse * v1" } );
    plightVert->addMainVar( (GLSLVariable)
                            { "vec4", "n", "normalize(osg_ViewMatrixInverse * vec4(gl_NormalMatrix * gl_Normal, 0.0))" } );

    plightVert->addExport( (GLSLExport)
                           { "normalVarying", "n.xyz" } );

    plightVert->addExport( (GLSLExport)
                           { "positionVarying", "v" } );
    plightVert->addExport( (GLSLExport)
                           { "modelVertex", "gl_Vertex" } );

    plightVert->addVarying( (GLSLVarying)
                            { "vec4", "positionVarying" } );
    plightVert->addVarying( (GLSLVarying)
                            { "vec3", "normalVarying" } );
    plightVert->addVarying( (GLSLVarying)
                            { "vec4", "modelVertex" } );
    shaderGenerator.addShaderFunction(plightVert, SHADER_TYPE_VERTEX);

    if(normalMap.valid()) {
      args.clear();
      args.push_back("n.xyz");
      BumpMapVert *bumpVert = new BumpMapVert(args, resPath);
      shaderGenerator.addShaderFunction(bumpVert, SHADER_TYPE_VERTEX);

      args.clear();
      args.push_back("texture2D( NormalMap, texCoord )");
      args.push_back("n");
      args.push_back("n");
      BumpMapFrag *bumpFrag = new BumpMapFrag(args, resPath);
      bumpFrag->addUniform( (GLSLUniform) { "sampler2D", "NormalMap" } );
      shaderGenerator.addShaderFunction(bumpFrag, SHADER_TYPE_FRAGMENT);
    }

    args.clear();
    args.push_back("col");
    args.push_back("n");
    args.push_back("col");
    PixelLightFrag *plightFrag = new PixelLightFrag(args, maxNumLights,
                                                    resPath);
    // invert the normal if gl_FrontFacing=true to handle back faces
    // correctly.
    // TODO: check not needed if backfaces not processed.
    plightFrag->addMainVar( (GLSLVariable)
                            { "vec3", "n", "normalize( gl_FrontFacing ? normalVarying : -normalVarying )"} );
    plightFrag->addVarying( (GLSLVarying)
                            { "vec3", "normalVarying" } );
    shaderGenerator.addShaderFunction(plightFrag, SHADER_TYPE_FRAGMENT);

    osg::Program *glslProgram = shaderGenerator.generate();

    if(normalMap.valid()) {
      glslProgram->addBindAttribLocation( "vertexTangent", TANGENT_UNIT );
    }

    stateSet->addUniform(noiseMapUniform.get());

    if(normalMap.valid()) {
      stateSet->addUniform(normalMapUniform.get());
      stateSet->addUniform(bumpNorFacUniform.get());
    }
    else {
      stateSet->removeUniform(normalMapUniform.get());
      stateSet->removeUniform(bumpNorFacUniform.get());
    }

    if(bumpMap.valid()) {
      stateSet->addUniform(bumpMapUniform.get());
    }
    else {
      stateSet->removeUniform(bumpMapUniform.get());
    }

    if(colorMap.valid() || normalMap.valid() || bumpMap.valid()) {
      stateSet->addUniform(texScaleUniform.get());
    }
    else {
      stateSet->removeUniform(texScaleUniform.get());
    }
    if(colorMap.valid()) {
      stateSet->addUniform(baseImageUniform.get());
    }
    else {
      stateSet->removeUniform(baseImageUniform.get());
    }

    if(lastProgram.valid()) {
      stateSet->removeAttribute(lastProgram.get());
    }
    stateSet->setAttributeAndModes(glslProgram,
                                   osg::StateAttribute::ON);

    stateSet->removeUniform(shadowSamplesUniform.get());
    stateSet->removeUniform(invShadowSamplesUniform.get());
    stateSet->removeUniform(invShadowTextureSizeUniform.get());
    stateSet->removeUniform(shadowScaleUniform.get());

    stateSet->addUniform(shadowSamplesUniform.get());
    stateSet->addUniform(invShadowSamplesUniform.get());
    stateSet->addUniform(invShadowTextureSizeUniform.get());
    stateSet->addUniform(shadowScaleUniform.get());

    lastProgram = glslProgram;
  }

  void OsgMaterial::setNoiseImage(osg::Image *i) {
    noiseMap->setImage(i);
  }

  void OsgMaterial::setShadowSamples(int v) {
    shadowSamplesUniform->set(v);
    invShadowSamplesUniform->set(1.f/(v*v));
  }

  void OsgMaterial::removeMaterialNode(MaterialNode* d) {
    std::vector<osg::ref_ptr<MaterialNode> >::iterator it;
    for(it=materialNodeVector.begin(); it!=materialNodeVector.end(); ++it) {
      if(it->get() == d) {
        materialNodeVector.erase(it);
        return;
      }
    }
  }

  void OsgMaterial::setShadowTextureSize(int size) {
    invShadowTextureSize = 1./size;
    invShadowTextureSizeUniform->set((float)invShadowTextureSize);
  }

  void OsgMaterial::addMaterialNode(MaterialNode *d) {
    materialNodeVector.push_back(d);
    if(normalMap.valid() || bumpMap.valid()) {
      d->setNeedTangents(true);
    }
    d->setTransparency((float)map.get("transparency", 0.0));
  }

  void OsgMaterial::setMaxNumLights(int n) {
    bool needUpdate = (maxNumLights != n);
    maxNumLights = n;
    if(needUpdate) updateShader(true);    
  }

} // end of namespace osg_material_manager
