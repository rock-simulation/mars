/*
 *  Copyright 2014, DFKI GmbH Robotics Innovation Center
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

/**
 * \file ShadowMap.cpp
 * \author Malte Langosz
 * \brief The ShadowMap is a clone of the original osgShadow::ShadowMap but
 *        allows to render the shadow texture in a given area of
 *        a defined node
*/

#include "ShadowMap.h"

#include <osgShadow/ShadowedScene>
#include <osg/Notify>
#include <osg/ComputeBoundsVisitor>
#include <osg/PolygonOffset>
#include <osg/CullFace>
#include <osg/io_utils>

#include <cstdio>

using namespace osgShadow;

#include <iostream>
//for debug
#include <osg/LightSource>
#include <osg/PolygonMode>
#include <osg/Geometry>
#include <osgDB/ReadFile>
#include <osgText/Text>

namespace mars {
  namespace graphics {

    ShadowMap::ShadowMap() {
      shadowTextureUnit = 2;
      centerObject = NULL;
      radius = 1.0;
      shadowTextureSize = 2048;
      // create own uniforms
      createUniforms();
    }

    void ShadowMap::setLight(osg::Light *l) {
      light = l;
    }


    void ShadowMap::setLight(osg::LightSource *l) {
      ls = l;
      light = ls->getLight();
    }

    void ShadowMap::createUniforms() {
      uniformList.clear();
      osg::Uniform* sampler = new osg::Uniform("osgShadow_shadowTexture",
                                               (int)shadowTextureUnit);
      uniformList.push_back(sampler);
      ambientBiasUniform = new osg::Uniform("osgShadow_ambientBias",
                                            osg::Vec2(0.5f,0.5f));
      uniformList.push_back(ambientBiasUniform.get());
      textureScaleUniform = new osg::Uniform("osgShadow_textureScale",
                                            1.0f);
      uniformList.push_back(textureScaleUniform.get());
    }

    void ShadowMap::initTexture() {
      texture = new osg::Texture2D;
      texture->setTextureSize(shadowTextureSize, shadowTextureSize);
      texture->setInternalFormat(GL_DEPTH_COMPONENT);
      texture->setShadowComparison(true);
      texture->setShadowTextureMode(osg::Texture2D::LUMINANCE);
      texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
      texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);

      // the shadow comparison should fail if object is outside the texture
      texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_BORDER);
      texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_BORDER);
      texture->setBorderColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    }

    void ShadowMap::applyState(osg::StateSet* state) {
      state->setTextureAttributeAndModes(shadowTextureUnit,texture.get(),
                                         osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
      state->setTextureMode(shadowTextureUnit, GL_TEXTURE_GEN_S,
                            osg::StateAttribute::ON);
      state->setTextureMode(shadowTextureUnit, GL_TEXTURE_GEN_T,
                            osg::StateAttribute::ON);
      state->setTextureMode(shadowTextureUnit, GL_TEXTURE_GEN_R,
                            osg::StateAttribute::ON);
      state->setTextureMode(shadowTextureUnit, GL_TEXTURE_GEN_Q,
                            osg::StateAttribute::ON);

      // add the uniform list to the stateset
      for(std::vector< osg::ref_ptr<osg::Uniform> >::const_iterator itr=uniformList.begin();
          itr!=uniformList.end(); ++itr) {
        state->addUniform(itr->get());
      }
    }

    void ShadowMap::removeTexture(osg::StateSet* state) {
      state->setTextureAttributeAndModes(shadowTextureUnit,texture.get(),
                                         osg::StateAttribute::OFF);
    }

    void ShadowMap::addTexture(osg::StateSet* state) {
      state->setTextureAttributeAndModes(shadowTextureUnit,texture.get(),
                                         osg::StateAttribute::ON);
    }

    void ShadowMap::init() {
      if (!_shadowedScene) return;

      // set up the render to texture camera.
      {
        // create the camera
        camera = new osg::Camera;
        camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF_INHERIT_VIEWPOINT);
        camera->setCullCallback(new CameraCullCallback(this));
        camera->setClearMask(GL_DEPTH_BUFFER_BIT);
        //_camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        camera->setClearColor(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
        camera->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);
        // set viewport
        camera->setViewport(0, 0, shadowTextureSize, shadowTextureSize);
        // set the camera to render before the main camera.
        camera->setRenderOrder(osg::Camera::PRE_RENDER);
        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
        //camera->setRenderTargetImplementation(osg::Camera::SEPERATE_WINDOW);
        // attach the texture and use it as the color buffer.
        camera->attach(osg::Camera::DEPTH_BUFFER, texture.get());
        osg::StateSet* stateset = camera->getOrCreateStateSet();

        // cull front faces so that only backfaces contribute to depth map
        osg::ref_ptr<osg::CullFace> cull_face = new osg::CullFace;
        cull_face->setMode(osg::CullFace::FRONT);
        stateset->setAttribute(cull_face.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        stateset->setMode(GL_CULL_FACE, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

        // negative polygonoffset - move the backface nearer to the eye point so that backfaces
        // shadow themselves
        float factor = 1.2;
        float units =  1.2;

        osg::ref_ptr<osg::PolygonOffset> polygon_offset = new osg::PolygonOffset;
        polygon_offset->setFactor(factor);
        polygon_offset->setUnits(units);
        stateset->setAttribute(polygon_offset.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        stateset->setMode(GL_POLYGON_OFFSET_FILL, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
      }

      {
        stateset = new osg::StateSet;
        /* should be applied to globalstateset for shader */

        stateset->setTextureAttributeAndModes(shadowTextureUnit,texture.get(),
                                              osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        stateset->setTextureMode(shadowTextureUnit, GL_TEXTURE_GEN_S,
                                 osg::StateAttribute::ON);
        stateset->setTextureMode(shadowTextureUnit, GL_TEXTURE_GEN_T,
                                 osg::StateAttribute::ON);
        stateset->setTextureMode(shadowTextureUnit, GL_TEXTURE_GEN_R,
                                 osg::StateAttribute::ON);
        stateset->setTextureMode(shadowTextureUnit, GL_TEXTURE_GEN_Q,
                                 osg::StateAttribute::ON);

        texgen = new osg::TexGen;
      }

      _dirty = false;
    }

    void ShadowMap::updateTexScale() {
      fprintf(stderr, "texscale: %g\n", 1./(texscale*2));
      textureScaleUniform->set(0);//1./(texscale*1000));
    }

    void ShadowMap::update(osg::NodeVisitor& nv) {
      //if(!renderShadow) return;
      _shadowedScene->osg::Group::traverse(nv);
    }

    void ShadowMap::cull(osgUtil::CullVisitor& cv) {
      // record the traversal mask on entry so we can reapply it later.
      unsigned int traversalMask = cv.getTraversalMask();

      osgUtil::RenderStage* orig_rs = cv.getRenderStage();

      // do traversal of shadow recieving scene which does need to be decorated by the shadow map
      {
        cv.pushStateSet(stateset.get());
        _shadowedScene->osg::Group::traverse(cv);
        cv.popStateSet();
      }

      // need to compute view frustum for RTT camera.
      // 1) get the light position
      // 2) get the center and extents of the view frustum

      const osg::Light* selectLight = 0;
      osg::Vec4 lightpos;
      osg::Vec3 lightDir;

      //MR testing giving a specific light
      osgUtil::PositionalStateContainer::AttrMatrixList& aml = orig_rs->getPositionalStateContainer()->getAttrMatrixList();
      for(osgUtil::PositionalStateContainer::AttrMatrixList::iterator itr = aml.begin(); itr != aml.end(); ++itr) {
        const osg::Light* light_ = dynamic_cast<const osg::Light*>(itr->first.get());
        if (light_) {
          if( light.valid()) {
            if( light.get() == light_ )
              selectLight = light_;
            else
              continue;
          }
          else
            selectLight = light_;

          osg::RefMatrix* matrix = itr->second.get();
          if (matrix) {
            lightpos = light_->getPosition() * (*matrix);
            lightDir = osg::Matrix::transform3x3( light_->getDirection(),
                                                  *matrix );
          }
          else {
            lightpos = light_->getPosition();
            lightDir = light_->getDirection();
          }
        }
      }

      osg::Matrix eyeToWorld;
      eyeToWorld.invert(*cv.getModelViewMatrix());

      lightpos = lightpos * eyeToWorld;
      lightDir = osg::Matrix::transform3x3( lightDir, eyeToWorld );
      lightDir.normalize();

      if (selectLight) {

        // set to ambient on light to black so that the ambient bias uniform can take it's affect
        const_cast<osg::Light*>(selectLight)->setAmbient(osg::Vec4(0.0f,0.0f,0.0f,1.0f));

        //std::cout<<"----- VxOSG::ShadowMap selectLight spot cutoff "<<selectLight->getSpotCutoff()<<std::endl;

        texscale = 1000;
        float fov = selectLight->getSpotCutoff() * 2;
        if(fov < 180.0f) {  // spotlight, then we don't need the bounding box
          osg::Vec3 position(lightpos.x(), lightpos.y(), lightpos.z());
          camera->setProjectionMatrixAsPerspective(fov, 1.0, 0.1, 1000.0);
          camera->setViewMatrixAsLookAt(position,position+lightDir,computeOrthogonalVector(lightDir));
        }
        else {
          // get the bounds of the model.
          osg::ComputeBoundsVisitor cbbv(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN);
          cbbv.setTraversalMask(getShadowedScene()->getCastsShadowTraversalMask());

          _shadowedScene->osg::Group::traverse(cbbv);

          osg::BoundingBox bb = cbbv.getBoundingBox();

          if (lightpos[3]!=0.0) {   // point Light
            if(centerObject) {
              osg::Vec3 position(lightpos.x(), lightpos.y(), lightpos.z());
              mars::utils::Vector v = centerObject->getPosition();
              osg::Vec3 centerPos(v.x(), v.y(), v.z());
              float centerDistance = (position-centerPos).length();
              float znear = centerDistance-radius;
              float zfar  = centerDistance+radius;
              float zNearRatio = 0.001f;
              if (znear<zfar*zNearRatio) znear = zfar*zNearRatio;
              float top   = (radius/centerDistance)*znear;
              float right = top;
              texscale = top*zfar/znear;
              camera->setProjectionMatrixAsFrustum(-right, right, -top,
                                                   top, znear, zfar);
              camera->setViewMatrixAsLookAt(position, centerPos,
                                            computeOrthogonalVector(centerPos-position));
            }
            else {
              osg::Vec3 position(lightpos.x(), lightpos.y(), lightpos.z());
              float centerDistance = (position-bb.center()).length();
              float znear = centerDistance-bb.radius();
              float zfar  = centerDistance+bb.radius();
              float zNearRatio = 0.001f;
              if (znear<zfar*zNearRatio) znear = zfar*zNearRatio;
              float top   = (bb.radius()/centerDistance)*znear;
              float right = top;
              texscale = top*zfar/znear;
              camera->setProjectionMatrixAsFrustum(-right, right, -top,
                                                   top, znear, zfar);
              camera->setViewMatrixAsLookAt(position, bb.center(),
                                            computeOrthogonalVector(bb.center()-position));
            }
          }
          else {   // directional light
            if(centerObject) {
              osg::Vec3 lightDir(lightpos.x(), lightpos.y(), lightpos.z());
              lightDir.normalize();
              // set the position far away along the light direction
              mars::utils::Vector v = centerObject->getPosition();
              osg::Vec3 centerPos(v.x(), v.y(), v.z());
              osg::Vec3 position = centerPos + lightDir * radius * 2;
              float centerDistance = (position-centerPos).length();
              float znear = centerDistance-radius;
              float zfar  = centerDistance+radius;
              float zNearRatio = 0.001f;
              if (znear<zfar*zNearRatio) znear = zfar*zNearRatio;
              float top   = (radius/centerDistance)*znear;
              float right = top;
              texscale = top*zfar/znear;
              camera->setProjectionMatrixAsFrustum(-right, right, -top,
                                                   top, znear, zfar);
              camera->setViewMatrixAsLookAt(position, centerPos,
                                            computeOrthogonalVector(centerPos-position));
            }
            else {
              // make an orthographic projection
              osg::Vec3 lightDir(lightpos.x(), lightpos.y(), lightpos.z());
              lightDir.normalize();
              // set the position far away along the light direction
              osg::Vec3 position = bb.center() + lightDir * bb.radius() * 2;
              float centerDistance = (position-bb.center()).length();
              float znear = centerDistance-bb.radius();
              float zfar  = centerDistance+bb.radius();
              float zNearRatio = 0.001f;
              if (znear<zfar*zNearRatio) znear = zfar*zNearRatio;
              float top   = (bb.radius()/centerDistance)*znear;
              float right = top;
              texscale = top*zfar/znear;
              camera->setProjectionMatrixAsFrustum(-right, right, -top,
                                                   top, znear, zfar);
              camera->setViewMatrixAsLookAt(position, bb.center(),
                                            computeOrthogonalVector(bb.center()-position));
              /*
              float centerDistance = (position-bb.center()).length();
              float znear = centerDistance-bb.radius();
              float zfar  = centerDistance+bb.radius();
              float zNearRatio = 0.001f;
              if (znear<zfar*zNearRatio) znear = zfar*zNearRatio;
              float top   = bb.radius();
              float right = top;

              camera->setProjectionMatrixAsOrtho(-right, right, -top,
                                                 top, znear, zfar);
              camera->setViewMatrixAsLookAt(position, bb.center(),
                                            computeOrthogonalVector(lightDir));
              */
            }
          }
        }

        cv.setTraversalMask( traversalMask &
                             getShadowedScene()->getCastsShadowTraversalMask() );

        // do RTT camera traversal
        camera->accept(cv);
        texgen->setMode(osg::TexGen::EYE_LINEAR);

#if IMPROVE_TEXGEN_PRECISION
        // compute the matrix which takes a vertex from local coords into tex coords
        // We actually use two matrices one used to define texgen
        // and second that will be used as modelview when appling to OpenGL
        texgen->setPlanesFromMatrix( camera->getProjectionMatrix() *
                                     osg::Matrix::translate(1.0,1.0,1.0) *
                                     osg::Matrix::scale(0.5f,0.5f,0.5f) );

        // Place texgen with modelview which removes big offsets (making it float friendly)
        osg::RefMatrix * refMatrix = new osg::RefMatrix
          ( camera->getInverseViewMatrix() * *cv.getModelViewMatrix() );

        cv.getRenderStage()->getPositionalStateContainer()->
          addPositionedTextureAttribute( shadowTextureUnit, refMatrix, texgen.get() );
#else
        // compute the matrix which takes a vertex from local coords into tex coords
        // will use this later to specify osg::TexGen..
        osg::Matrix MVPT = camera->getViewMatrix() *
          camera->getProjectionMatrix() *
          osg::Matrix::translate(1.0,1.0,1.0) *
          osg::Matrix::scale(0.5f,0.5f,0.5f);

        texgen->setPlanesFromMatrix(MVPT);

        orig_rs->getPositionalStateContainer()->addPositionedTextureAttribute(shadowTextureUnit, cv.getModelViewMatrix(), texgen.get());
#endif
      }


      // reapply the original traversal mask
      cv.setTraversalMask( traversalMask );
    }

  } // end of namespace graphics
} // end of namespace mars
