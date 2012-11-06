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

#include <osg/Node>
#include <osg/Geometry>
#include <osg/Geode>
#include <osgText/Text>

#include <osg/LineWidth>
#include <osg/PositionAttitudeTransform>

#include "HUDTerminal.h"
#include <cstdio>

namespace mars {
  namespace graphics {

    HUDTerminal::HUDTerminal() {
      hudBox = new osg::Group;
      hudTerminalList = new osg::Group;

      row_index = 0;
      max_caracters = 10000;
      line_spacing = 1.0;
      cull_mask = 0xffffffff;
      visible = true;
    }

    HUDTerminal::~HUDTerminal(void) {
    }

    void HUDTerminal::setSize(double width, double height) {
      this->width = width;
      this->height = height;
    }

    void HUDTerminal::setFontSize(double _font_size) {
      font_size = _font_size;
    }

    void HUDTerminal::setMaxCaracters(int size) {
      max_caracters = size;
    }

    void HUDTerminal::setLineSpacing(double _line_spacing) {
      line_spacing = _line_spacing;
    }

    void HUDTerminal::setPos(double x, double y) {
      posx = x;
      posy = y;
    }

    void HUDTerminal::setViewSize(double width, double height) {
      view_width = width;
      view_height = height;
    }

    void HUDTerminal::setBackgroundColor(double r, double g, double b, double a) {
      background_color[0] = r;
      background_color[1] = g;
      background_color[2] = b;
      background_color[3] = a;
    }

    void HUDTerminal::setBorderColor(double r, double g, double b, double a) {
      border_color[0] = r;
      border_color[1] = g;
      border_color[2] = b;
      border_color[3] = a;
    }

    void HUDTerminal::setBorderWidth(double border_width) {
      this->border_width = border_width;
    }

    void HUDTerminal::createBox(void) {
      osg::ref_ptr<osg::Geometry> quadGeom = new osg::Geometry;
      osg::ref_ptr<osg::Geode> geode = new osg::Geode;
      osg::Vec3Array* corners = new osg::Vec3Array(4);
      osg::ref_ptr<osg::Vec2Array> texcoords(new osg::Vec2Array());

      (*corners)[0].set(posx, posy, -0.1f);
      (*corners)[1].set(posx + width, posy, -0.1f);
      (*corners)[2].set(posx + width, posy + height, -0.1f);
      (*corners)[3].set(posx, posy + height, -0.1f);
    
      texcoords->push_back(osg::Vec2(0.0f, 0.0f));
      texcoords->push_back(osg::Vec2(1.0f, 0.0f));
      texcoords->push_back(osg::Vec2(1.0f, 1.0f));
      texcoords->push_back(osg::Vec2(0.0f, 1.0f));
  
      quadGeom->setVertexArray(corners);
      quadGeom->setTexCoordArray(0,texcoords.get());
  
      osg::Vec4Array* colours = new osg::Vec4Array(1);
  
      (*colours)[0].set(background_color[0], background_color[1],
                        background_color[2], background_color[3]);
  
      quadGeom->setColorArray(colours);
      quadGeom->setColorBinding(osg::Geometry::BIND_OVERALL);
  
      osg::Vec3Array* normals = new osg::Vec3Array(1);
    
      (*normals)[0].set(0, 0, 1);
      (*normals)[0].normalize();
    
      quadGeom->setNormalArray(normals);
      quadGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);
  
      quadGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,8));
  
      geode->addDrawable(quadGeom.get());
      hudBox->addChild(geode.get());
    
      if(border_width > 0.0) {
        osg::ref_ptr<osg::Geometry> linesGeom = new osg::Geometry;
        osg::ref_ptr<osg::Geode> linesGeode = new osg::Geode;
        //nodemanager tempnode;
    
        osg::Vec3Array* vertices = new osg::Vec3Array(8);
        (*vertices)[0].set(posx, posy, -0.05f);
        (*vertices)[1].set(posx+width, posy, -0.05f);
        (*vertices)[2].set(posx+width, posy, -0.05f);
        (*vertices)[3].set(posx+width, posy+height, -0.05f);
        (*vertices)[4].set(posx+width, posy+height, -0.05f);
        (*vertices)[5].set(posx, posy+height, -0.05f);
        (*vertices)[6].set(posx, posy+height, -0.05f);
        (*vertices)[7].set(posx, posy, -0.05f);

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
        hudBox->addChild(linesGeode.get());
      }
      //osg::StateSet* stateset = hudBox->getOrCreateStateSet();
      //stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
      //stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
      //stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
      //stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
    }

    void HUDTerminal::addText(std::string text, double color[4]) {
      double pos;
      osg::Vec3 t_pos;
      osg::ref_ptr<osg::Geode> node;
      osg::ref_ptr<osg::PositionAttitudeTransform> transform;
      unsigned int max_row_index = (height - 10) / (font_size*line_spacing);
  
      while(text.length() > (unsigned int)max_caracters) {
        addText(text.substr(0, max_caracters), color);
        text = text.substr(max_caracters);
      }

      transform = new osg::PositionAttitudeTransform();
  
      pos = height - row_index*(font_size*line_spacing) - 5;
      node = createLabel(pos, text, color);
      transform->setPosition(osg::Vec3(posx + 3, posy + pos, 0.0f));
      transform->addChild(node.get());
      hudBox->addChild(transform.get());
      hudTerminalList->addChild(transform.get());


      if(hudTerminalList->getNumChildren() > max_row_index) {
        osg::Node* toRemove = hudTerminalList->getChild(0);
        hudBox->removeChild(toRemove);
        hudTerminalList->removeChild(toRemove);
      }
      if(row_index >= (unsigned int)max_row_index) {
        for(unsigned int i=0; i<hudTerminalList->getNumChildren(); i++) {
          transform = dynamic_cast<osg::PositionAttitudeTransform*>(hudTerminalList->getChild(i));
          t_pos = transform->getPosition();
          t_pos[1] += font_size*line_spacing;
          transform->setPosition(t_pos);
        }
      }
      else row_index++;
    }

    osg::ref_ptr<osg::Geode> HUDTerminal::createLabel(double pos, std::string label,
                                                      double color[4]) {
      osg::ref_ptr<osg::Geode> geode = new osg::Geode();
      std::string timesFont = config_path;// = Pathes::getStuffPath();
      timesFont.append("fonts/arial.ttf");

      osgText::Text* text = new  osgText::Text;
      geode->addDrawable( text);

      //osg::StateSet* stateset = geode->getOrCreateStateSet();
      //stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
      //stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
      //stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
      //stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);

      text->setFont(timesFont);
      text->setPosition(osg::Vec3(0.0f, 0.0f, 0.0f));
      text->setCharacterSize(font_size);
      text->setAxisAlignment(osgText::Text::XY_PLANE);
      text->setAlignment(osgText::Text::LEFT_TOP);
      text->setText(label);
      text->setColor(osg::Vec4(color[0], color[1], color[2], color[3]));

      return geode;
    }

    osg::Group* HUDTerminal::getNode(void) {
      return hudBox.get();
    }

    void HUDTerminal::resize(double _width, double _height) {
      (void)_width;
      (void)_height;
    }

    void HUDTerminal::switchCullMask() {
      if(visible) {
        visible = false;
        hudBox->setNodeMask(0);
      }
      else {
        visible = true;
        hudBox->setNodeMask(cull_mask);
      }
    }

    void HUDTerminal::xorCullMask(unsigned int mask) {
      cull_mask = cull_mask^mask;
      hudBox->setNodeMask(cull_mask);
    }

  } // end of namespace graphics
} // end of namespace mars
