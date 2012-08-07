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

/*
 * AxisPrimitive.cpp
 *
 *  Created on: 19.04.2011
 *      Author: daniel
 */

#include "AxisPrimitive.h"

#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Vec3>
#include <osg/PositionAttitudeTransform>

namespace mars {
  namespace graphics {

    using mars::utils::Vector;

    AxisPrimitive::AxisPrimitive(const Vector &first, const Vector &second,
                                 const Vector &third,
                                 const Vector &axis1, const Vector &axis2) {
      // create objects
      osg::ref_ptr<osg::Geode> geodeConnector1 = new osg::Geode;
      osg::ref_ptr<osg::Geode> geodeConnector2 = new osg::Geode;
      osg::ref_ptr<osg::Geode> geodeAxis1 = new osg::Geode;
      osg::ref_ptr<osg::Geode> geodeAxis2 = new osg::Geode;
      osg::ref_ptr<osg::ShapeDrawable> shapeConnector1;
      osg::ref_ptr<osg::ShapeDrawable> shapeConnector2;
      osg::ref_ptr<osg::ShapeDrawable> shapeAxis1;
      osg::ref_ptr<osg::ShapeDrawable> shapeAxis2;
      osg::ref_ptr<osg::TessellationHints> hints = new osg::TessellationHints;
      hints->setDetailRatio(1.0f);

      //load vectors
      osg::Vec3 v_first = osg::Vec3(first.x(),first.y(),first.z());
      osg::Vec3 v_second = osg::Vec3(second.x(),second.y(),second.z());
      osg::Vec3 v_third = osg::Vec3(third.x(),third.y(),third.z());
      osg::Vec3 v_axis1 = osg::Vec3(axis1.x(),axis1.y(),axis1.z());
      osg::Vec3 v_axis2 = osg::Vec3(axis2.x(),axis2.y(),axis2.z());
      osg::Vec3 calcTemp1 = v_second - v_first;
      osg::Vec3 calcTemp2 = v_third - v_second;

      // create lines as boxes and cylinders
      shapeConnector1 = new osg::ShapeDrawable(new osg::Box(
                                                            osg::Vec3(0,0,0), 0.02,0.02, calcTemp1.length()), hints.get());
      shapeConnector2 = new osg::ShapeDrawable(new osg::Box(
                                                            osg::Vec3(0,0,0), 0.02,0.02, calcTemp2.length()), hints.get());
      shapeAxis1 = new osg::ShapeDrawable(new osg::Cylinder(
                                                            osg::Vec3(0,0,0), 0.03, 0.4), hints.get());
      shapeAxis2 = new osg::ShapeDrawable(new osg::Cylinder(
                                                            osg::Vec3(0,0,0), 0.03, 0.4), hints.get());

      // set colors
      shapeConnector1->setColor(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
      shapeConnector2->setColor(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
      shapeAxis1->setColor(osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f));
      shapeAxis2->setColor(osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f));
      geodeConnector1->addDrawable(shapeConnector1.get());
      geodeConnector2->addDrawable(shapeConnector2.get());
      geodeAxis1->addDrawable(shapeAxis1.get());
      geodeAxis2->addDrawable(shapeAxis2.get());

      //position line segments
      osg::ref_ptr<osg::PositionAttitudeTransform> transformConnector1 = new osg::PositionAttitudeTransform();
      osg::Quat oquatConnector1;
      oquatConnector1.makeRotate(osg::Vec3(0,0,1), calcTemp1);
      transformConnector1->setAttitude(oquatConnector1);
      transformConnector1->setPosition(v_first+(calcTemp1/2));
      transformConnector1->addChild(geodeConnector1.get());

      osg::ref_ptr<osg::PositionAttitudeTransform> transformConnector2 = new osg::PositionAttitudeTransform();
      osg::Quat oquatConnector2;
      oquatConnector2.makeRotate(osg::Vec3(0,0,1), calcTemp2);
      transformConnector2->setAttitude(oquatConnector2);
      transformConnector2->setPosition(v_second+(calcTemp2/2));
      transformConnector2->addChild(geodeConnector2.get());

      osg::ref_ptr<osg::PositionAttitudeTransform> transformAxis1 = new osg::PositionAttitudeTransform();
      osg::Quat oquatAxis1;
      oquatAxis1.makeRotate(osg::Vec3(0,0,1), v_axis1);
      transformAxis1->setAttitude(oquatAxis1);
      transformAxis1->setPosition(v_second);
      transformAxis1->addChild(geodeAxis1.get());

      osg::ref_ptr<osg::PositionAttitudeTransform> transformAxis2 = new osg::PositionAttitudeTransform();
      osg::Quat oquatAxis2;
      oquatAxis2.makeRotate(osg::Vec3(0,0,1), v_axis2);
      transformAxis2->setAttitude(oquatAxis2);
      transformAxis2->setPosition(v_second);
      transformAxis2->addChild(geodeAxis2.get());

      addChild(transformConnector1.get());
      addChild(transformConnector2.get());
      addChild(transformAxis1.get());
      addChild(transformAxis2.get());
    }

  } // end of namespace graphics 
} // end of namespace mars
