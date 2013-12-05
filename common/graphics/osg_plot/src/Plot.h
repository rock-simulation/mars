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
 * \file Plot.h
 * \author Malte Langosz
 * \brief 
 **/

#ifndef OSG_PLOT_H
#define OSG_PLOT_H

#ifdef _PRINT_HEADER_
#warning "Plot.h"
#endif

#include "Curve.h"

#include <osg/MatrixTransform>
#include <osg/Geometry>
#include <osgText/Text>

#include <list>

namespace osg_plot {

  class CurveP;

  class Plot : public osg::Group {

  public:
    Plot();
    ~Plot();

    Curve* createCurve(void);
    void removeCurve(Curve*);

    void update(void);

  private:
    int numXTicks, numYTicks;
    float xTicksDiff, yTicksDiff;

    osg::ref_ptr<osg::Vec3Array> xLines;
    osg::ref_ptr<osg::Vec3Array> background;
    osg::ref_ptr<osg::Geometry> bGeom;
    osg::ref_ptr<osg::Geometry> xGeom;
    osg::ref_ptr<osg::DrawArrays> xDrawArray;
    std::list< osg::ref_ptr<CurveP> > curves;

    std::list< osg::ref_ptr<osgText::Text> > xLabels;
    std::list< osg::ref_ptr<osgText::Text> > yLabels;

  };

} // end of namespace: osg_plot

#endif // OSG_PLOT_H
