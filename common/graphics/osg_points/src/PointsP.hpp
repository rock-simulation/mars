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
 * \file PointsP.hpp
 * \author Malte Langosz
 * \brief 
 **/

#ifndef OSG_POINTS_P_H
#define OSG_POINTS_P_H

#include "Points.hpp"

#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osg/Geometry>
#include <osg/LineWidth>

namespace osg_points {

  class PointsP : public osg::Group, public Points {

  public:
    PointsP();
    ~PointsP();

    void appendData(Vector v);
    void setData(std::vector<Vector> points);
    void setColor(Color c);
    void setLineWidth(double w);
    void dirty(void);
    void* getOSGNode();

  private:
    osg::ref_ptr<osg::Vec3Array> points;
    osg::ref_ptr<osg::Geometry> pointsGeom;
    osg::ref_ptr<osg::MatrixTransform> pointsTransform;
    osg::ref_ptr<osg::DrawArrays> drawArray;
    osg::ref_ptr<osg::LineWidth> linew;
    osg::ref_ptr<osg::Vec4Array> colors;
    osg::ref_ptr<osg::Geode> node;
  };

} // end of namespace: osg_points

#endif // OSG_POINTS_P_H
