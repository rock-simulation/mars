/*
 *  Copyright 2013, DFKI GmbH Robotics Innovation Center
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
 * \file Curve.h
 * \author Malte Langosz
 * \brief 
 **/

#ifndef OSG_CURVE_H
#define OSG_CURVE_H

#ifdef _PRINT_HEADER_
#warning "Curve.h"
#endif

#include <string>

namespace osg_plot {

  class Curve {

  public:
    Curve() {}
    virtual ~Curve() {}

    virtual void setMaxNumPoints(unsigned long n) = 0;
    virtual void setTitle(std::string s) = 0;
    virtual void appendData(double x, double y) = 0;
  };

} // end of namespace: osg_plot

#endif // OSG_CURVE_H
