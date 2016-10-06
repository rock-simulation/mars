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
 * \file Points.cpp
 * \author Malte (malte.langosz@dfki.de)
 * \brief A
 *
 * Version 0.1
 */

#include "PointsP.hpp"

#include <cstdio>

namespace osg_points {
  
  PointsP::PointsP() {

    pointsTransform = new osg::MatrixTransform;

    node = new osg::Geode;
    pointsGeom = new osg::Geometry;

    points = new osg::Vec3Array();
    points->setDataVariance(osg::Object::DYNAMIC);
    pointsGeom->setDataVariance(osg::Object::DYNAMIC);
    pointsGeom->setVertexArray(points.get());

    colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4(1, 0, 0, 1.0));
    pointsGeom->setColorArray(colors);
    pointsGeom->setColorBinding(osg::Geometry::BIND_OVERALL);

    // set the normal in the same way color.
    osg::Vec3Array* normals = new osg::Vec3Array;
    normals->push_back(osg::Vec3(0.0f,0.0f,1.0f));
    pointsGeom->setNormalArray(normals);
    pointsGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);

    drawArray = new osg::DrawArrays(GL_POINTS, 0, points->size());
    drawArray->setFirst(0);
    drawArray->setCount(points->size());
        
    pointsGeom->addPrimitiveSet(drawArray.get());
    node->addDrawable(pointsGeom.get());

    pointsTransform->addChild(node.get());

    this->addChild(pointsTransform.get());

    linew = new osg::Point(2.0);
    node->getOrCreateStateSet()->setAttributeAndModes(linew.get(),
                                                      osg::StateAttribute::ON);
	
    //node->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
    node->getOrCreateStateSet()->setRenderBinDetails(10, "RenderBin");	
    node->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    node->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::OFF);
  }

  PointsP::~PointsP(void) {
  }

  void PointsP::appendData(Vector v) {
    points->push_back(osg::Vec3(v.x, v.y, v.z));
    drawArray->setCount(points->size());
    dirty();
  }

  void PointsP::setData(const std::vector<Vector> &p) {
    std::vector<Vector>::const_iterator it=p.begin();
    points->clear();
    for(;it!=p.end(); ++it) {
      points->push_back(osg::Vec3(it->x, it->y, it->z));
    }
    drawArray->setCount(points->size());
    dirty();
  }

  void PointsP::setColor(Color c) {
    colors->clear();
    colors->push_back(osg::Vec4(c.r, c.g, c.b, c.a));
    dirty();
  }

  void PointsP::setLineWidth(double w) {
    linew->setSize(w);
    dirty();
  }

  void PointsP::dirty(void) {
    pointsGeom->dirtyDisplayList();
    pointsGeom->dirtyBound();   
  }

  void* PointsP::getOSGNode() {
    return (void*)(osg::Node*)node.get();
  }

} // end of namespace: osg_points
