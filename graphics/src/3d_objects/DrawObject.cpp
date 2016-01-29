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
#include "../MarsMaterial.h"

#include <iostream>

#include <osg/CullFace>
#include <osg/PolygonMode>
#include <osg/ComputeBoundsVisitor>

#include <osgUtil/TangentSpaceGenerator>

#include <osgDB/WriteFile>
#include <osgDB/ReadFile>

#ifdef HAVE_OSG_VERSION_H
  #include <osg/Version>
#else
  #include <osg/Export>
#endif

#include "../GraphicsManager.h"

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
      return new OSGMaterialStruct(mStruct);
    }

    osg::ref_ptr<osg::Material> DrawObject::selectionMaterial = makeSelectionMaterial();

    DrawObject::DrawObject(GraphicsManager *g)
      : id_(0),
        nodeMask_(0xff),
        stateFilename_(""),
        selected_(false),
        selectable_(true),
        group_(0),
        material_(0),
        posTransform_(0),
        scaleTransform_(0),
        blendEquation_(0),
        maxNumLights(1),
        g(g),
        sharedStateGroup(false),
        marsMaterial(NULL),
        showSelected(true),
        tangentsGenerated(false),
        isHidden(true) {
    }

    DrawObject::~DrawObject() {
      if(marsMaterial!=NULL) marsMaterial->removeDrawObject(id_);
    }

    unsigned long DrawObject::getID(void) const {
      return id_;
    }
    void DrawObject::setID(unsigned long id) {
      id_ = id;
    }

    void DrawObject::createObject(unsigned long id,
                                  const Vector &pivot,
                                  unsigned long sharedID) {
      id_ = id;
      pivot_ = pivot;

      brightnessUniform = new osg::Uniform("brightness", 1.0f);
      transparencyUniform = new osg::Uniform("alpha", 1.0f);
      lineLaserPosUniform = new osg::Uniform("lineLaserPos", osg::Vec3f(0.0f, 0.0f, 0.0f));
      lineLaserNormalUniform = new osg::Uniform("lineLaserNormal", osg::Vec3f(1.0f, 0.0f, 0.0f));
      lineLaserColor = new osg::Uniform("lineLaserColor", osg::Vec4f(1.0f, 0.0f, 0.0f, 1.0f));
      lineLaserDirection = new osg::Uniform("lineLaserDirection", osg::Vec3f(0.0f, 0.0f, 1.0f));
      lineLaserOpeningAngle = new osg::Uniform("lineLaserOpeningAngle", (float)M_PI * 2.0f);

      lightPosUniform = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "lightPos", maxNumLights);
      lightAmbientUniform = new osg::Uniform(osg::Uniform::FLOAT_VEC4, "lightAmbient", maxNumLights);
      lightDiffuseUniform = new osg::Uniform(osg::Uniform::FLOAT_VEC4, "lightDiffuse", maxNumLights);
      lightSpecularUniform = new osg::Uniform(osg::Uniform::FLOAT_VEC4, "lightSpecular", maxNumLights);
      lightSpotDirUniform = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "lightSpotDir", maxNumLights);
      lightIsSpotUniform = new osg::Uniform(osg::Uniform::INT, "lightIsSpot", maxNumLights);
      lightIsDirectionalUniform = new osg::Uniform(osg::Uniform::INT, "lightIsDirectional", maxNumLights);
      lightConstantAttUniform = new osg::Uniform(osg::Uniform::FLOAT, "lightConstantAtt", maxNumLights);
      lightLinearAttUniform = new osg::Uniform(osg::Uniform::FLOAT, "lightLinearAtt", maxNumLights);
      lightQuadraticAttUniform = new osg::Uniform(osg::Uniform::FLOAT, "lightQuadraticAtt", maxNumLights);
      lightIsSetUniform = new osg::Uniform(osg::Uniform::INT, "lightIsSet", maxNumLights);
      lightCosCutoffUniform = new osg::Uniform(osg::Uniform::FLOAT, "lightCosCutoff", maxNumLights);
      lightSpotExponentUniform = new osg::Uniform(osg::Uniform::FLOAT, "lightSpotExponent", maxNumLights);
      useFogUniform = new osg::Uniform("useFog", 1);
      useNoiseUniform = new osg::Uniform("useNoise", 1);
      useShadowUniform = new osg::Uniform("useShadow", 1);
      numLightsUniform = new osg::Uniform("numLights", maxNumLights);
      drawLineLaserUniform = new osg::Uniform("drawLineLaser", 0);

      scaleTransform_ = new osg::MatrixTransform();
      scaleTransform_->setMatrix(osg::Matrix::scale(1.0, 1.0, 1.0));
      scaleTransform_->setDataVariance(osg::Node::STATIC);

      posTransform_ = new osg::PositionAttitudeTransform();
      posTransform_->setPivotPoint(osg::Vec3(pivot_.x(), pivot_.y(), pivot_.z()));
      posTransform_->setPosition(osg::Vec3(0.0, 0.0, 0.0));
      posTransform_->addChild(scaleTransform_.get());
      posTransform_->setNodeMask(nodeMask_);

      group_ = new osg::Group;
      if(sharedID) {
        stateGroup_ = g->getSharedStateGroup(sharedID);
        if(stateGroup_.valid()) {
          sharedStateGroup = true;
        }
        else {
          sharedStateGroup = false;
          stateGroup_ = new osg::Group;
        }
      }
      else {
        stateGroup_ = new osg::Group;
      }
      if(!sharedStateGroup) {
        stateGroup_->addChild(posTransform_.get());
      }

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

      if(lod.valid()) {
        scaleTransform_->addChild(lod.get());
      }
      else {
        scaleTransform_->addChild(group_.get());
      }
    }

    void DrawObject::addLODGeodes(std::list< osg::ref_ptr< osg::Geode > > geodes,
                                 float start, float end) {
      std::list< osg::ref_ptr< osg::Geode > >::iterator it;
      if(!lod.valid()) {
        lod = new osg::LOD();
      }
      for(it=geodes.begin(); it!=geodes.end(); ++it) {
        lod->addChild(it->get(), start, end);
        for(unsigned int i=0; i<it->get()->getNumDrawables(); ++i) {
          osg::Drawable *draw = it->get()->getDrawable(i);
          geometry_.push_back(draw->asGeometry());
        }
      }
    }

    osg::ref_ptr<osg::Material> DrawObject::getMaterial() {
      return material_->getMaterial();
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
      osgDB::writeNodeFile(*(stateGroup_.get()), filename.data());
    }

    void DrawObject::setTransparency(float t) {
      // handle transparency
      transparencyUniform->set(1-t);
      if(sharedStateGroup) return;
      osg::StateSet *state = stateGroup_->getOrCreateStateSet();
      if (t >= 0.00001) {
        state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        state->setMode(GL_BLEND,osg::StateAttribute::ON);
        state->setRenderBinDetails(1, "DepthSortedBin");
        depth = new osg::Depth;
        depth->setWriteMask( false );
        state->setAttributeAndModes(depth.get(),
                                    osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
      }
      else {
        if(depth.valid()) {
          state->removeAttribute(depth.get());
        }
        state->setRenderingHint(osg::StateSet::DEFAULT_BIN);
        state->setMode(GL_BLEND,osg::StateAttribute::OFF);
        state->setRenderBinDetails(0, "RenderBin");
      }
    }

    // the material struct can also contain a static texture (texture file)
    void DrawObject::setMaterial(const MaterialData &mStruct, bool _useFog,
                                 bool _useNoise, bool _drawLineLaser,
                                 bool _marsShadow) {

      //osg::StateSet *mState = g->getMaterialStateSet(mStruct);
      bool show_ = !isHidden;
      if(mGroup_.valid()) {
        // todo: do not show if is already hidden
        hide();
      }
      if(!mStruct.normalmap.empty()) generateTangents();
      mGroup_ = g->getMaterialGroup(mStruct);//new osg::Group();
      materialState = g->getMaterialStateSet(mStruct);
      if(show_) {
        show();
      }
      //mGroup->setStateSet(mState);
      //scaleTransform_->addChild(mGroup);
      //scaleTransform_->removeChild(group_.get());
      if(sharedStateGroup) return;
      //return;
      useFog = _useFog;
      if(useFog) useFogUniform->set(1);
      else useFogUniform->set(0);

      useNoise = _useNoise;
      if(useNoise) useNoiseUniform->set(1);
      else useNoiseUniform->set(0);

      if(_marsShadow) useShadowUniform->set(1);
      else useShadowUniform->set(0);

      drawLineLaser = _drawLineLaser;
      if(drawLineLaser) drawLineLaserUniform->set(1);
      else drawLineLaserUniform->set(0);

      if(mStruct.brightness != 0.0) {
        brightnessUniform->set((float)mStruct.brightness);
      } else {
        brightnessUniform->set(1.0f);
      }
      if (!mStruct.exists) return;
      setNodeMask(mStruct.cullMask);

      osg::StateSet *state = stateGroup_->getOrCreateStateSet();

      // handle transparency
      transparencyUniform->set((float)(1.f-(float)(mStruct.transparency)));
      if (mStruct.transparency != 0) {
        state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        state->setMode(GL_BLEND,osg::StateAttribute::ON);
        state->setRenderBinDetails(1, "DepthSortedBin");
        osg::ref_ptr<osg::Depth> depth = new osg::Depth;
        depth->setWriteMask( false );
        state->setAttributeAndModes(depth.get(),
                                    osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
      }

      state->addUniform(brightnessUniform.get());
      state->addUniform(transparencyUniform.get());
      state->addUniform(lightPosUniform.get());
      state->addUniform(lightAmbientUniform.get());
      state->addUniform(lightDiffuseUniform.get());
      state->addUniform(lightSpecularUniform.get());
      state->addUniform(lightSpotDirUniform.get());
      state->addUniform(lightIsSpotUniform.get());
      state->addUniform(lightIsDirectionalUniform.get());
      state->addUniform(lightConstantAttUniform.get());
      state->addUniform(lightLinearAttUniform.get());
      state->addUniform(lightQuadraticAttUniform.get());
      state->addUniform(lightIsSetUniform.get());
      state->addUniform(lightCosCutoffUniform.get());
      state->addUniform(lightSpotExponentUniform.get());

      state->addUniform(lineLaserPosUniform.get());
      state->addUniform(lineLaserNormalUniform.get());
      state->addUniform(lineLaserColor.get());
      state->addUniform(lineLaserDirection.get());
      state->addUniform(lineLaserOpeningAngle.get());
      state->addUniform(useFogUniform.get());
      state->addUniform(useNoiseUniform.get());
      state->addUniform(useShadowUniform.get());
      state->addUniform(numLightsUniform.get());
      state->addUniform(drawLineLaserUniform.get());
    }

    void DrawObject::setBlending(bool mode) {
      if(sharedStateGroup) return;
      if(blendEquation_ && !mode) {
        osg::StateSet *state = stateGroup_->getOrCreateStateSet();
        state->setAttributeAndModes(blendEquation_.get(),
                                    osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF);
      }
      else if(mode) {
        if(!blendEquation_) {
          blendEquation_ = new osg::BlendEquation(osg::BlendEquation::FUNC_ADD);
          //blendEquation->setDataVariance(osg::Object::DYNAMIC);
        }
        osg::StateSet *state = stateGroup_->getOrCreateStateSet();
        state->setAttributeAndModes(blendEquation_.get(),
                                    osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
      }
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

    void DrawObject::generateTangents() {
      if(tangentsGenerated) return;
      std::list< osg::ref_ptr<osg::Geometry> >::iterator it;
      for(it = geometry_.begin(); it != geometry_.end(); ++it) {
        generateTangents(it->get());
      }
      tangentsGenerated = true;
    }

    void DrawObject::generateTangents(osg::ref_ptr<osg::Geometry> geom) {
      osg::ref_ptr< osgUtil::TangentSpaceGenerator > tsg = new osgUtil::TangentSpaceGenerator;

        if(geom->empty()) {
          cerr << "WARNING: empty geometry for DrawObject " << id_ << "!" << endl;
        }
        // color map unit for texture coordinates
        tsg->generate( geom.get(), DEFAULT_UV_UNIT );
        osg::Vec4Array *tangents = tsg->getTangentArray();
        if(tangents==NULL || tangents->size()==0) {
          cerr << "Failed to generate tangents for DrawObject " << id_ << "!" << endl;
          return;
        }


#if (OPENSCENEGRAPH_MAJOR_VERSION < 3 || ( OPENSCENEGRAPH_MAJOR_VERSION == 3 && OPENSCENEGRAPH_MINOR_VERSION < 2))
        geom->setVertexAttribData( TANGENT_UNIT, osg::Geometry::ArrayData( tangents, osg::Geometry::BIND_PER_VERTEX ) );
#elif (OPENSCENEGRAPH_MAJOR_VERSION > 3 || (OPENSCENEGRAPH_MAJOR_VERSION == 3 && OPENSCENEGRAPH_MINOR_VERSION >= 2))
        geom->setVertexAttribArray( TANGENT_UNIT,
                                   tangents, osg::Array::BIND_PER_VERTEX );
#else
  #error Unknown OSG Version
#endif
    }

    void DrawObject::removeBits(unsigned int bits) {
      nodeMask_ &= ~bits;
      posTransform_->setNodeMask(nodeMask_);
    }
    void DrawObject::setBits(unsigned int bits) {
      nodeMask_ = bits;
      posTransform_->setNodeMask(nodeMask_);
    }

    void DrawObject::setShowSelected(bool val) {
      showSelected = val;
      setSelected(selected_);
    }
    
    void DrawObject::setSelected(bool val) {
      selected_ = val;
      if(selectable_) {
        osg::PolygonMode *polyModeObj;

        osg::StateSet *state = group_->getOrCreateStateSet();

        polyModeObj = dynamic_cast<osg::PolygonMode*>
          (state->getAttribute(osg::StateAttribute::POLYGONMODE));

        if (!polyModeObj){
          polyModeObj = new osg::PolygonMode;
          state->setAttribute( polyModeObj );
        }

        if(val && showSelected) {
          polyModeObj->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
          //state->setAttributeAndModes(material_.get(), osg::StateAttribute::OFF);

          state->setAttributeAndModes(selectionMaterial.get(),
                                      osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
          state->setMode(GL_LIGHTING,
                         osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
          state->setMode(GL_FOG, osg::StateAttribute::OFF);

        }
        else {
          polyModeObj->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL);

          state->removeAttribute(selectionMaterial.get());
          //state->setAttributeAndModes(material_.get(), osg::StateAttribute::ON);
          state->removeMode(GL_LIGHTING);
          state->removeMode(GL_FOG);

        }
      }
    }

    void DrawObject::setRenderBinNumber(int number) {
      osg::StateSet *state = stateGroup_->getOrCreateStateSet();
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

    void DrawObject::show() {
      hide();
      isHidden = false;
      if(sharedStateGroup) {
        stateGroup_->addChild(posTransform_.get());
      }
      else {
        mGroup_->addChild(stateGroup_.get());
      }
    }

    void DrawObject::hide() {
      isHidden = true;
      if(sharedStateGroup) {
        stateGroup_->removeChild(posTransform_.get());
      }
      else {
        mGroup_->removeChild(stateGroup_.get());
      }
    }

    void DrawObject::seperateMaterial() {
      if(!sharedStateGroup) {
        mGroup_->removeChild(stateGroup_.get());
        posTransform_->setStateSet(materialState.get());
      }
      else {
        // this will be more complicated
      }

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
      if(sharedStateGroup) return;
      useFog = val;
      if(val) useFogUniform->set(1);
      else useFogUniform->set(0);
    }

    void DrawObject::setUseNoise(bool val) {
      if(sharedStateGroup) return;
      useNoise = val;
      if(sharedStateGroup) return;
      if(val) useNoiseUniform->set(1);
      else useNoiseUniform->set(0);
    }

    void DrawObject::setDrawLineLaser(bool val) {
      if(sharedStateGroup) return;
      drawLineLaser = val;
      if(sharedStateGroup) return;
      if(val) drawLineLaserUniform->set(1);
      else drawLineLaserUniform->set(0);
    }

    void DrawObject::setUseShadow(bool val) {
      if(sharedStateGroup) return;
      if(val) useShadowUniform->set(1);
      else useShadowUniform->set(0);
    }

    void DrawObject::setBrightness(float val) {
      if(sharedStateGroup) return;
      brightnessUniform->set(val);
    }

    void DrawObject::collideSphere(Vector pos, sReal radius) {
    }

    void DrawObject::setExperimentalLineLaser(Vector pos, Vector n,
                                              Vector color, Vector laserAngle,
                                              float openingAngle) {
      if(sharedStateGroup) return;
      lineLaserPosUniform->set(osg::Vec3f(pos.x(), pos.y(), pos.z()));
      lineLaserNormalUniform->set(osg::Vec3f(n.x(), n.y(), n.z()));
      lineLaserColor->set(osg::Vec4f(color.x(), color.y(), color.z(), 1.0f ) );
      lineLaserDirection->set(osg::Vec3f(laserAngle.x(), laserAngle.y(), laserAngle.z()));
      lineLaserOpeningAngle->set( openingAngle);
    }

    void DrawObject::updateLights(std::vector<mars::interfaces::LightData*> &lightList) {
      if(sharedStateGroup) return;
      std::vector<mars::interfaces::LightData*>::iterator it;
      std::list<mars::interfaces::LightData*> lightList_;
      std::list<mars::interfaces::LightData*>::iterator it2;
      double dist1, dist2;

      for(it=lightList.begin(); it!=lightList.end(); ++it) {
        dist1 = Vector((*it)->pos - position_).norm();
        for(it2=lightList_.begin(); it2!=lightList_.end(); ++it2) {
          dist2 = Vector((*it2)->pos - position_).norm();
          if(dist1 < dist2) {
            lightList_.insert(it2, *it);
            break;
          }
        }
        if(it2 == lightList_.end()) {
          lightList_.push_back(*it);
        }
      }
      int i=0;
      for(it2=lightList_.begin(); it2!=lightList_.end() && i<maxNumLights; ++it2, ++i) {
        /*
        fprintf(stderr, "set light: %d\n", i);
        fprintf(stderr, "  pos: %g %g %g\n", (*it2)->pos.x(), (*it2)->pos.y(), (*it2)->pos.z());
        fprintf(stderr, "  ambient: %g %g %g %g\n", (*it2)->ambient.r, (*it2)->ambient.g,
                (*it2)->ambient.b, (*it2)->ambient.a);
        fprintf(stderr, "  diffuse: %g %g %g %g\n", (*it2)->diffuse.r, (*it2)->diffuse.g,
                (*it2)->diffuse.b, (*it2)->diffuse.a);
        */
        lightPosUniform->setElement(i, osg::Vec3f((*it2)->pos.x(),
                                                 (*it2)->pos.y(),
                                                 (*it2)->pos.z()));
        lightAmbientUniform->setElement(i, osg::Vec4f((*it2)->ambient.r,
                                                     (*it2)->ambient.g,
                                                     (*it2)->ambient.b,
                                                     (*it2)->ambient.a));
        lightDiffuseUniform->setElement(i, osg::Vec4f((*it2)->diffuse.r,
                                                     (*it2)->diffuse.g,
                                                     (*it2)->diffuse.b,
                                                     (*it2)->diffuse.a));
        lightSpecularUniform->setElement(i, osg::Vec4f((*it2)->specular.r,
                                                      (*it2)->specular.g,
                                                      (*it2)->specular.b,
                                                      (*it2)->specular.a));
        lightIsSpotUniform->setElement(i, (*it2)->type-1);
        Vector v = (*it2)->lookAt;// - (*it2)->pos;
        lightSpotDirUniform->setElement(i, osg::Vec3f(v.x(), v.y(), v.z()));
        lightIsDirectionalUniform->setElement(i, (*it2)->directional);
        lightConstantAttUniform->setElement(i, (float)(*it2)->constantAttenuation);
        lightLinearAttUniform->setElement(i, (float)(*it2)->linearAttenuation);
        lightQuadraticAttUniform->setElement(i, (float)(*it2)->quadraticAttenuation);
        lightIsSetUniform->setElement(i, 1);
        lightCosCutoffUniform->setElement(i, (float)cos((*it2)->angle*0.017453292519943));
        lightSpotExponentUniform->setElement(i, (float)(*it2)->exponent);
      }
      for(; i<maxNumLights; ++i) {
        lightIsSetUniform->setElement(i, 0);
      }
    }

  } // end of namespace graphics
} // end of namespace mars
