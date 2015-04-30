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
 * \file TextInterface.h
 * \author Malte Langosz
 * \brief This library wraps the osgText lib and adds the possiblity to
 *        use an background color and border for text labels.
 **/

#ifndef OSG_TEXT_INTERFACE_H
#define OSG_TEXT_INTERFACE_H

#ifdef _PRINT_HEADER_
#warning "Text.h"
#endif

#include <string>

namespace osg_text {

  class Color {
  public:
    Color() : r(1.0), g(1.0), b(1.0), a(1.0) {
    }
    Color(double r, double g, double b, double a) :
      r(r), g(g), b(b), a(a) {
    }
    double r, g, b, a;
  };

  enum TextAlign {
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_RIGHT
  };

  class TextInterface {

  public:
    TextInterface() {}

    virtual ~TextInterface() {}
    virtual void setText(const std::string &s) = 0;
    virtual void setBackgroundColor(const Color &c) = 0;
    virtual void setBorderColor(const Color &c) = 0;
    virtual void setBorderWidth(double w) = 0;
    virtual void setPadding(double left, double top, double right,
                            double bottom) = 0;
    virtual void* getOSGNode() = 0;
    virtual void setFixedWidth(double w) = 0;
    virtual void setFixedHeight(double h) = 0;
    virtual void setPosition(double x, double y) = 0;
    virtual void getRectangle(double *left, double *right,
                              double *top, double *bottom) = 0;
  };

} // end of namespace: osg_text

#endif // OSG_TEXT_INTERFACE_H
