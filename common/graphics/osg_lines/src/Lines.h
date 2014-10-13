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
 * \file Lines.h
 * \author Malte Langosz
 * \brief 
 **/

#ifndef OSG_LINES_H
#define OSG_LINES_H

#ifdef _PRINT_HEADER_
#warning "Lines.h"
#endif

#include <list>

namespace osg_lines {

  struct Vector {
    Vector(double x, double y, double z) : x(x), y(y), z(z) {}
    double x, y, z;
  };

  struct Color {
    Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
    float r, g, b, a;
  };

  class Lines {

  public:
    Lines() {}
    virtual ~Lines() {}

    virtual void appendData(Vector v) = 0;
    virtual void setData(std::list<Vector> points) = 0;
    virtual void drawStrip(bool strip=true) = 0;
    virtual void setColor(Color c) = 0;
    virtual void setLineWidth(double w) = 0;
    virtual void* getOSGNode() = 0;
  };

} // end of namespace: osg_lines

#endif // OSG_LINES_H
