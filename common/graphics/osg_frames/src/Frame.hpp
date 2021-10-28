/*
 *  Copyright 2021, DFKI GmbH Robotics Innovation Center
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
 * \file Frame.hpp
 * \author Malte Langosz
 * \brief 
 **/

#ifndef OSG_FRAME_HPP
#define OSG_FRAME_HPP

#ifdef _PRINT_HEADER_
#warning "Frame.hpp"
#endif

#include <list>

namespace osg_frames {

  struct Vector {
    Vector(double x, double y, double z) : x(x), y(y), z(z) {}
    double x, y, z;
  };

  class Frame {

  public:
    Frame() {}
    virtual ~Frame() {}

    virtual void setPosition(double x, double y, double z) = 0;
    virtual void setRotation(double x, double y, double z, double w) = 0;
    virtual void setScale(double x) = 0;
    virtual void* getOSGNode() = 0;
  };

} // end of namespace: osg_frames

#endif // OSG_FRAME_HPP
