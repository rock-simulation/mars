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
 * \file Plot.cpp
 * \author Malte (malte.langosz@dfki.de)
 * \brief A
 *
 * Version 0.1
 */

#include "Plot.h"
#include "CurveP.h"

#include <osg/Geode>
#include <osg/LineWidth>

namespace osg_plot {
  
  Plot::Plot() : numXTicks(8), numYTicks(10) {

    osg::ref_ptr<osg::Geode> xNode = new osg::Geode;
    xGeom = new osg::Geometry;
    bGeom = new osg::Geometry;

    // set the normal in the same way color.
    osg::Vec3Array* normals = new osg::Vec3Array;
    normals->push_back(osg::Vec3(0.0f,0.0f,1.0f));

    xLines = new osg::Vec3Array();
    xLines->setDataVariance(osg::Object::DYNAMIC);
    xLines->push_back(osg::Vec3(0.0, 0.0, 0.0));
    xLines->push_back(osg::Vec3(1.0, 0.0, 0.0));
    xLines->push_back(osg::Vec3(0.0, 0.0, 0.0));
    xLines->push_back(osg::Vec3(0.0, 1.0, 0.0));

    background = new osg::Vec3Array();
    background->push_back(osg::Vec3(-0.2, -0.1, -0.02));
    background->push_back(osg::Vec3(1.35, -0.1, -0.02));
    background->push_back(osg::Vec3(1.35, 1.1, -0.02));
    background->push_back(osg::Vec3(-0.2, 1.1, -0.02));
    bGeom->setVertexArray(background.get());
    bGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));

    osg::Vec4Array* bColors = new osg::Vec4Array;
    bColors->push_back(osg::Vec4(0.2, 0.2, 0.2, 1.0));
    bGeom->setColorArray(bColors);
    bGeom->setColorBinding(osg::Geometry::BIND_OVERALL);

    bGeom->setNormalArray(normals);
    bGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);
    
    xTicksDiff = 1.0 / (numXTicks-1);
    char text[52];
    for(int i=0; i<numXTicks; ++i) {
      xLines->push_back(osg::Vec3(xTicksDiff*i, 0.0, 0.0));
      xLines->push_back(osg::Vec3(xTicksDiff*i, -0.025, 0.0));
      sprintf(text, "%5.3f", xTicksDiff*i);
      osgText::Text *label = new osgText::Text;
      label->setText(text);
      label->setFont("fonts/arial.ttf");
      label->setPosition(osg::Vec3(xTicksDiff*i, -0.025f, 0.0f));
      label->setCharacterSize(0.04);
      label->setAxisAlignment(osgText::Text::XY_PLANE);
      label->setAlignment(osgText::Text::CENTER_TOP);
      label->setColor(osg::Vec4(0.0, 1.0, 0.0, 1.0));
      xNode->addDrawable(label);
      xLabels.push_back(label);
    }

    yTicksDiff = 1.0 / (numYTicks-1);
    for(int i=0; i<numYTicks; ++i) {
      xLines->push_back(osg::Vec3(0.0, yTicksDiff*i, 0.0f));
      xLines->push_back(osg::Vec3(-0.025, yTicksDiff*i, 0.0f));
      sprintf(text, "%5.3f", yTicksDiff*i);
      osgText::Text *label = new osgText::Text;
      label->setText(text);
      label->setFont("fonts/arial.ttf");
      label->setPosition(osg::Vec3(-0.025f, yTicksDiff*i, 0.0f));
      label->setCharacterSize(0.04);
      label->setAxisAlignment(osgText::Text::XY_PLANE);
      label->setAlignment(osgText::Text::RIGHT_CENTER);
      label->setColor(osg::Vec4(0.0, 1.0, 0.0, 1.0));
      xNode->addDrawable(label);
      yLabels.push_back(label);
    }

    xGeom->setDataVariance(osg::Object::DYNAMIC);
    xGeom->setVertexArray(xLines.get());

    osg::Vec4Array* xColors = new osg::Vec4Array;
    xColors->push_back(osg::Vec4(0.0, 1.0, 0.0, 1.0));
    xGeom->setColorArray(xColors);
    xGeom->setColorBinding(osg::Geometry::BIND_OVERALL);

    // set the normal in the same way color.
    xGeom->setNormalArray(normals);
    xGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);

    xDrawArray = new osg::DrawArrays(osg::PrimitiveSet::LINES,
                                     0, 1);
    xDrawArray->setFirst(0);
    xDrawArray->setCount(xLines->size());
    xGeom->addPrimitiveSet(xDrawArray.get());
    xNode->addDrawable(xGeom.get());
    xNode->addDrawable(bGeom.get());

    this->addChild(xNode.get());

    osg::ref_ptr<osg::LineWidth> linew;
    linew = new osg::LineWidth(2.0);
    xNode->getOrCreateStateSet()->setAttributeAndModes(linew.get(),
                                                       osg::StateAttribute::ON);
	
    //xNode->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
    xNode->getOrCreateStateSet()->setRenderBinDetails(10, "RenderBin");	
    xNode->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    xNode->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::OFF);
    
  }

  Plot::~Plot(void) {
    curves.clear();
    xLabels.clear();
    yLabels.clear();
  }

  Curve* Plot::createCurve(void) {
    CurveP *curve = new CurveP(curves.size()%3);
    curve->yPos = 1.0-curves.size() * 0.1;
    this->addChild(curve);
    curves.push_back(curve);
    return curve;
  }

  void Plot::update(void) {
    std::list< osg::ref_ptr<CurveP> >::iterator it = curves.begin();
    std::list< osg::ref_ptr<osgText::Text> >::iterator itT = xLabels.begin();

    double minX = DBL_MAX;
    double maxX = DBL_MIN;
    double minY = DBL_MAX;
    double maxY = DBL_MIN;

    for(; it!=curves.end(); ++it) {
      (*it)->crop();
      (*it)->getBounds(&minX, &maxX, &minY, &maxY);
    }

    for(it=curves.begin(); it!=curves.end(); ++it) {
      (*it)->rescale(minX, maxX, minY, maxY);
      (*it)->dirty();
    }

    int i = 0;
    char text[52];
    for(; itT!=xLabels.end(); ++itT, ++i) {
      sprintf(text, "%5.3f", minX + xTicksDiff*i*(maxX-minX));
      (*itT)->setText(text);
    }

    i = 0;
    for(itT=yLabels.begin(); itT!=yLabels.end(); ++itT, ++i) {
      sprintf(text, "%5.3f", minY + yTicksDiff*i*(maxY-minY));
      (*itT)->setText(text);
    }
    
  }

} // end of namespace: osg_plot
