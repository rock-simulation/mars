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
 * \file Curve.cpp
 * \author Malte (malte.langosz@dfki.de)
 * \brief A
 *
 * Version 0.1
 */

#include "CurveP.h"

#include <osg/Geode>
#include <osg/LineWidth>

namespace osg_plot {
  
  CurveP::CurveP(int c) : maxPoints(500), color(c), yPos(0.0) {

    defColors[0] = (Color){1.0, 0.0, 0.0, 1.0};
    defColors[1] = (Color){1.0, 1.0, 0.0, 1.0};
    defColors[2] = (Color){0.0, 1.0, 1.0, 1.0};

    curveTransform = new osg::MatrixTransform;

    osg::ref_ptr<osg::Geode> node = new osg::Geode;
    osg::ref_ptr<osg::Geode> textNode = new osg::Geode;
    linesGeom = new osg::Geometry;

    points = new osg::Vec3Array();
    points->setDataVariance(osg::Object::DYNAMIC);
    points->push_back(osg::Vec3(0.0, 0.00, 0.0));
    points->push_back(osg::Vec3(0.00001, 0.0, 0.0));
    linesGeom->setDataVariance(osg::Object::DYNAMIC);
    linesGeom->setVertexArray(points.get());

    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4(defColors[color].r, defColors[color].g,
                                defColors[color].b, defColors[color].a));
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

    xLabelText = new  osgText::Text;
    xLabelText->setText("1.0");
    xLabelText->setFont("fonts/arial.ttf");
    xLabelText->setPosition(osg::Vec3(1.0f, 0.0f, 0.0f));
    xLabelText->setCharacterSize(0.04);
    xLabelText->setAxisAlignment(osgText::Text::XY_PLANE);
    xLabelText->setAlignment(osgText::Text::LEFT_TOP);
    xLabelText->setColor(osg::Vec4(defColors[color].r, defColors[color].g,
                                   defColors[color].b, defColors[color].a));
    textNode->addDrawable(xLabelText.get());

    curveTransform->addChild(node.get());

    this->addChild(curveTransform.get());
    this->addChild(textNode.get());

    osg::ref_ptr<osg::LineWidth> linew;
    linew = new osg::LineWidth(2.0);
    node->getOrCreateStateSet()->setAttributeAndModes(linew.get(),
                                                      osg::StateAttribute::ON);
	
    //node->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
    node->getOrCreateStateSet()->setRenderBinDetails(10, "RenderBin");	
    node->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    node->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::OFF);

    textNode->getOrCreateStateSet()->setAttributeAndModes(linew.get(),
                                                       osg::StateAttribute::ON);
	
    //textNode->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
    textNode->getOrCreateStateSet()->setRenderBinDetails(10, "RenderBin");	
    textNode->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    textNode->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::OFF);
    
  }

  CurveP::~CurveP(void) {
  }

  void CurveP::appendData(double x, double y) {
    points->push_back(osg::Vec3(x, y, 0.0));
  }

  void CurveP::crop(void) {
    long count;
    if( (count = (long)points->size() - (long)maxPoints) > 0) {
      points->erase(points->begin(), points->begin()+count);
    }
    drawArray->setFirst(0);
    drawArray->setCount(points->size());
  }

  void CurveP::getBounds(double *minX, double *maxX,
                         double *minY, double *maxY) {
    osg::Vec3Array::iterator it = points->begin();
    for(; it!=points->end(); ++it) {
      if(it->x() < *minX) {
        *minX = it->x();
      }
      if(it->x() > *maxX) {
        *maxX = it->x();
      }
      if(it->y() < *minY) {
        *minY = it->y();
      }
      if(it->y() > *maxY) {
        *maxY = it->y();
      }
    }
  }

  void CurveP::rescale(double minX, double maxX,
                       double minY, double maxY) {
    char xLabel[56];
    sprintf(xLabel, "%10s: %6.3f", title.c_str(), points->back().y());
    xLabelText->setText(xLabel);
    //xLabelText->setPosition(osg::Vec3((points->back().x()-minX)/(maxX-minX), 0.0f,
    //                                  (points->back().z()-minY)/(maxY-minY)));

    xLabelText->setPosition(osg::Vec3(1.0f, yPos, 0.0));
    curveTransform->setMatrix(osg::Matrix::translate(-minX, -minY, 0)*
                              osg::Matrix::scale(1/(maxX-minX),
                                                 1/(maxY-minY), 1.0));
  }

  void CurveP::dirty(void) {
    linesGeom->dirtyDisplayList();
    linesGeom->dirtyBound();   
  }

} // end of namespace: osg_plot
