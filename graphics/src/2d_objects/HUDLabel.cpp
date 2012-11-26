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

#include "HUDLabel.h"

namespace mars {
  namespace graphics {

    HUDLabel::HUDLabel(osg::ref_ptr<osg::Group> group)
      : parent(group),
        scaleTransform(new osg::MatrixTransform),
        labelText(0),
        pl(0), pt(0), pr(0), pb(0),
        direction(0),
        cull_mask(0xffffffff),
        visible(true)
    {
      scaleTransform->setMatrix(osg::Matrix::scale(1.0, 1.0, 1.0));
      if(parent.get() != NULL) parent->addChild(scaleTransform);
    }

    HUDLabel::HUDLabel()
      : parent(NULL),
        scaleTransform(new osg::MatrixTransform),
        labelText(0),
        pl(0), pt(0), pr(0), pb(0),
        direction(0),
        cull_mask(0xffffffff),
        visible(true)
    {
      scaleTransform->setMatrix(osg::Matrix::scale(1.0, 1.0, 1.0));
    }

    HUDLabel::~HUDLabel(void) {
    }

    void HUDLabel::setFontSize(double _font_size) {
      font_size = _font_size;
    }

    void HUDLabel::setPos(double x, double y) {
      posx = x;
      posy = y;
    }

    void HUDLabel::setViewSize(double width, double height) {
      view_width = width;
      view_height = height;
    }

    void HUDLabel::setBackgroundColor(double r, double g, double b, double a) {
      background_color[0] = r;
      background_color[1] = g;
      background_color[2] = b;
      background_color[3] = a;
    }

    void HUDLabel::setBorderColor(double r, double g, double b, double a) {
      border_color[0] = r;
      border_color[1] = g;
      border_color[2] = b;
      border_color[3] = a;
    }

    void HUDLabel::setBorderWidth(double border_width) {
      this->border_width = border_width;
    }

    void HUDLabel::setPadding(double left, double top,
                              double right, double bottom) {
      pl = left;
      pt = top;
      pr = right;
      pb = bottom;
    }

    void HUDLabel::setText(std::string text, double color[4]) {
      if(labelText == 0) {
        osg::Vec3 t_pos;
        osg::ref_ptr<osg::Geode> node;
        osg::ref_ptr<osg::PositionAttitudeTransform> transform;
    
        node = createLabel(text, color);
    
        transform = new osg::PositionAttitudeTransform();
        transform->addChild(node.get());
    
        scaleTransform->addChild(transform.get());
        //scaleTransform->addChild(node.get());
    
        osg::BoundingBox bb;
        bb.expandBy(node->getDrawable(0)->getBound());
        float d = bb.zMin()-2.0;
        float w = bb.xMax() - bb.xMin() + pl + pr;
        float h = bb.yMax() - bb.yMin() + pt + pb;

        double pos;
        if(direction) pos = posx - w;
        else pos = posx;

        transform->setPosition(osg::Vec3(pos, posy, -1.5f));
  

        // now get the bounding box
        if(background_color[3] > 0.0001) { 
          osg::ref_ptr<osg::Geode> geode = new osg::Geode;
          osg::Geometry* geom = new osg::Geometry;
    
          osg::Vec3Array* vertices = new osg::Vec3Array;
          vertices->push_back(osg::Vec3(pos-pl, posy-h+pt, d));
          vertices->push_back(osg::Vec3(pos-pl, posy+pt, d));
          vertices->push_back(osg::Vec3(pos+w-pl, posy+pt, d));
          vertices->push_back(osg::Vec3(pos+w-pl, posy-h+pt, d));
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
          stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
          stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
          stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
          stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
          stateset->setRenderBinDetails(21, "RenderBin");
          geode->addDrawable(geom);
          scaleTransform->addChild(geode.get());
        }

        if(border_width > 0.0) {
          osg::ref_ptr<osg::Geometry> linesGeom = new osg::Geometry;
          osg::ref_ptr<osg::Geode> linesGeode = new osg::Geode;
          //nodemanager tempnode;
    
          osg::Vec3Array* vertices = new osg::Vec3Array(8);
          (*vertices)[0].set(pos-pl, posy-h+pt, d);
          (*vertices)[1].set(pos-pl, posy+pt, d);
          (*vertices)[2].set(pos-pl, posy+pt, d);
          (*vertices)[3].set(pos+w-pl, posy+pt, d);
          (*vertices)[4].set(pos+w-pl, posy+pt, d);
          (*vertices)[5].set(pos+w-pl, posy-h+pt, d);
          (*vertices)[6].set(pos+w-pl, posy-h+pt, d);
          (*vertices)[7].set(pos-pl, posy-h+pt, d);

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
          linesGeom->getOrCreateStateSet()->setRenderBinDetails(20, "RenderBin");

          // add the points geometry to the geode.
          linesGeode->addDrawable(linesGeom.get());
          scaleTransform->addChild(linesGeode.get());
        }
      }
      else {
        labelText->setText(text);
        labelText->setColor(osg::Vec4(color[0], color[1], color[2], color[3]));
      }
    }

    osg::ref_ptr<osg::Geode> HUDLabel::createLabel(std::string label,
                                                   double color[4]) {
      osg::ref_ptr<osg::Geode> geode = new osg::Geode();
      std::string timesFont = config_path;//Pathes::getStuffPath();
      timesFont.append("/Fonts/arial.ttf");

      labelText = new  osgText::Text;
      geode->addDrawable(labelText.get());

 
      osg::StateSet* stateset = geode->getOrCreateStateSet();
      stateset->setRenderBinDetails(22, "RenderBin");
      stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
      stateset->setMode(GL_BLEND,osg::StateAttribute::OFF);
      //stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
      stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);

      osg::BoundingBox bb;
      bb.expandBy(geode->getDrawable(0)->getBound());
      //float d = bb.zMin()-2.0;
      //float w = bb.xMax() - bb.xMin() + pl + pr;
      //float h = bb.yMax() - bb.yMin() + pt + pb;
	
      //double pos;
      //if(direction) pos = posx - w;
      //else pos = posx;

	
      labelText->setFont(timesFont);
      //labelText->setPosition(osg::Vec3(pos, posy, -1.5f));
      labelText->setPosition(osg::Vec3(0.0f, 0.0f, 0.0f));
      labelText->setCharacterSize(font_size);
      labelText->setAxisAlignment(osgText::Text::XY_PLANE);
      labelText->setAlignment(osgText::Text::LEFT_TOP);
      labelText->setText(label);
      labelText->setColor(osg::Vec4(color[0], color[1], color[2], color[3]));

      return geode;
    }

    osg::Group* HUDLabel::getNode(void) {
      if(parent.get() == NULL)
        return scaleTransform.get();
      else
        return parent.get();
    }

    void HUDLabel::resize(double _width, double _height) {
      double scale_x = _width / view_width;
      double scale_y = _height / view_height;

      scaleTransform->setMatrix(osg::Matrix::scale(scale_x, scale_y, 1.0));
    }

    void HUDLabel::setDirection(int _direction) {
      direction = _direction;
    }

    void HUDLabel::switchCullMask() {
      if(visible) {
        visible = false;
        scaleTransform->setNodeMask(0);
      }
      else {
        visible = true;
        scaleTransform->setNodeMask(cull_mask);
      }
    }

    void HUDLabel::xorCullMask(unsigned int mask) {
      cull_mask = cull_mask^mask;
      scaleTransform->setNodeMask(cull_mask);
    }

  } // end of namespace graphics
} // end of namespace mars
