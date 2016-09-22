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
 * \file ShadowMap.h
 * \author Malte Langosz
 * \brief The ShadowMap is a clone of the original osgShadow::ShadowMap but
 *        allows to render the shadow texture in a given area of
 *        a defined node
 */

#ifndef MARS_GRAPHICS_SHADOW_MAP_H
#define MARS_GRAPHICS_SHADOW_MAP_H

#include <osg/Camera>
#include <osg/Material>
#include <osg/MatrixTransform>
#include <osg/Object>
#include <osgShadow/ShadowTechnique>

#include "DrawObject.h"

namespace mars {
  namespace graphics {

    class ShadowMap  : public osgShadow::ShadowTechnique {

    public:
      ShadowMap(void);
      ShadowMap(const ShadowMap& copy,
		const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
      virtual ~ShadowMap(void) {}

      META_Object(mars_graphics, ShadowMap);

      void setLight(osg::Light* light);
      void setLight(osg::LightSource* ls);

      virtual void init();
      virtual void update(osg::NodeVisitor& nv);
      virtual void cull(osgUtil::CullVisitor& cv);
      virtual void cleanSceneGraph() {}

      void setCenterObject(DrawObject *cO) {
        centerObject = cO;
      }
      void setRadius(double v) {
        radius = v;
      }

      void setShadowTextureSize(int v) {
        shadowTextureSize = v;
      }

      void initTexture();
      osg::Texture2D* getTexture() {
        return texture.get();
      }
      void applyState(osg::StateSet *state);
      void removeTexture(osg::StateSet* state);
      void addTexture(osg::StateSet* state);
      void updateTexScale();
      float getTexScale() {
        return texscale;
      }

      virtual void resizeGLObjectBuffers(unsigned int maxSize);
      virtual void releaseGLObjects(osg::State* = 0) const;

    protected:
      virtual void createUniforms();

      osg::ref_ptr<osg::Camera> camera;
      osg::ref_ptr<osg::TexGen> texgen;
      osg::ref_ptr<osg::Texture2D> texture;
      osg::ref_ptr<osg::StateSet> stateset;
      osg::ref_ptr<osg::Light> light;
      osg::ref_ptr<osg::LightSource>  ls;
      DrawObject* centerObject;
      double radius;

      osg::ref_ptr<osg::Uniform> ambientBiasUniform;
      osg::ref_ptr<osg::Uniform> textureScaleUniform;
      osg::ref_ptr<osg::Uniform> texGenMatrixUniform;
      std::vector< osg::ref_ptr<osg::Uniform> > uniformList;
      unsigned int shadowTextureUnit;
      int shadowTextureSize;
      float texscale;
    }; // end of class ShadowMap

  } // end of namespace graphics
} // end of namespace mars

#endif /* MARS_GRAPHICS_SHADOW_MAP_H */
