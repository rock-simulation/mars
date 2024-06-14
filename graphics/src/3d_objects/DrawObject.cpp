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

#include <iostream>

#include <osg/CullFace>
#include <osg/PolygonMode>
#include <osg/ComputeBoundsVisitor>
#include <osg/LineWidth>

#include <osgDB/WriteFile>
#include <osgDB/ReadFile>

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
        nodeMask_(0xfffffff),
        stateFilename_(""),
        selected_(false),
        selectable_(true),
        group_(0),
        posTransform_(0),
        scaleTransform_(0),
        outlineEffect_(0),
        maxNumLights(1),
        g(g),
        sharedStateGroup(false),
        showSelected(true),
        isHidden(true),
        brightness(1.0),
        frame(NULL) {
    }

    DrawObject::~DrawObject() {
      if(materialNode.valid()) materialNode->removeChild(posTransform_.get());
      if(!sharedStateGroup) {
        // todo: remove materialnode from manager
      }
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

      scaleTransform_ = new osg::MatrixTransform();
      scaleTransform_->setMatrix(osg::Matrix::scale(1.0, 1.0, 1.0));
      scaleTransform_->setDataVariance(osg::Node::STATIC);

      posTransform_ = new osg::PositionAttitudeTransform();
      posTransform_->setPivotPoint(osg::Vec3(pivot_.x(), pivot_.y(), pivot_.z()));
      posTransform_->setPosition(osg::Vec3(0.0, 0.0, 0.0));
      posTransform_->setNodeMask(nodeMask_);

      #ifdef USE_OSGFX
      outlineEffect_ = new osgFX::Outline;
      outlineEffect_->setWidth(12);
      outlineEffect_->setColor(osg::Vec4(0,0.75,0,1));
      outlineEffect_->setEnabled(false);
      outlineEffect_->addChild(scaleTransform_.get());      
      posTransform_->addChild(outlineEffect_.get());
      #else
      posTransform_->addChild(scaleTransform_.get());
      #endif

      group_ = new osg::Group;
      if(sharedID) {
        materialNode = g->getSharedStateGroup(sharedID);
        if(materialNode.valid()) {
          sharedStateGroup = true;
        }
        else {
          sharedStateGroup = false;
        }
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
      // note obj export ignores stateset's of groups
      if(materialNode.valid()) {
        osg::ref_ptr<osg::Group> tmpNode = new osg::Group;
        osg::ref_ptr<osg_material_manager::OsgMaterial> material = materialNode->getMaterial();
        if(material.valid()) {
          osg::StateSet *set = material->getStateSet();
          tmpNode->setStateSet(set);
        }
        tmpNode->getOrCreateStateSet()->merge(*(materialNode->getStateSet()));
        tmpNode->addChild(posTransform_.get());
        osgDB::writeNodeFile(*(tmpNode.get()), filename.data());
      }
    }

    // the material struct can also contain a static texture (texture file)
    void DrawObject::setMaterial(const std::string &name) {

      //osg::StateSet *mState = g->getMaterialStateSet(mStruct);
      bool show_ = !isHidden;
      if(materialNode.valid()) {
        // todo: do not show if is already hidden
        hide();
      }
      if(!sharedStateGroup) {
        // todo: remove materialNode from manager
        materialNode = g->getMaterialNode(name);
        materialNode->setBrightness(brightness);
      }
      if(show_) {
        show();
      }
    }

    void DrawObject::setPosition(const Vector &_pos) {
      position_ = _pos;
      posTransform_->setPosition(osg::Vec3(position_.x(), position_.y(), position_.z()));
      if(frame) {
        frame->setPosition(position_.x(), position_.y(), position_.z());
      }
    }

    void DrawObject::setQuaternion(const Quaternion &q) {
      osg::Quat oQuat;
      oQuat.set(q.x(), q.y(), q.z(), q.w());
      posTransform_->setAttitude(oQuat);
      if(frame) {
        frame->setRotation(q.x(), q.y(), q.z(), q.w());
      }
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
      if (selectable_) {
        #ifdef USE_OSGFX
        if (outlineEffect_ == 0) {
          std::cerr << "No Outline effect created for id " << this->id_ << std::endl;
        }

        if (selected_ && showSelected) {
          outlineEffect_->setEnabled(true);
        } else {
          outlineEffect_->setEnabled(false);
        }

        #else
        osg::PolygonMode *polyModeObj;

        osg::StateSet *state = group_->getOrCreateStateSet();
        polyModeObj = dynamic_cast<osg::PolygonMode*>
          (state->getAttribute(osg::StateAttribute::POLYGONMODE));

        if (!polyModeObj){
          polyModeObj = new osg::PolygonMode;
          state->setAttribute( polyModeObj );
        }

        if(val && showSelected) {
          if(!selectShader.valid()) {
            selectShader = new osg::Program;
            char fragmentSource[]=
              "void main(void)\n"
              "{\n"
              "    gl_FragColor = vec4(0, 0.75, 0, 1);\n"
              "}\n";
            selectShader->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragmentSource));
          }
          state->setAttributeAndModes(selectShader.get(), osg::StateAttribute::ON);
          osg::LineWidth *lw = new osg::LineWidth(2);
          state->setAttributeAndModes(lw, osg::StateAttribute::ON);
          polyModeObj->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
          //state->setAttributeAndModes(material_.get(), osg::StateAttribute::OFF);

          state->setAttributeAndModes(selectionMaterial.get(),
                                      osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
          state->setMode(GL_LIGHTING,
                         osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED  | osg::StateAttribute::OVERRIDE);
          state->setMode(GL_FOG, osg::StateAttribute::OFF  | osg::StateAttribute::OVERRIDE);

        }
        else {
          if(selectShader.valid()) {
            state->removeAttribute(selectShader.get());
          }
          polyModeObj->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL);

          state->removeAttribute(selectionMaterial.get());
          //state->setAttributeAndModes(material_.get(), osg::StateAttribute::ON);
          state->removeMode(GL_LIGHTING);
          state->removeMode(GL_FOG);

        }
        #endif
      }
      for(auto it:selectionChilds) {
        it->setSelected(val);
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

    void DrawObject::show() {
      if(!materialNode.valid()) return;
      hide();
      isHidden = false;
      materialNode->addChild(posTransform_.get());
    }

    void DrawObject::hide() {
      if(!materialNode.valid()) return;
      isHidden = true;
      materialNode->removeChild(posTransform_.get());
    }

    void DrawObject::seperateMaterial() {
      if(!materialNode.valid()) return;
      materialNode->removeChild(posTransform_.get());
      posTransform_->setStateSet(materialNode->getStateSet());
      scaleTransform_->setStateSet(materialNode->getMaterial()->getStateSet());
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

    void DrawObject::setBrightness(double v) {
      brightness = v;
      if(materialNode.valid()) {
        materialNode->setBrightness(brightness);
      }
    }

    void DrawObject::collideSphere(Vector pos, sReal radius) {
    }

  } // end of namespace graphics
} // end of namespace mars
