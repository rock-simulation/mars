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
 * \file LinesP.h
 * \author Malte Langosz
 * \brief 
 **/

#ifndef OSG_LINES_P_H
#define OSG_LINES_P_H

#ifdef _PRINT_HEADER_
#warning "LinesP.h"
#endif

#include "Lines.h"

#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osg/Geometry>
#include <osg/LineWidth>

namespace osg_lines {

  class LinesP : public osg::Group, public Lines {

  public:
    LinesP();
    ~LinesP();

    void appendData(Vector v);
    void clearData();
    void setData(std::list<Vector> points);
    void drawStrip(bool strip=true);
    void setColor(Color c);
    void setLineWidth(double w);
    void dirty(void);
    void* getOSGNode();
    void setBezierMode(bool bezier);
    void setBezierInterpolationPoints(int numPoints);

  private:
    bool strip, bezierMode;
    int bezierInterpolationPoints;
    osg::ref_ptr<osg::Vec3Array> points, origPoints;
    osg::ref_ptr<osg::Geometry> linesGeom;
    osg::ref_ptr<osg::MatrixTransform> linesTransform;
    osg::ref_ptr<osg::DrawArrays> drawArray;
    osg::ref_ptr<osg::LineWidth> linew;
    osg::ref_ptr<osg::Vec4Array> colors;
    osg::ref_ptr<osg::Geode> node;

    osg::Vec3 getBezierPoint(float t);
  };

} // end of namespace: osg_lines

#endif // OSG_LINES_P_H
