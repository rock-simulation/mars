/*
 *  Copyright 2015, DFKI GmbH Robotics Innovation Center
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
 *  MarsMaterial.cpp
 *  General MarsMaterial to inherit from.
 *
 *  Created by Langosz on 20.10.09.
 */

#include <mars/utils/misc.h>
#include "MarsMaterial.h"
#include "gui_helper_functions.h"
#include "../wrapper/OSGMaterialStruct.h"
#include "../shader/shader-generator.h"
#include "../shader/shader-function.h"
#include "../shader/bumpmapping.h"
#include "../shader/pixellight.h"

#include <iostream>

#ifdef HAVE_OSG_VERSION_H
  #include <osg/Version>
#else
  #include <osg/Export>
#endif
#include <osg/TexMat>


namespace mars {
  namespace graphics {

    using namespace std;
    using mars::interfaces::sReal;
    using mars::utils::Vector;
    using mars::utils::Quaternion;
    using mars::interfaces::LightData;
    using mars::interfaces::MaterialData;
    using mars::utils::Color;

    MarsMaterial::MarsMaterial(std::string resPath, int shadowTextureSize)
      : material_(0),
        colorMap_(0),
        normalMap_(0),
        bumpMap_(0),
        hasShaderSources(false),
        useMARSShader(true),
        maxNumLights(1),
        resPath(resPath),
        invShadowTextureSize(1./shadowTextureSize) {
      group_ = new osg::Group();
      baseImageUniform = new osg::Uniform("BaseImage", COLOR_MAP_UNIT);
      normalMapUniform = new osg::Uniform("NormalMap", NORMAL_MAP_UNIT);
      bumpMapUniform = new osg::Uniform("BumpMap", BUMP_MAP_UNIT);
      noiseMapUniform = new osg::Uniform("NoiseMap", NOISE_MAP_UNIT);
      texScaleUniform = new osg::Uniform("texScale", 1.0f);
      shadowScaleUniform = new osg::Uniform("shadowScale", 0.5f);
      bumpNorFacUniform = new osg::Uniform("bumpNorFac", 1.0f);
      shadowSamplesUniform = new osg::Uniform("shadowSamples", 1);
      invShadowSamplesUniform = new osg::Uniform("invShadowSamples",
                                                 1.f/16);
      invShadowTextureSizeUniform = new osg::Uniform("invShadowTextureSize",
                                                     (float)(invShadowTextureSize));

      noiseMap_ = new osg::Texture2D();
      noiseMap_->setDataVariance(osg::Object::DYNAMIC);
      noiseMap_->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
      noiseMap_->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
      noiseMap_->setWrap(osg::Texture::WRAP_R, osg::Texture::REPEAT);
      noiseMap_->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
      noiseMap_->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
    }

    MarsMaterial::~MarsMaterial() {
    }

    // the material struct can also contain a static texture (texture file)
    void MarsMaterial::setMaterial(const MaterialData &mStruct) {
      //return;
      materialData = mStruct;
      name = mStruct.name;
      getLight = mStruct.getLight;
      exists = mStruct.exists;
      if (!exists) return;

      // create the osg::Material
      material_ = new OSGMaterialStruct(mStruct);
      // get the StateSet of the Object
      osg::StateSet *state = group_->getOrCreateStateSet();

      // set the material
      state->setAttributeAndModes(material_.get(), osg::StateAttribute::ON);

      if(!getLight) {
        osg::ref_ptr<osg::CullFace> cull = new osg::CullFace();
        cull->setMode(osg::CullFace::BACK);
        state->setAttributeAndModes(cull.get(), osg::StateAttribute::OFF);
        state->setMode(GL_LIGHTING,
                       osg::StateAttribute::OFF);
        state->setMode(GL_FOG, osg::StateAttribute::OFF);
      }

      if (mStruct.texturename != "") {
        osg::ref_ptr<osg::Image> textureImage = GuiHelper::loadImage(mStruct.texturename);
        //osgDB::readImageFile(mStruct.texturename);
        if(!textureImage.valid()){
          // do not fail silently, at least give error msg for now
          // TODO: not sure if colorMap_ is expected to be not null
          //   if not we can just skip below code if texture load failed.
          cerr << "failed to load " << mStruct.texturename  << endl;
        }

        colorMap_ = GuiHelper::loadTexture(mStruct.texturename);
        state->setTextureAttributeAndModes(COLOR_MAP_UNIT, colorMap_,
                                           osg::StateAttribute::ON);

        if(mStruct.tex_scale != 1.0) {
          osg::ref_ptr<osg::TexMat> scaleTexture = new osg::TexMat();
          scaleTexture->setMatrix(osg::Matrix::scale(mStruct.tex_scale, mStruct.tex_scale, mStruct.tex_scale));
          state->setTextureAttributeAndModes(COLOR_MAP_UNIT, scaleTexture.get(),
                                             osg::StateAttribute::ON);
        }
      }
      else {
        if(colorMap_.valid()) colorMap_ = 0;
      }
      texScaleUniform->set((float)mStruct.tex_scale);
      bool generateTangents = false;
      if (mStruct.normalmap != "") {
        generateTangents = true;
        if(mStruct.bumpmap != "") {
          setBumpMap(mStruct.bumpmap);
        }
        else if(bumpMap_.valid()) {
          bumpMap_ = 0;
        }
        setNormalMap(mStruct.normalmap);
        bumpNorFacUniform->set((float)mStruct.bumpNorFac);
        // generate tangents
      }
      else {
        normalMap_ = 0;
      }
      updateShader(true);

      std::map<unsigned long, DrawObject*>::iterator it;
      for(it=drawObjectMap.begin(); it!=drawObjectMap.end(); ++it) {
        if(generateTangents) {
          it->second->generateTangents();
        }
        it->second->setTransparency((float)mStruct.transparency);
      }
    }

    void MarsMaterial::edit(std::string key, std::string value) {
      if(utils::matchPattern("*/ambientColor/*", key) ||
         utils::matchPattern("*/ambientFront/*", key)) {
        double v = atof(value.c_str());
        if(key[key.size()-1] == 'a') materialData.ambientFront.a = v;
        else if(key[key.size()-1] == 'r') materialData.ambientFront.r = v;
        else if(key[key.size()-1] == 'g') materialData.ambientFront.g = v;
        else if(key[key.size()-1] == 'b') materialData.ambientFront.b = v;
        setMaterial(materialData);
      }
      if(utils::matchPattern("*/diffuseColor/*", key) ||
         utils::matchPattern("*/diffuseFront/*", key)) {
        double v = atof(value.c_str());
        if(key[key.size()-1] == 'a') materialData.diffuseFront.a = v;
        else if(key[key.size()-1] == 'r') materialData.diffuseFront.r = v;
        else if(key[key.size()-1] == 'g') materialData.diffuseFront.g = v;
        else if(key[key.size()-1] == 'b') materialData.diffuseFront.b = v;
        setMaterial(materialData);
      }
      if(utils::matchPattern("*/specularColor/*", key) ||
         utils::matchPattern("*/specularFront/*", key)) {
        double v = atof(value.c_str());
        if(key[key.size()-1] == 'a') materialData.specularFront.a = v;
        else if(key[key.size()-1] == 'r') materialData.specularFront.r = v;
        else if(key[key.size()-1] == 'g') materialData.specularFront.g = v;
        else if(key[key.size()-1] == 'b') materialData.specularFront.b = v;
        setMaterial(materialData);
      }
      if(utils::matchPattern("*/emissionColor/*", key) ||
         utils::matchPattern("*/emissionFront/*", key)) {
        double v = atof(value.c_str());
        if(key[key.size()-1] == 'a') materialData.emissionFront.a = v;
        else if(key[key.size()-1] == 'r') materialData.emissionFront.r = v;
        else if(key[key.size()-1] == 'g') materialData.emissionFront.g = v;
        else if(key[key.size()-1] == 'b') materialData.emissionFront.b = v;
        setMaterial(materialData);
      }

      if(utils::matchPattern("*/diffuseTexture", key) ||
         utils::matchPattern("*/texturename", key)) {
        if(value == "") {
          materialData.texturename = "";
          setMaterial(materialData);
        }
        else if(utils::pathExists(value)) {
          materialData.texturename = value;
          setMaterial(materialData);
        }
      }
      if(utils::matchPattern("*/normalTexture", key) ||
         utils::matchPattern("*/bumpmap", key)) {
        if(value == "") {
          materialData.normalmap = "";
          setMaterial(materialData);
        }
        else if(utils::pathExists(value)) {
          materialData.normalmap = value;
          setMaterial(materialData);
        }
      }
      if(utils::matchPattern("*/displacementTexture", key) ||
         utils::matchPattern("*/displacementmap", key)) {
        if(value == "") {
          materialData.bumpmap = "";
          setMaterial(materialData);
        }
        else if(utils::pathExists(value)) {
          materialData.bumpmap = value;
          setMaterial(materialData);
        }
      }
      if(utils::matchPattern("*/bumpNorFac", key)) {
        materialData.bumpNorFac = atof(value.c_str());
        setMaterial(materialData);
      }
      if(utils::matchPattern("*/shininess", key)) {
        materialData.shininess = atof(value.c_str());
        setMaterial(materialData);
      }
      if(utils::matchPattern("*/transparency", key)) {
        materialData.transparency = atof(value.c_str());
        setMaterial(materialData);
      }
      if(utils::matchPattern("*/tex_scale", key)) {
        materialData.tex_scale = atof(value.c_str());
        setMaterial(materialData);
      }
    }

    void MarsMaterial::setTexture(osg::Texture2D *texture) {
      colorMap_ = texture;
      osg::StateSet *state = group_->getOrCreateStateSet();
      state->setTextureAttributeAndModes(COLOR_MAP_UNIT, colorMap_,
                                         osg::StateAttribute::ON);
    }

    void MarsMaterial::setBumpMap(const std::string &bumpMap) {
      bumpMap_ = GuiHelper::loadTexture( bumpMap.c_str() );
      bumpMap_->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR );
      bumpMap_->setFilter( osg::Texture::MAG_FILTER, osg::Texture::LINEAR );
      bumpMap_->setWrap( osg::Texture::WRAP_S, osg::Texture::REPEAT );
      bumpMap_->setWrap( osg::Texture::WRAP_T, osg::Texture::REPEAT );
      bumpMap_->setMaxAnisotropy(8);

      osg::StateSet *state = group_->getOrCreateStateSet();
      state->setTextureAttributeAndModes(BUMP_MAP_UNIT, bumpMap_,
                                         osg::StateAttribute::ON);
    }

    void MarsMaterial::setNormalMap(const std::string &normalMap) {
      normalMap_ = GuiHelper::loadTexture( normalMap.c_str() );
      normalMap_->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR );
      normalMap_->setFilter( osg::Texture::MAG_FILTER, osg::Texture::LINEAR );
      normalMap_->setWrap( osg::Texture::WRAP_S, osg::Texture::REPEAT );
      normalMap_->setWrap( osg::Texture::WRAP_T, osg::Texture::REPEAT );
      normalMap_->setMaxAnisotropy(8);

      osg::StateSet *state = group_->getOrCreateStateSet();
      state->setTextureAttributeAndModes(NORMAL_MAP_UNIT, normalMap_,
                                         osg::StateAttribute::ON);
    }

    void MarsMaterial::setUseMARSShader(bool val) {
      useMARSShader = val;
      updateShader(true);
    }

    void MarsMaterial::setShadowScale(float v) {
      //fprintf(stderr, "****** da : %g\n", 1.f/(v*v));
      shadowScaleUniform->set(1.f/(v*v));
    }

    void MarsMaterial::updateShader(bool reload) {
      if(!exists) return;
      osg::StateSet* stateSet = group_->getOrCreateStateSet();

      //return;
      if(!useMARSShader || !getLight) {
        if(lastProgram.valid()) {
          stateSet->removeAttribute(lastProgram.get());
          lastProgram = NULL;
        }
        if(normalMap_.valid()) {
          stateSet->setTextureAttributeAndModes(NORMAL_MAP_UNIT, normalMap_,
                                         osg::StateAttribute::OFF);
        }
        if(bumpMap_.valid()) {
          stateSet->setTextureAttributeAndModes(BUMP_MAP_UNIT, bumpMap_,
                                         osg::StateAttribute::OFF);
        }
        stateSet->setTextureAttributeAndModes(NOISE_MAP_UNIT, noiseMap_,
                                              osg::StateAttribute::OFF);

        return;
      }
      if(normalMap_.valid()) {
        stateSet->setTextureAttributeAndModes(NORMAL_MAP_UNIT, normalMap_,
                                              osg::StateAttribute::ON);
      }
      if(bumpMap_.valid()) {
        stateSet->setTextureAttributeAndModes(BUMP_MAP_UNIT, bumpMap_,
                                              osg::StateAttribute::ON);
      }

      if(!reload && hasShaderSources) {
        // no need to regenerate, shader source did not changed
        return;
      }
      hasShaderSources = true;

      stateSet->setTextureAttributeAndModes(NOISE_MAP_UNIT, noiseMap_,
                                            osg::StateAttribute::ON);

      ShaderGenerator shaderGenerator;
      vector<string> args;

      bool hasTexture = (colorMap_.valid()
                         || normalMap_.valid());

      {
        ShaderFunc *vertexShader = new ShaderFunc;
        vertexShader->addExport( (GLSLExport)
                                 {"gl_Position", "gl_ModelViewProjectionMatrix * gl_Vertex"} );
        vertexShader->addExport( (GLSLExport) {"gl_ClipVertex", "v1"} );
        if(hasTexture) {
          vertexShader->addExport( (GLSLExport)
                                   { "gl_TexCoord[0].xy", "gl_MultiTexCoord0.xy" });
        }
        shaderGenerator.addShaderFunction(vertexShader, mars::interfaces::SHADER_TYPE_VERTEX);
      }

      {
        ShaderFunc *fragmentShader = new ShaderFunc;
        if(hasTexture) {
          fragmentShader->addUniform( (GLSLUniform)
                                      { "float", "texScale" } );
          if(bumpMap_.valid()) {
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
        if(colorMap_.valid()) {
          fragmentShader->addUniform( (GLSLUniform)
                                      { "sampler2D", "BaseImage" } );
          fragmentShader->addMainVar( (GLSLVariable)
                                      { "vec4", "col", "texture2D( BaseImage, texCoord)" });
        }
        else {
          fragmentShader->addMainVar( (GLSLVariable)
                                      { "vec4", "col", "vec4(1.0)" });
        }
        fragmentShader->addExport( (GLSLExport) {"gl_FragColor", "col"} );
        shaderGenerator.addShaderFunction(fragmentShader, mars::interfaces::SHADER_TYPE_FRAGMENT);
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
      shaderGenerator.addShaderFunction(plightVert, mars::interfaces::SHADER_TYPE_VERTEX);

      if(normalMap_.valid()) {
        args.clear();
        args.push_back("n.xyz");
        BumpMapVert *bumpVert = new BumpMapVert(args, resPath);
        shaderGenerator.addShaderFunction(bumpVert, mars::interfaces::SHADER_TYPE_VERTEX);

        args.clear();
        args.push_back("texture2D( NormalMap, texCoord )");
        args.push_back("n");
        args.push_back("n");
        BumpMapFrag *bumpFrag = new BumpMapFrag(args, resPath);
        bumpFrag->addUniform( (GLSLUniform) { "sampler2D", "NormalMap" } );
        shaderGenerator.addShaderFunction(bumpFrag, mars::interfaces::SHADER_TYPE_FRAGMENT);
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
      shaderGenerator.addShaderFunction(plightFrag, mars::interfaces::SHADER_TYPE_FRAGMENT);

      osg::Program *glslProgram = shaderGenerator.generate();

      if(normalMap_.valid()) {
        glslProgram->addBindAttribLocation( "vertexTangent", TANGENT_UNIT );
      }

      stateSet->addUniform(noiseMapUniform.get());

      if(normalMap_.valid()) {
        stateSet->addUniform(normalMapUniform.get());
        stateSet->addUniform(bumpNorFacUniform.get());
      }
      else {
        stateSet->removeUniform(normalMapUniform.get());
        stateSet->removeUniform(bumpNorFacUniform.get());
      }

      if(bumpMap_.valid()) {
        stateSet->addUniform(bumpMapUniform.get());
      }
      else {
        stateSet->removeUniform(bumpMapUniform.get());
      }

      if(colorMap_.valid() || normalMap_.valid() || bumpMap_.valid()) {
        stateSet->addUniform(texScaleUniform.get());
      }
      else {
        stateSet->removeUniform(texScaleUniform.get());
      }
      if(colorMap_.valid()) {
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

    void MarsMaterial::setNoiseImage(osg::Image *i) {
      noiseMap_->setImage(i);
    }

    void MarsMaterial::setShadowSamples(int v) {
      shadowSamplesUniform->set(v);
      invShadowSamplesUniform->set(1.f/(v*v));
    }

  } // end of namespace graphics
} // end of namespace mars
