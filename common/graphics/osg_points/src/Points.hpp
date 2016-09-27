/*
 *  Copyright 2016, DFKI GmbH Robotics Innovation Center
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
 * \file Points.hpp
 * \author Malte Langosz
 * \brief 
 **/

#ifndef OSG_POINTS_H
#define OSG_POINTS_H

#include <vector>

namespace osg_points {

  struct Vector {
    Vector(double x, double y, double z) : x(x), y(y), z(z) {}
    double x, y, z;
  };

  struct Color {
    Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
    float r, g, b, a;
  };

  class Points {

  public:
    Points() {}
    virtual ~Points() {}

    virtual void appendData(Vector v) = 0;
    virtual void setData(std::vector<Vector> points) = 0;
    virtual void setColor(Color c) = 0;
    virtual void setLineWidth(double w) = 0;
    virtual void* getOSGNode() = 0;
  };

} // end of namespace: osg_points

#endif // OSG_POINTS_H
