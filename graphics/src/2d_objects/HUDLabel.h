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
 * \file HUDLabel.h
 * \author Malte Langosz
 * \brief The "HUDLabel" 
 */

#ifndef MARS_GRAPHICS_HUDLABEL_H
#define MARS_GRAPHICS_HUDLABEL_H

#include "HUDElement.h"

#include <osg/MatrixTransform>
#include <osgText/Text>
#include <string>

namespace mars {
  namespace graphics {

    class HUDLabel : public HUDElement {

    public:
      HUDLabel(osg::ref_ptr<osg::Group> group);
      HUDLabel(void);
      ~HUDLabel(void);
  
      void setFontSize(double _font_size);
      void setPos(double x, double y);
      void setBackgroundColor(double r, double g, double b, double a);
      void setBorderColor(double r, double g, double b, double a);
      void setBorderWidth(double border_width);
      void setPadding(double left, double top, double right, double bottom);
      void setDirection(int _direction);
      void setText(std::string text, double color[4]);
      osg::Group* getNode(void);
      void switchCullMask(void);
      void xorCullMask(unsigned int mask);

    private:
      osg::ref_ptr<osg::Group> parent;
      osg::ref_ptr<osg::MatrixTransform> scaleTransform;
      osg::ref_ptr<osgText::Text> labelText;

      double posx, posy;
      double background_color[4], border_color[4];
      double border_width;
      double font_size;
      double pl, pt, pr, pb;
      int direction;
      bool label_created;
      unsigned int cull_mask;
      bool visible;

      osg::ref_ptr<osg::Geode> createLabel(std::string label,
                                           double color[4]);

    }; // end of class HUDLabel

  } // end of namespace graphics
} // end of namespace mars

#endif /* MARS_GRAPHICS_HUDLABEL_H */
