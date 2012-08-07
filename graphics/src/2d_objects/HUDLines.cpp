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

/**
 * \file HUD.cpp
 * \author Malte Roemmermann
 * \brief The "HUD" class contains all necessary methods for rendering
 * data into a texture.
 */

#include <osg/GLExtensions>
#include <osg/Node>
#include <osg/Geometry>
#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/Texture2D>
#include <osg/TextureRectangle>
#include <osg/Stencil>
#include <osg/ColorMask>
#include <osg/Depth>
#include <osg/Billboard>
#include <osg/Material>
#include <osg/AnimationPath>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osgUtil/SmoothingVisitor>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osg/LineWidth>
#include <osg/PositionAttitudeTransform>

#include "HUDLines.h"

namespace mars {
  namespace graphics {

    HUDLines::HUDLines(osg::ref_ptr<osg::Group> group)
  : parent(group),
    scaleTransform(new osg::MatrixTransform),
    cull_mask(0xffffffff),
    pl(0), pt(0), pr(0), pb(0),
    visible(true),
    point_size(1.0),
    render_order(10),
    init(true)
    {
      scaleTransform->setMatrix(osg::Matrix::scale(1.0, 1.0, 1.0));
      if(parent.get() != NULL) parent->addChild(scaleTransform);
    }
    HUDLines::HUDLines()
  : parent(NULL),
    scaleTransform(new osg::MatrixTransform),
    cull_mask(0xffffffff),
    pl(0), pt(0), pr(0), pb(0),
    visible(true),
    point_size(1.0),
    render_order(10),
    init(true)
    {
      scaleTransform->setMatrix(osg::Matrix::scale(1.0, 1.0, 1.0));
    }

    HUDLines::~HUDLines(void) {
    }

    void HUDLines::setPos(double x, double y) {
      posx = x;
      posy = y;
    }

    void HUDLines::setViewSize(double width, double height) {
      view_width = width;
      view_height = height;
    }

    void HUDLines::setBackgroundColor(double r, double g, double b, double a) {
      background_color[0] = r;
      background_color[1] = g;
      background_color[2] = b;
      background_color[3] = a;
    }

    void HUDLines::setBorderColor(double r, double g, double b, double a) {
      border_color[0] = r;
      border_color[1] = g;
      border_color[2] = b;
      border_color[3] = a;
    }

    void HUDLines::setBorderWidth(double border_width) {
      this->border_width = border_width;
    }

    void HUDLines::setPadding(double left, double top,
                              double right, double bottom) {
      pl = left;
      pt = top;
      pr = right;
      pb = bottom;
    }

    void HUDLines::setLines(std::vector<double> *_vertices, double color[4]) {
      if(init) {
        init = false;

        osg::ref_ptr<osg::Geode> node = new osg::Geode;
        osg::ref_ptr<osg::Geometry> linesGeom = new osg::Geometry;


        osg::Vec3Array* ver = new osg::Vec3Array((*_vertices).size()/3);
        ver->setDataVariance(osg::Object::DYNAMIC);
        for(int i=0; i<_vertices->size()/3; ++i)
          (*ver)[i].set((*_vertices)[i*3],
                        (*_vertices)[(i*3)+1],
                        (*_vertices)[(i*3)+2]-1.0);

        linesGeom->setDataVariance(osg::Object::DYNAMIC);
        linesGeom->setVertexArray(ver);

        osg::Vec4Array* colors = new osg::Vec4Array;
        colors->push_back(osg::Vec4(color[0], color[1], color[2], color[3]));
        linesGeom->setColorArray(colors);
        linesGeom->setColorBinding(osg::Geometry::BIND_OVERALL);

        // set the normal in the same way color.
        osg::Vec3Array* normals = new osg::Vec3Array;
        normals->push_back(osg::Vec3(0.0f,0.0f,1.0f));
        linesGeom->setNormalArray(normals);
        linesGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);

        linesGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,
                                                       0, (*_vertices).size()/3));


        node->addDrawable(linesGeom.get());

        osg::ref_ptr<osg::LineWidth> linew;
        linew = new osg::LineWidth(point_size);
        node->getOrCreateStateSet()->setAttributeAndModes(linew.get(),
                                                          osg::StateAttribute::ON);
	
        node->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
        node->getOrCreateStateSet()->setRenderBinDetails(render_order, "RenderBin");	

        scaleTransform->addChild(node.get());
        
        osg::BoundingBox bb;
        bb.expandBy(node->getDrawable(0)->getBound());
        float d = bb.zMin()-2.0;
        float w = bb.xMax() - bb.xMin();
        float h = bb.yMax() - bb.yMin();

        // now get the bounding box
        if(background_color[3] > 0.0001) { 
          osg::ref_ptr<osg::Geode> geode = new osg::Geode;
          osg::Geometry* geom = new osg::Geometry;
    
          osg::Vec3Array* vertices = new osg::Vec3Array;
          vertices->push_back(osg::Vec3(posx-pl, posy-h+pt, d));
          vertices->push_back(osg::Vec3(posx-pl, posy+pt, d));
          vertices->push_back(osg::Vec3(posx+w-pl, posy+pt, d));
          vertices->push_back(osg::Vec3(posx+w-pl, posy-h+pt, d));
          geom->setVertexArray(vertices);

          osg::Vec3Array* normals = new osg::Vec3Array;
          normals->push_back(osg::Vec3(0.0f,0.0f,1.0f));
          geom->setNormalArray(normals);
          geom->setNormalBinding(osg::Geometry::BIND_OVERALL);
    
          osg::Vec4Array* colors = new osg::Vec4Array;
          colors->push_back(osg::Vec4(background_color[0], background_color[1],
                                      background_color[2], background_color[3]));
          geom->setColorArray(colors);
          geom->setColorBinding(osg::Geometry::BIND_OVERALL);
    
          geom->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,4));
    
          osg::StateSet* stateset = geom->getOrCreateStateSet();
          //stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
          //stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
          //stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
          //stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
          geode->addDrawable(geom);
          scaleTransform->addChild(geode.get());
        }

        if(border_width > 0.0) {
          osg::ref_ptr<osg::Geometry> linesGeom = new osg::Geometry;
          osg::ref_ptr<osg::Geode> linesGeode = new osg::Geode;
          //nodemanager tempnode;
    
          osg::Vec3Array* vertices = new osg::Vec3Array(8);
          (*vertices)[0].set(posx-pl, posy-h+pt, d);
          (*vertices)[1].set(posx-pl, posy+pt, d);
          (*vertices)[2].set(posx-pl, posy+pt, d);
          (*vertices)[3].set(posx+w-pl, posy+pt, d);
          (*vertices)[4].set(posx+w-pl, posy+pt, d);
          (*vertices)[5].set(posx+w-pl, posy-h+pt, d);
          (*vertices)[6].set(posx+w-pl, posy-h+pt, d);
          (*vertices)[7].set(posx-pl, posy-h+pt, d);

          // pass the created vertex array to the points geometry object.
          linesGeom->setVertexArray(vertices);
    
          // set the colors as before, plus using the above
          osg::Vec4Array* colors2 = new osg::Vec4Array;
          colors2->push_back(osg::Vec4(border_color[0], border_color[1],
                                       border_color[2], border_color[3]));
          linesGeom->setColorArray(colors2);
          linesGeom->setColorBinding(osg::Geometry::BIND_OVERALL);
    
          // set the normal in the same way color.
          osg::Vec3Array* normals = new osg::Vec3Array;
          normals->push_back(osg::Vec3(0.0f,0.0f,1.0f));
          linesGeom->setNormalArray(normals);
          linesGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);
    
          // since we know up front,
          linesGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,
                                                         0,8));

          osg::ref_ptr<osg::LineWidth> linew = new osg::LineWidth(border_width);
          linesGeom->getOrCreateStateSet()->setAttributeAndModes(linew.get(),
                                                                 osg::StateAttribute::ON);

          // add the points geometry to the geode.
          linesGeode->addDrawable(linesGeom.get());
          scaleTransform->addChild(linesGeode.get());
        }
      }
      else {
        osg::Geode* geode;
        osg::Drawable* drawable;
        osg::Geometry* geometry;
        osg::ref_ptr<osg::Vec3Array> v(new osg::Vec3Array());

        if(geode = dynamic_cast<osg::Geode*>(scaleTransform->getChild(0))) {
          if((drawable = geode->getDrawable(0))) {
            //fprintf(stderr, "\nhere we are! 3");
            if((geometry = drawable->asGeometry())) {
              for(unsigned int i=0; i<(*_vertices).size()/3; ++i) {
                v->push_back(osg::Vec3((*_vertices)[i*3],
                                       (*_vertices)[(i*3)+1],
                                       (*_vertices)[(i*3)+2]-1.0));
              }
              geometry->setVertexArray(v.get());
              osg::Vec4Array* colors = new osg::Vec4Array;
              colors->push_back(osg::Vec4(color[0], color[1], color[2], color[3]));
              geometry->setColorArray(colors);
              geometry->dirtyDisplayList();
              geometry->dirtyBound();
              (dynamic_cast<osg::DrawArrays*>(geometry->getPrimitiveSet(0)))->setCount((*_vertices).size()/3);

            }
          }
        }

      }
    }

    osg::Group* HUDLines::getNode(void) {
      if(parent.get() == NULL)
        return scaleTransform.get();
      else
        return parent.get();
    }

    void HUDLines::resize(double _width, double _height) {
      double scale_x = _width / view_width;
      double scale_y = _height / view_height;

      scaleTransform->setMatrix(osg::Matrix::scale(scale_x, scale_y, 1.0));
    }

    void HUDLines::switchCullMask() {
      if(visible) {
        visible = false;
        scaleTransform->setNodeMask(0);
      }
      else {
        visible = true;
        scaleTransform->setNodeMask(cull_mask);
      }
    }

    void HUDLines::xorCullMask(unsigned int mask) {
      cull_mask = cull_mask^mask;
      scaleTransform->setNodeMask(cull_mask);
    }

  } // end of namespace graphics
} // end of namespace mars
