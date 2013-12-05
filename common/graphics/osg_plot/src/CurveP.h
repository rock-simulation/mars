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
 * \file CurveP.h
 * \author Malte Langosz
 * \brief 
 **/

#ifndef OSG_CURVE_P_H
#define OSG_CURVE_P_H

#ifdef _PRINT_HEADER_
#warning "CurveP.h"
#endif

#include "Curve.h"

#include <osg/MatrixTransform>
#include <osg/Geometry>
#include <osgText/Text>

namespace osg_plot {

  struct Color {
    float r, g, b, a;
  };

  class CurveP : public osg::Group, public Curve {
    friend class Plot;

  public:
    CurveP(int c);
    ~CurveP();

    void setMaxNumPoints(unsigned long n) {maxPoints = n;}
    void setTitle(std::string s) {title = s.c_str();}

    void appendData(double x, double y);
    void crop(void);
    void getBounds(double *minX, double *maxX, double *minY, double *maxY);
    void rescale(double minX, double maxX, double minY, double maxY);
    void dirty(void);

  private:
    unsigned long maxPoints;
    int color;
    float yPos;
    std::string title;

    Color defColors[3];
    osg::ref_ptr<osg::Vec3Array> points;
    osg::ref_ptr<osg::Geometry> linesGeom;
    osg::ref_ptr<osg::MatrixTransform> curveTransform;
    osg::ref_ptr<osg::DrawArrays> drawArray;

    osg::ref_ptr<osgText::Text> xLabelText;
  };

} // end of namespace: osg_plot

#endif // OSG_CURVE_P_H
