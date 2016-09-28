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
 *  MaterialNode.cpp
 *
 *  Created by Langosz 2016
 */

#include "MaterialNode.h"
#include "OsgMaterial.h"

#include <osg/Geode>
#include <osg/PositionAttitudeTransform>
#include <osgUtil/TangentSpaceGenerator>
#ifdef HAVE_OSG_VERSION_H
  #include <osg/Version>
#else
  #include <osg/Export>
#endif

#include <iostream>

namespace osg_material_manager {

  using namespace mars::utils;

  MaterialNode::MaterialNode()
    : isCreated(false), maxNumLights(1), useFog(true), useNoise(true),
      getLight(true), brightness_(1.0), drawLineLaser(false), shadow(true),
      needTangents(false) {
  }

  MaterialNode::~MaterialNode() {
    if(material.valid()) material->removeMaterialNode(this);
  }

  void MaterialNode::setMaterial(OsgMaterial *m) {
    material = m;
  }

  void MaterialNode::createNodeState() {

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

    osg::StateSet *state = getOrCreateStateSet();
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

    isCreated = true;
  }


  osg::ref_ptr<OsgMaterial> MaterialNode::getMaterial() {
    return material;
  }

  void MaterialNode::setTransparency(float t) {
    if(!isCreated) return;
    // handle transparency
    transparencyUniform->set(1-t);
    osg::StateSet *state = getOrCreateStateSet();
    if (t >= 0.00001) {
      //state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
      state->setMode(GL_ALPHA_TEST, osg::StateAttribute::ON);
      state->setMode(GL_BLEND,osg::StateAttribute::ON);
      //state->setRenderBinDetails(1, "TransparentBin");
      state->setRenderBinDetails(1, "DepthSortedBin");
      /*
      depth = new osg::Depth;
      depth->setWriteMask( false );
      state->setAttributeAndModes(depth.get(),
                                  osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
      */
    }
    else {
      if(depth.valid()) {
        state->removeAttribute(depth.get());
      }
      state->setRenderingHint(osg::StateSet::DEFAULT_BIN);
      state->setMode(GL_ALPHA_TEST, osg::StateAttribute::OFF);
      state->setMode(GL_BLEND,osg::StateAttribute::OFF);
      state->setRenderBinDetails(0, "RenderBin");
    }
  }

  void MaterialNode::setBlending(bool mode) {
    if(!isCreated) return;
    if(blendEquation_ && !mode) {
      osg::StateSet *state = getOrCreateStateSet();
      state->setAttributeAndModes(blendEquation_.get(),
                                  osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF);
    }
    else if(mode) {
      if(!blendEquation_) {
        blendEquation_ = new osg::BlendEquation(osg::BlendEquation::FUNC_ADD);
        //blendEquation->setDataVariance(osg::Object::DYNAMIC);
      }
      osg::StateSet *state = getOrCreateStateSet();
      state->setAttributeAndModes(blendEquation_.get(),
                                  osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
    }
  }

  void MaterialNode::setUseFog(bool val) {
    if(!isCreated) return;
    useFog = val;
    if(val) useFogUniform->set(1);
    else useFogUniform->set(0);
  }

  void MaterialNode::setUseNoise(bool val) {
    if(!isCreated) return;
    useNoise = val;
    if(val) useNoiseUniform->set(1);
    else useNoiseUniform->set(0);
  }

  void MaterialNode::setDrawLineLaser(bool val) {
    if(!isCreated) return;
    drawLineLaser = val;
    if(val) drawLineLaserUniform->set(1);
    else drawLineLaserUniform->set(0);
  }

  void MaterialNode::setUseShadow(bool val) {
    if(!isCreated) return;
    if(val) useShadowUniform->set(1);
    else useShadowUniform->set(0);
  }

  void MaterialNode::setBrightness(float val) {
    if(!isCreated) return;
    brightnessUniform->set(val);
  }

  void MaterialNode::setExperimentalLineLaser(Vector pos, Vector n,
                                              Vector color, Vector laserAngle,
                                              float openingAngle) {
    if(!isCreated) return;
    lineLaserPosUniform->set(osg::Vec3f(pos.x(), pos.y(), pos.z()));
    lineLaserNormalUniform->set(osg::Vec3f(n.x(), n.y(), n.z()));
    lineLaserColor->set(osg::Vec4f(color.x(), color.y(), color.z(), 1.0f ) );
    lineLaserDirection->set(osg::Vec3f(laserAngle.x(), laserAngle.y(), laserAngle.z()));
    lineLaserOpeningAngle->set( openingAngle);
  }

  void MaterialNode::updateLights(std::vector<mars::interfaces::LightData*> &lightList) {
    if(!isCreated) return;
    std::vector<mars::interfaces::LightData*>::iterator it;
    std::list<mars::interfaces::LightData*> lightList_;
    std::list<mars::interfaces::LightData*>::iterator it2;
    double dist1, dist2;
    Vector position_;
    getNodePosition(&position_);
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

  void MaterialNode::generateTangents() {
    if(needTangents) {
      // search for geom children and generate tangents
      TangentVisitor visitor;
      this->accept(visitor);
    }
  }

  void MaterialNode::setNeedTangents(bool v) {
    needTangents = v;
    generateTangents();
  }

  bool MaterialNode::addChild(osg::Node *child) {
    bool success = osg::Group::addChild(child);
    if(success) {
      generateTangents();
    }
    return success;
  }

  void MaterialNode::getNodePosition(mars::utils::Vector *pos) {
    PositionVisitor visitor;
    this->accept(visitor);
    *pos = visitor.pos;
  }

  void TangentVisitor::apply(osg::Node &searchNode){
    // search for geometries and generate tangents for them
    osg::Geode* geode=dynamic_cast<osg::Geode*>(&searchNode);
    if(geode) {
      for(unsigned int i=0; i<geode->getNumDrawables(); ++i) {
        osg::Geometry* geom=dynamic_cast<osg::Geometry*>(geode->getDrawable(i));
        if(geom) {
          generateTangents(geom);
        }
      }
    }

    traverse(searchNode);
  }

  void TangentVisitor::generateTangents(osg::Geometry *geom) {
    osg::ref_ptr< osgUtil::TangentSpaceGenerator > tsg = new osgUtil::TangentSpaceGenerator;
    // color map unit for texture coordinates
    tsg->generate( geom, DEFAULT_UV_UNIT );
    osg::Vec4Array *tangents = tsg->getTangentArray();
    if(tangents==NULL || tangents->size()==0) {
      std::cerr << "Failed to generate tangents for plane!" << std::endl;
    }
    else {
#if (OPENSCENEGRAPH_MAJOR_VERSION < 3 || ( OPENSCENEGRAPH_MAJOR_VERSION == 3 && OPENSCENEGRAPH_MINOR_VERSION < 2))
      geom->setVertexAttribData( TANGENT_UNIT, osg::Geometry::ArrayData( tangents, osg::Geometry::BIND_PER_VERTEX ) );
#elif (OPENSCENEGRAPH_MAJOR_VERSION > 3 || (OPENSCENEGRAPH_MAJOR_VERSION == 3 && OPENSCENEGRAPH_MINOR_VERSION >= 2))
      geom->setVertexAttribArray( TANGENT_UNIT,
                                  tangents, osg::Array::BIND_PER_VERTEX );
#else
#error Unknown OSG Version
#endif
    }
  }

  void PositionVisitor::apply(osg::Node &searchNode){
    // search for the first pos attitude transform and save the position
    osg::PositionAttitudeTransform* transform=dynamic_cast<osg::PositionAttitudeTransform*>(&searchNode);
    if(transform) {
      osg::Vec3d v = transform->getPosition();
      pos.x() = v[0];
      pos.y() = v[1];
      pos.z() = v[2];
    }
    else {
      traverse(searchNode);
    }
  }

} // end of namespace osg_material_manager
