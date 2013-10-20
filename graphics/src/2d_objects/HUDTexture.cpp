/*
 *  Copyright 2011, 2012, 2013, DFKI GmbH Robotics Innovation Center
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
 * \file HUD.cpp
 * \author Malte Langosz
 * \brief The "HUD" class contains all necessary methods for rendering
 * data into a texture.
 */

#include <osg/GLExtensions>
#include <osg/Node>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/Texture2D>
#include <osg/TextureRectangle>
#include <osg/Stencil>
#include <osg/ColorMask>
#include <osg/Depth>
#include <osg/Material>

#include <osg/LineWidth>
#include <osg/PositionAttitudeTransform>

#include <cstdio>

#include "HUDTexture.h"

namespace mars {
  namespace graphics {

    HUDTexture::HUDTexture(osg::Group* group)
      : parent(group),
        scaleTransform(new osg::MatrixTransform),
        cull_mask(0xffffffff),
        visible(true),
        texture_type(TEXTURE_UNKNOWN)
    {
      scaleTransform->setMatrix(osg::Matrix::scale(1.0, 1.0, 1.0));

      posTransform = new osg::PositionAttitudeTransform();
      posTransform->setPivotPoint(osg::Vec3(0.0, 0.0, 0.0));
      posTransform->setPosition(osg::Vec3(0.0, 0.0, 0.0));
      scaleTransform->addChild(posTransform.get());

      if(!parent.valid()) {
        parent = new osg::Group;
      }
    }

    HUDTexture::HUDTexture()
      : parent(new osg::Group),
        scaleTransform(new osg::MatrixTransform),
        cull_mask(0xffffffff),
        visible(true),
        texture_type(TEXTURE_UNKNOWN)
    {
      scaleTransform->setMatrix(osg::Matrix::scale(1.0, 1.0, 1.0));
      posTransform = new osg::PositionAttitudeTransform();
      posTransform->setPivotPoint(osg::Vec3(0.0, 0.0, 0.0));
      posTransform->setPosition(osg::Vec3(0.0, 0.0, 0.0));
      scaleTransform->addChild(posTransform.get());
    }

    HUDTexture::~HUDTexture(void) {
    }

    void HUDTexture::setSize(double width, double height) {
      this->width = width;
      this->height = height;
    }

    void HUDTexture::setTextureSize(double width, double height) {
      t_width = width;
      t_height = height;
    }

    void HUDTexture::setPos(double x, double y) {
      posx = x;
      posy = y;
      posTransform->setPosition(osg::Vec3(posx, posy, 0.0));  
    }

    void HUDTexture::setBorderColor(double r, double g, double b, double a) {
      border_color[0] = r;
      border_color[1] = g;
      border_color[2] = b;
      border_color[3] = a;
    }

    void HUDTexture::setBorderWidth(double border_width) {
      this->border_width = border_width;
    }

    void HUDTexture::createBox(void) {
      osg::ref_ptr<osg::Geometry> quadGeom = new osg::Geometry;
      osg::Vec3Array* corners = new osg::Vec3Array(4);
      osg::Vec2Array* texcoords = new osg::Vec2Array(4);

      geode = new osg::Geode;

      (*corners)[0].set(0, 0, -1.5f);
      (*corners)[1].set(0 + width, 0, -1.5f);
      (*corners)[2].set(0 + width, 0 + height, -1.5f);
      (*corners)[3].set(0, 0 + height, -1.5f);
    
      (*texcoords)[0].set(0.0f, 0.0f);
      (*texcoords)[1].set(1.0f, 0.0f);
      (*texcoords)[2].set(1.0f, 1.0f);
      (*texcoords)[3].set(0.0f, 1.0f);
  
      quadGeom->setVertexArray(corners);
      quadGeom->setTexCoordArray(0,texcoords);
  
  
      osg::Vec4Array* colours = new osg::Vec4Array(1);
  
      (*colours)[0].set(1.0, 1.0, 1.0, 1.0);
  
      quadGeom->setColorArray(colours);
      quadGeom->setColorBinding(osg::Geometry::BIND_OVERALL);
    
      osg::Vec3Array* normals = new osg::Vec3Array(1);
    
      (*normals)[0].set(0, 0, 1);
      (*normals)[0].normalize();
    
      quadGeom->setNormalArray(normals);
      quadGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);
      //quadGeom->setDataVariance(osg::Object::DYNAMIC);
  
      quadGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));
  
      geode->addDrawable(quadGeom.get());
      
      image = new osg::Image();
      image->setImage(t_width, t_height, 1, GL_RGBA,
                      GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, 
                      (unsigned char*)malloc(t_width*t_height*4),
                      osg::Image::NO_DELETE);
      texture = new osg::Texture2D();
      texture->setDataVariance(osg::Object::DYNAMIC);
      texture->setImage(image.get());
      /*
      geode->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture.get(),
                                                                osg::StateAttribute::ON);
*/
      posTransform->addChild(geode.get());
  
      if(border_width > 0.0) {
        osg::ref_ptr<osg::Geometry> linesGeom = new osg::Geometry;
        osg::ref_ptr<osg::Geode> linesGeode = new osg::Geode;
        //nodemanager tempnode;
    
        osg::Vec3Array* vertices = new osg::Vec3Array(8);
        (*vertices)[0].set(0, 0, -2.0f);
        (*vertices)[1].set(0+width, 0, -2.0f);
        (*vertices)[2].set(0+width, 0, -2.0f);
        (*vertices)[3].set(0+width, 0+height, -2.0f);
        (*vertices)[4].set(0+width, 0+height, -2.0f);
        (*vertices)[5].set(0, 0+height, -2.0f);
        (*vertices)[6].set(0, 0+height, -2.0f);
        (*vertices)[7].set(0, 0, -2.0f);

        // pass the created vertex array to the points geometry object.
        linesGeom->setVertexArray(vertices);
    
        // set the colors as before, plus using the above
        osg::Vec4Array* colors2 = new osg::Vec4Array;
        colors2->push_back(osg::Vec4(border_color[0], border_color[1],
                                     border_color[2], border_color[3]));
        linesGeom->setColorArray(colors2);
        linesGeom->setColorBinding(osg::Geometry::BIND_OVERALL);
    
        // set the normal in the same way color.
        linesGeom->setNormalArray(normals);
        linesGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);
    
        // This time we simply use primitive, and hardwire the number of coords to use
        // since we know up front,
        linesGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,8));

        osg::ref_ptr<osg::LineWidth> linew = new osg::LineWidth(border_width);
        linesGeom->getOrCreateStateSet()->setAttributeAndModes(linew.get(),
                                                               osg::StateAttribute::ON);

        // add the points geometry to the geode.
        linesGeode->addDrawable(linesGeom.get());
        posTransform->addChild(linesGeode.get());
      }
      osg::StateSet* stateset = parent->getOrCreateStateSet();

      stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
      stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
      stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
      stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
      //stateset->setRenderBinDetails(30, "RenderBin");
      parent->addChild(scaleTransform.get());
    }

    osg::Group* HUDTexture::getNode(void) {
      return parent.get();
    }

    void HUDTexture::switchCullMask() {
      if(visible) {
        visible = false;
        scaleTransform->setNodeMask(0);
      }
      else {
        visible = true;
        scaleTransform->setNodeMask(cull_mask);
      }
    }

    void HUDTexture::xorCullMask(unsigned int mask) {
      cull_mask = cull_mask^mask;
      scaleTransform->setNodeMask(cull_mask);
    }

    void HUDTexture::setImageData(void *data) {
      memcpy(image->data(), data, t_width*t_height*4);
      image->dirty();
      if (texture_type != TEXTURE_IMAGE) {
        texture->setImage(image.get());
        geode->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture.get(),
                                                                  osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
        texture_type = TEXTURE_IMAGE;
      }
    }

    void HUDTexture::setTexture(osg::Texture2D *texture) {
      if(texture) {
        geode->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture,
                                                                  osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);        
      }
      else {
        geode->getOrCreateStateSet()->setTextureAttributeAndModes(0, this->texture.get(),
                                                                  osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);    
      }
      texture_type = TEXTURE_RTT;
    }

  } // end of namespace graphics
} // end of namespace mars
