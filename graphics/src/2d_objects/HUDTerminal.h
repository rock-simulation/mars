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
 * \file HUDTerminal.h
 * \author Malte Roemmermann
 * \brief The "HUDTerminal" 
 */

#ifndef MARS_GRAPHICS_HUDTERMINAL_H
#define MARS_GRAPHICS_HUDTERMINAL_H

#include "HUDElement.h"

#include <osg/MatrixTransform>
#include <string>

namespace mars {
  namespace graphics {

    class HUDTerminal : public HUDElement {

    public:
      HUDTerminal(void);
      ~HUDTerminal(void);
  
      void setSize(double width, double height);
      void setFontSize(double _font_size);
      void setMaxCaracters(int size);
      void setLineSpacing(double _line_spacing);
      void setPos(double x, double y);
      void setViewSize(double widht, double height);
      void setBackgroundColor(double r, double g, double b, double a);
      void setBorderColor(double r, double g, double b, double a);
      void setBorderWidth(double border_width);
  
      void createBox(void);
      void addText(std::string text, double color[4]);
      void resize(double _width, double _height);
      osg::Group* getNode(void);
      void switchCullMask();
      void xorCullMask(unsigned int mask);

    private:
      osg::ref_ptr<osg::Group> hudBox;
      osg::ref_ptr<osg::Group> hudTerminalList;

      double width, height, view_width, view_height;
      double posx, posy;
      double background_color[4], border_color[4];
      double border_width;
      double font_size;
      double line_spacing;
      unsigned int row_index;
      int max_caracters;
      unsigned int cull_mask;
      bool visible;

      osg::ref_ptr<osg::Geode> createLabel(double pos,
                                           std::string label,
                                           double color[4]);
    }; // end of class HUDTerminal

  } // end of namespace graphics
} // end of namespace mars

#endif /* MARS_GRAPHICS_HUDTERMINAL_H */
