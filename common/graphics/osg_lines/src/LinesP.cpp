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
 * \file Lines.cpp
 * \author Malte (malte.langosz@dfki.de)
 * \brief A
 *
 * Version 0.1
 */

#include "LinesP.h"

#include <cstdio>

namespace osg_lines {
  
  LinesP::LinesP() {

    linesTransform = new osg::MatrixTransform;

    node = new osg::Geode;
    linesGeom = new osg::Geometry;

    points = new osg::Vec3Array();
    points->setDataVariance(osg::Object::DYNAMIC);
    linesGeom->setDataVariance(osg::Object::DYNAMIC);
    linesGeom->setVertexArray(points.get());

    colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4(1, 0, 0, 1.0));
    linesGeom->setColorArray(colors);
    linesGeom->setColorBinding(osg::Geometry::BIND_OVERALL);

    // set the normal in the same way color.
    osg::Vec3Array* normals = new osg::Vec3Array;
    normals->push_back(osg::Vec3(0.0f,0.0f,1.0f));
    linesGeom->setNormalArray(normals);
    linesGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);

    drawArray = new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP,
                                    0, points->size());
    drawArray->setFirst(0);
    drawArray->setCount(points->size());
        
    linesGeom->addPrimitiveSet(drawArray.get());
    node->addDrawable(linesGeom.get());

    linesTransform->addChild(node.get());

    this->addChild(linesTransform.get());

    linew = new osg::LineWidth(2.0);
    node->getOrCreateStateSet()->setAttributeAndModes(linew.get(),
                                                      osg::StateAttribute::ON);
	
    //node->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
    node->getOrCreateStateSet()->setRenderBinDetails(10, "RenderBin");	
    node->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    node->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::OFF);
  }

  LinesP::~LinesP(void) {
  }

  void LinesP::appendData(Vector v) {
    points->push_back(osg::Vec3(v.x, v.y, v.z));
    drawArray->setCount(points->size());
    dirty();
  }

  void LinesP::setData(std::list<Vector> p) {
    std::list<Vector>::iterator it=p.begin();
    points->clear();
    for(;it!=p.end(); ++it) {
      points->push_back(osg::Vec3(it->x, it->y, it->z));
    }
    drawArray->setCount(points->size());
    dirty();
  }

  void LinesP::drawStrip(bool strip) {
    linesGeom->removePrimitiveSet(0);
    if(strip) {
      drawArray = new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP,
                                      0, points->size());
      drawArray->setFirst(0);
      drawArray->setCount(points->size());
      linesGeom->addPrimitiveSet(drawArray.get());
    }
    else {
      drawArray = new osg::DrawArrays(osg::PrimitiveSet::LINES,
                                      0, points->size());
      drawArray->setFirst(0);
      drawArray->setCount(points->size());
      linesGeom->addPrimitiveSet(drawArray.get());
    }
  }

  void LinesP::setColor(Color c) {
    colors->clear();
    colors->push_back(osg::Vec4(c.r, c.g, c.b, c.a));
    dirty();
  }

  void LinesP::setLineWidth(double w) {
    linew->setWidth(w);
    dirty();
  }

  void LinesP::dirty(void) {
    linesGeom->dirtyDisplayList();
    linesGeom->dirtyBound();   
  }

  void* LinesP::getOSGNode() {
    return (void*)(osg::Node*)node.get();
  }

} // end of namespace: osg_lines
