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

/*
 *  DrawObject.cpp
 *  General DrawObject to inherit from.
 *
 *  Created by Roemmermann on 20.10.09.
 */

#include "DrawObject.h"
#include "gui_helper_functions.h"
#include "../wrapper/OSGMaterialStruct.h"
#include "../shader/shader-generator.h"
#include "../shader/shader-function.h"
#include "../shader/bumpmapping.h"
#include "../shader/pixellight.h"

#include <iostream>

#include <osg/CullFace>
#include <osg/PolygonMode>
#include <osg/Depth>
#include <osg/ComputeBoundsVisitor>
#include <osg/TexMat>

#include <osgUtil/TangentSpaceGenerator>

#include <osgDB/WriteFile>
#include <osgDB/ReadFile>

namespace mars {
  namespace graphics {

    using namespace std;
    using mars::interfaces::sReal;
    using mars::utils::Vector;
    using mars::utils::Quaternion;
    using mars::interfaces::LightData;
    using mars::interfaces::MaterialData;
    using mars::utils::Color;

    static osg::Material* makeSelectionMaterial()
    {
      // create selection material, will be used if object is selected
      //NEW_MATERIAL_STRUCT(mStruct);
      MaterialData mStruct;
      mStruct.diffuseFront = Color( 0.0, 1.0, 0.0, 1.0 );
      mStruct.diffuseBack = Color( 0.0, 1.0, 0.0, 1.0 );
      return new OSGMaterialStruct(mStruct);
    }

    osg::ref_ptr<osg::Material> DrawObject::selectionMaterial = makeSelectionMaterial();

    DrawObject::DrawObject()
      : id_(0),
        nodeMask_(0xff),
        stateFilename_(""),
        selected_(false),
        selectable_(true),
        lastProgram(NULL),
        normalMapUniform(NULL),
        baseImageUniform(NULL),
	group_(0),
        material_(0),
        colorMap_(0),
        normalMap_(0),
        posTransform_(0),
        scaleTransform_(0),
        blendEquation_(0),
        hasShaderSources(false),
        useMARSShader(true) {
    }

    DrawObject::~DrawObject() {
      /*if(lastProgram) delete lastProgram;
        if(normalMapUniform) delete normalMapUniform;
        if(baseImageUniform) delete baseImageUniform;*/
    }
    
    unsigned long DrawObject::getID(void) const {
      return id_;
    }
    void DrawObject::setID(unsigned long id) {
      id_ = id;
    }

    void DrawObject::createObject(unsigned long id,
                                  const Vector &pivot) {
      id_ = id;
      pivot_ = pivot;

      brightnessUniform = new osg::Uniform("brightness", 1.0f);
      transparencyUniform = new osg::Uniform("alpha", 1.0f);
      texScaleUniform = new osg::Uniform("texScale", 1.0f);
      scaleTransform_ = new osg::MatrixTransform();
      scaleTransform_->setMatrix(osg::Matrix::scale(1.0, 1.0, 1.0));
      scaleTransform_->setDataVariance(osg::Node::STATIC);

      posTransform_ = new osg::PositionAttitudeTransform();
      posTransform_->setPivotPoint(osg::Vec3(pivot_.x(), pivot_.y(), pivot_.z()));
      posTransform_->setPosition(osg::Vec3(0.0, 0.0, 0.0));
      posTransform_->addChild(scaleTransform_.get());
      posTransform_->setNodeMask(nodeMask_);

      group_ = new osg::Group;

      std::list< osg::ref_ptr< osg::Geode > > geodes = createGeometry();
      for(std::list< osg::ref_ptr< osg::Geode > >::iterator it = geodes.begin();
          it != geodes.end(); ++it) {
        group_->addChild(it->get());
        for(unsigned int i=0; i<it->get()->getNumDrawables(); ++i) {
          osg::Drawable *draw = it->get()->getDrawable(i);
          geometry_.push_back(draw->asGeometry());
        }
      }

      // get the size of the object
      osg::ComputeBoundsVisitor cbbv;
      group_->accept(cbbv);
      osg::BoundingBox bb = cbbv.getBoundingBox();
      if(fabs(bb.xMax()) > fabs(bb.xMin())) {
        geometrySize_.x() = fabs(bb.xMax() - bb.xMin());
      } else {
        geometrySize_.x() = fabs(bb.xMin() - bb.xMax());
      }
      if(fabs(bb.yMax()) > fabs(bb.yMin())) {
        geometrySize_.y() = fabs(bb.yMax() - bb.yMin());
      } else {
        geometrySize_.y() = fabs(bb.yMin() - bb.yMax());
      }
      if(fabs(bb.zMax()) > fabs(bb.zMin())) {
        geometrySize_.z() = fabs(bb.zMax() - bb.zMin());
      } else {
        geometrySize_.z() = fabs(bb.zMin() - bb.zMax());
      }

      scaleTransform_->addChild(group_.get());
    }


    void DrawObject::setStateFilename(const std::string &filename, int create) {
      stateFilename_ = filename;
      if (create) {
        FILE* stateFile = fopen(stateFilename_.data(), "w");
        fclose(stateFile);
      }
    }
    void DrawObject::exportState(void) {
      if (id_) {
        FILE* stateFile = fopen(stateFilename_.data(), "a");
        fprintf(stateFile,"%lu\t%11.6f\t%11.6f\t%11.6f", id_, position_.x(), position_.y(), position_.z());
        fprintf(stateFile,"\t%11.6f\t%11.6f\t%11.6f\t%11.6f\n", quaternion_.x(), quaternion_.y(), quaternion_.z(), quaternion_.w());
        fclose(stateFile);
      }
    }
    void DrawObject::exportModel(const std::string &filename) {
      osgDB::writeNodeFile(*(posTransform_.get()), filename.data());
    }

    // the material struct can also contain a static texture (texture file)
    void DrawObject::setMaterial(const MaterialData &mStruct, bool _useFog,
                                 bool _useNoise) {
      //return;
      useFog = _useFog;
      useNoise = _useNoise;
      getLight = mStruct.getLight;

      if(mStruct.brightness != 0.0) {
        brightnessUniform->set((float)mStruct.brightness);
      } else {
        brightnessUniform->set(1.0f);
      }
      if (!mStruct.exists) return;
      setNodeMask(mStruct.cullMask);

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
                       osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
        state->setMode(GL_FOG, osg::StateAttribute::OFF);
      }

      // handle transparency
      transparencyUniform->set((float)(1.0f-(float)(mStruct.transparency)));
      if (mStruct.transparency != 0) {
        state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        state->setMode(GL_BLEND,osg::StateAttribute::ON);
        state->setRenderBinDetails(1, "DepthSortedBin");
        osg::ref_ptr<osg::Depth> depth = new osg::Depth;
        depth->setWriteMask( false );
        state->setAttributeAndModes(depth.get(),
                                    osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
      }

      if (mStruct.texturename != "") {
        osg::ref_ptr<osg::Image> textureImage = osgDB::readImageFile(mStruct.texturename);
        if(!textureImage.valid()){
          // do not fail silently, at least give error msg for now
          // TODO: not sure if colorMap_ is expected to be not null
          //   if not we can just skip below code if texture load failed.
          cerr << "failed to load " << mStruct.texturename  << endl;
        }

        colorMap_ = new osg::Texture2D;
        colorMap_->setImage(textureImage.get());
        colorMap_->setWrap(osg::Texture2D::WRAP_R, osg::Texture2D::REPEAT);
        colorMap_->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT);
        colorMap_->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT);

        state->setTextureAttributeAndModes(COLOR_MAP_UNIT, colorMap_,
                                           osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

        texScaleUniform->set((float)mStruct.tex_scale);
        if(mStruct.tex_scale != 1.0) {
          osg::ref_ptr<osg::TexMat> scaleTexture = new osg::TexMat();
          scaleTexture->setMatrix(osg::Matrix::scale(mStruct.tex_scale, mStruct.tex_scale, mStruct.tex_scale));
          state->setTextureAttributeAndModes(COLOR_MAP_UNIT, scaleTexture.get(),
                                             osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
        }

        if (mStruct.reflect) {
          // TODO: texgen ?
          /*
            osg::TexGen *texgen = new osg::TexGen;
            texgen->setMode(osg::TexGen::SPHERE_MAP);
            state->setTextureAttributeAndModes(textureUnitCounter, texgen,
            osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
          */
        }
      }

      if (mStruct.bumpmap != "") {
        setBumpMap(mStruct.bumpmap);
      } else {
        if(lastProgram) updateShader(lastLights, true);
      }
    }

    void DrawObject::setBlending(bool mode) {
      if(blendEquation_ && !mode) {
        osg::StateSet *state = group_->getOrCreateStateSet();
        state->setAttributeAndModes(blendEquation_.get(),
                                    osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF);
      }
      else if(mode) {
        if(!blendEquation_) {
          blendEquation_ = new osg::BlendEquation(osg::BlendEquation::FUNC_ADD);
          //blendEquation->setDataVariance(osg::Object::DYNAMIC);
        }
        osg::StateSet *state = group_->getOrCreateStateSet();
        state->setAttributeAndModes(blendEquation_.get(),
                                    osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
      }
    }

    void DrawObject::setTexture(osg::Texture2D *texture) {
      colorMap_ = texture;
      osg::StateSet *state = group_->getOrCreateStateSet();
      state->setTextureAttributeAndModes(COLOR_MAP_UNIT, colorMap_,
                                         osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
      if(lastProgram) updateShader(lastLights, true);
    }

    void DrawObject::setBumpMap(const std::string &bumpMap) {
      generateTangents();

      normalMap_ = GuiHelper::loadTexture( bumpMap.data() );
      normalMap_->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR );
      normalMap_->setFilter( osg::Texture::MAG_FILTER, osg::Texture::LINEAR );
      normalMap_->setWrap( osg::Texture::WRAP_S, osg::Texture::REPEAT );
      normalMap_->setWrap( osg::Texture::WRAP_T, osg::Texture::REPEAT );
      normalMap_->setMaxAnisotropy(8);

      osg::StateSet *state = group_->getOrCreateStateSet();
      state->setTextureAttributeAndModes(NORMAL_MAP_UNIT, normalMap_,
                                         osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
      if(lastProgram) updateShader(lastLights, true);
    }


    void DrawObject::setPosition(const Vector &_pos) {
      position_ = _pos;
      posTransform_->setPosition(osg::Vec3(position_.x(), position_.y(), position_.z()));  
    }

    void DrawObject::setQuaternion(const Quaternion &q) {
      osg::Quat oQuat;
      oQuat.set(q.x(), q.y(), q.z(), q.w());
      posTransform_->setAttitude(oQuat);
      quaternion_ = q;
    }

    void DrawObject::setScale(const Vector &scale) {
      scaleTransform_->setMatrix(osg::Matrix::scale(
                                                    scale.x(), scale.y(), scale.z()));
      posTransform_->setPivotPoint(osg::Vec3(
                                             pivot_.x()*scale.x(), pivot_.y()*scale.y(), pivot_.z()*scale.z()));
      scaledSize_ = Vector(
                           scale.x() * geometrySize_.x(),
                           scale.y() * geometrySize_.y(),
                           scale.z() * geometrySize_.z());
    }

    void DrawObject::setScaledSize(const Vector &scaledSize) {
      setScale(Vector(
                      scaledSize.x() / geometrySize_.x(),
                      scaledSize.y() / geometrySize_.y(),
                      scaledSize.z() / geometrySize_.z()));
    }

    void DrawObject::generateTangents()
    {
      osg::ref_ptr< osgUtil::TangentSpaceGenerator > tsg = new osgUtil::TangentSpaceGenerator;
      for(std::list< osg::ref_ptr<osg::Geometry> >::iterator it = geometry_.begin();
          it != geometry_.end(); ++it) {
        osg::ref_ptr<osg::Geometry> geom = *it;
        if(geom->empty()) {
          cerr << "WARNING: empty geometry for DrawObject " << id_ << "!" << endl;
        }
        tsg->generate( geom.get(), DEFAULT_UV_UNIT );
        osg::Vec4Array *tangents = tsg->getTangentArray();
        if(tangents==NULL || tangents->size()==0) {
          cerr << "Failed to generate tangents for DrawObject " << id_ << "!" << endl;
          return;
        }
        geom->setVertexAttribData( TANGENT_UNIT,
                                   osg::Geometry::ArrayData( tangents, osg::Geometry::BIND_PER_VERTEX ) );
      }
    }

    void DrawObject::removeBits(unsigned int bits) {
      nodeMask_ &= ~bits;
      posTransform_->setNodeMask(nodeMask_);
    }
    void DrawObject::setBits(unsigned int bits) {
      nodeMask_ = bits;
      posTransform_->setNodeMask(nodeMask_);
    }

    void DrawObject::setSelected(bool val) {
      if(selectable_) {
        osg::PolygonMode *polyModeObj;

        osg::StateSet *state = group_->getOrCreateStateSet();
  
        polyModeObj = dynamic_cast<osg::PolygonMode*>
          (state->getAttribute(osg::StateAttribute::POLYGONMODE));
  
        if (!polyModeObj){
          polyModeObj = new osg::PolygonMode;
          state->setAttribute( polyModeObj );    
        }
 
        if(val) {
          polyModeObj->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
          state->setAttributeAndModes(material_.get(), osg::StateAttribute::OFF);
          state->setAttributeAndModes(selectionMaterial.get(),
                                      osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
          state->setMode(GL_LIGHTING,
                         osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
          state->setMode(GL_FOG, osg::StateAttribute::OFF);
        }
        else {
          polyModeObj->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL);
          state->setAttributeAndModes(selectionMaterial.get(), osg::StateAttribute::OFF);
          state->setAttributeAndModes(material_.get(), osg::StateAttribute::ON);
          state->setMode(GL_LIGHTING,
                         osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
          state->setMode(GL_FOG, osg::StateAttribute::ON);
        }
      }
    }

    void DrawObject::setRenderBinNumber(int number) {
      osg::StateSet *state = group_->getOrCreateStateSet();
      if(number == 0) {
        state->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
      }
      else {
        state->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
      }
      state->setRenderBinDetails(number, "RenderBin");
    }

    bool DrawObject::containsNode(osg::Node* node) {
      if(posTransform_) {
        return (posTransform_ == node) || posTransform_->containsNode(node);
      }
      return false;
    }

    void DrawObject::updateShader(
                                  vector<mars::interfaces::LightData*> &lightList,
                                  bool reload,
                                  const std::map<mars::interfaces::ShaderType, std::string> &shaderSources) {

      //return;
      if(!useMARSShader || !getLight) return;
      if(shaderSources.empty()) {
        if(reload && hasShaderSources) {
          // no need to regenerate, shader source did not changed
          return;
        }
      }
      else {
        hasShaderSources = true;
      }

      if(shaderSources.empty()) { // generate shader!
        ShaderGenerator shaderGenerator;
        vector<string> args;

        bool hasTexture = (colorMap_.valid()
                           || normalMap_.valid());

        {
          ShaderFunc *vertexShader = new ShaderFunc;
          vertexShader->addExport( (GLSLExport)
                                   {"gl_Position", "gl_ModelViewProjectionMatrix * gl_Vertex"} );
          vertexShader->addExport( (GLSLExport) {"gl_ClipVertex", "v"} );
          if(hasTexture) {
            vertexShader->addExport( (GLSLExport)
                                     { "gl_TexCoord[0].xy", "gl_MultiTexCoord0.xy" });
          }
          shaderGenerator.addShaderFunction(vertexShader, mars::interfaces::SHADER_TYPE_VERTEX);
        }

        {
          ShaderFunc *fragmentShader = new ShaderFunc;
          if(colorMap_.valid()) {
            fragmentShader->addUniform( (GLSLUniform)
                                        { "float", "texScale" } );
            fragmentShader->addUniform( (GLSLUniform)
                                        { "sampler2D", "BaseImage" } );
            fragmentShader->addMainVar( (GLSLVariable)
                                        { "vec4", "col", "texture2D( BaseImage, gl_TexCoord[0].xy * texScale)" });
          } else {
            fragmentShader->addMainVar( (GLSLVariable)
                                        { "vec4", "col", "vec4(1.0)" });
          }
          fragmentShader->addExport( (GLSLExport) {"gl_FragColor", "col"} );
          shaderGenerator.addShaderFunction(fragmentShader, mars::interfaces::SHADER_TYPE_FRAGMENT);
        }

        args.clear();
        args.push_back("v");
        PixelLightVert *plightVert = new PixelLightVert(args, lightList);
        plightVert->addMainVar( (GLSLVariable)
                                { "vec4", "v", "gl_ModelViewMatrix * gl_Vertex" } );
        plightVert->addExport( (GLSLExport)
                               { "normalVarying", "normalize( gl_NormalMatrix * gl_Normal )" } );
        plightVert->addVarying( (GLSLVarying)
                                { "vec3", "normalVarying" } );
        shaderGenerator.addShaderFunction(plightVert, mars::interfaces::SHADER_TYPE_VERTEX);

        if(normalMap_.valid()) {
          args.clear();
          args.push_back("gl_NormalMatrix * gl_Normal");
          BumpMapVert *bumpVert = new BumpMapVert(args, lightList);
          shaderGenerator.addShaderFunction(bumpVert, mars::interfaces::SHADER_TYPE_VERTEX);

          args.clear();
          args.push_back("texture2D( NormalMap, gl_TexCoord[0].xy * texScale )");
          args.push_back("n");
          BumpMapFrag *bumpFrag = new BumpMapFrag(args);
          bumpFrag->addUniform( (GLSLUniform) { "sampler2D", "NormalMap" } );
          shaderGenerator.addShaderFunction(bumpFrag, mars::interfaces::SHADER_TYPE_FRAGMENT);
        }

        args.clear();
        args.push_back("col");
        args.push_back("n");
        args.push_back("col");
        PixelLightFrag *plightFrag = new PixelLightFrag(args, useFog,
                                                        useNoise, lightList);
        // invert the normal if gl_FrontFacing=true to handle back faces
        // correctly.
        // TODO: check not needed if backfaces not processed.
        plightFrag->addMainVar( (GLSLVariable)
                                { "vec3", "n", "normalize( gl_FrontFacing ? normalVarying : -normalVarying )"} );
        plightFrag->addVarying( (GLSLVarying)
                                { "vec3", "normalVarying" } );
        shaderGenerator.addShaderFunction(plightFrag, mars::interfaces::SHADER_TYPE_FRAGMENT);

        osg::StateSet* stateSet = getObject()->getOrCreateStateSet();
        osg::Program *glslProgram = shaderGenerator.generate();

        if(normalMap_.valid()) {
          glslProgram->addBindAttribLocation( "vertexTangent", TANGENT_UNIT );
        }

        if(lastProgram) {
          stateSet->setAttributeAndModes(lastProgram, osg::StateAttribute::OFF);
        } else {
          stateSet->addUniform(brightnessUniform.get());
          stateSet->addUniform(transparencyUniform.get());
          stateSet->addUniform(texScaleUniform.get());
        }

        if(normalMapUniform) {
          stateSet->removeUniform(normalMapUniform);
          normalMapUniform = NULL;
        }
        if(normalMap_.valid()) {
          normalMapUniform = new osg::Uniform("NormalMap", NORMAL_MAP_UNIT);
          stateSet->addUniform(normalMapUniform);
        }

        if(baseImageUniform) {
          stateSet->removeUniform(baseImageUniform);
          baseImageUniform = NULL;
        }
        if(colorMap_.valid()) {
          baseImageUniform = new osg::Uniform("BaseImage", COLOR_MAP_UNIT);
          stateSet->addUniform(baseImageUniform);
        }

        stateSet->setAttributeAndModes(glslProgram, osg::StateAttribute::ON);
        lastProgram = glslProgram;

      } else if(shaderSources.count(mars::interfaces::SHADER_TYPE_FFP)>0) {
        // nothing to do, this node uses fixed function pipeline

      } else { // GLSL code specified in NodeData
        osg::Program* glslProgram = new osg::Program;
        for(std::map<mars::interfaces::ShaderType, std::string>::const_iterator it = shaderSources.begin();
            it != shaderSources.end(); ++it)
          {
            osg::Shader* shaderStage;
            switch(it->first) {
            case mars::interfaces::SHADER_TYPE_FFP: break;
            case mars::interfaces::SHADER_TYPE_FRAGMENT:
              shaderStage = new osg::Shader( osg::Shader::FRAGMENT );
              shaderStage->setShaderSource(it->second);
              glslProgram->addShader( shaderStage );
              break;
            case mars::interfaces::SHADER_TYPE_VERTEX:
              shaderStage = new osg::Shader( osg::Shader::VERTEX );
              shaderStage->setShaderSource(it->second);
              glslProgram->addShader( shaderStage );
              break;
            case mars::interfaces::SHADER_TYPE_GEOMETRY:
              shaderStage = new osg::Shader( osg::Shader::GEOMETRY );
              shaderStage->setShaderSource(it->second);
              glslProgram->addShader( shaderStage );
              break;
            default:
              // unknown shader
              break;
            }
          }

        osg::StateSet *stateSet = getObject()->getOrCreateStateSet();
        if(lastProgram) {
          stateSet->setAttributeAndModes(lastProgram, osg::StateAttribute::OFF);
        }
        stateSet->setAttributeAndModes(glslProgram, osg::StateAttribute::ON);
        lastProgram = glslProgram;
      }

      vector<LightData*> lightListBuf;
      lightListBuf.insert(lightListBuf.begin(), lightList.begin(), lightList.end());
      lastLights = lightListBuf;
    }

    void DrawObject::showNormals(bool val) {
      if(normal_geode.valid()) {
        if(val) {
          scaleTransform_->addChild(normal_geode.get());
        }
        else {
          scaleTransform_->removeChild(normal_geode.get());
        }
      }
    }

    void DrawObject::setUseFog(bool val) {
      useFog = val;
      if(lastProgram) updateShader(lastLights, true);
    }

    void DrawObject::setUseNoise(bool val) {
      useNoise = val;
      if(lastProgram) updateShader(lastLights, true);
    }

    void DrawObject::setBrightness(float val) {
      brightnessUniform->set(val);
    }

    void DrawObject::collideSphere(Vector pos, sReal radius) {
    }

  } // end of namespace graphics
} // end of namespace mars
