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
 * \file Text.cpp
 * \author Malte Langosz (malte.langosz@dfki.de)
 * \brief
 *
 * Version 0.1
 */

#include "Text.h"

#include <osg/Geode>
#include <osg/LineWidth>
#include <osg/PositionAttitudeTransform>
#include <cstdio>

namespace osg_text {

  Text::Text(std::string text, double fontSize, Color textColor,
             double posX, double posY, TextAlign textAlign,
             double paddingL, double paddingT,
             double paddingR, double paddingB,
             Color backgroundColor, Color borderColor,
             double borderWidth,
             std::string fontPath) : posX(posX), posY(posY),
                                     textAlign(textAlign),
                                     pl(paddingL), pt(paddingT),
                                     pr(paddingR), pb(paddingB),
                                     borderWidth(borderWidth),
                                     fixedWidth(-1), fixedHeight(-1),
                                     fontSize(fontSize) {

    labelGeode = new osg::Geode();
    std::string font;
    if(fontPath.empty()) {
      font = MARS_PREFERENCES_DEFAULT_RESOURCES_PATH;
      font += "/mars/graphics/resources/Fonts";
      font += "/arial.ttf";
    }
    else {
      font = fontPath;
    }
    //fprintf(stderr, "font: %s\n", font.c_str());
    labelText = new osgText::Text;

    labelGeode->addDrawable(labelText.get());

    osg::StateSet* stateset = labelGeode->getOrCreateStateSet();
    stateset->setRenderBinDetails(22, "RenderBin");
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    stateset->setMode(GL_BLEND,osg::StateAttribute::OFF);
    //stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);

    labelText->setFont(font);
    labelText->setPosition(osg::Vec3(0.0f, 0.0f, 0.0f));
    //fprintf(stderr, "set: %g, %s\n", fontSize, text.c_str());
    labelText->setCharacterSize(fontSize);
    //labelText->setCharacterSizeMode(osgText::Text::SCREEN_COORDS);
    labelText->setAxisAlignment(osgText::Text::XY_PLANE);
    labelText->setAlignment(osgText::Text::LEFT_TOP);
    text_ = text;
    labelText->setText(text);
    labelText->setColor(osg::Vec4(textColor.r, textColor.g,
                                  textColor.b, textColor.a));


    resolutionCorrectionX = resolutionCorrectionY = 1.;
    resolutionCorrection = new osg::MatrixTransform();
    resolutionCorrection->setMatrix(osg::Matrix::scale(1., 1., 1.));
    resolutionCorrection->addChild(labelGeode.get());
    transform = new osg::PositionAttitudeTransform();
    transform->addChild(resolutionCorrection.get());

    updateBoundingBox();
    updatePosition();

    createBackground(backgroundColor);
    createBorder(borderColor);
  }

  Text::~Text(void) {
  }

  void Text::setText(const std::string &s) {
    text_ = s;
    labelText->setText(s);
    labelGeode->dirtyBound();
    labelGeode->computeBound();
    updateSize();
  }

  void Text::setFontSize(const double fontSize) {
    this->fontSize = fontSize;
    labelText->setCharacterSize(fontSize);
    updateSize();
  }

  void Text::setBackgroundColor(const Color &c) {
    (*backgroundColor)[0] = osg::Vec4(c.r, c.g, c.b, c.a);
    backgroundGeom->dirtyDisplayList();
  }

  void Text::setBorderColor(const Color &c) {
    (*borderColor)[0] = osg::Vec4(c.r, c.g, c.b, c.a);
    //borderGeom->dirty();
  }

  void Text::setPadding(double left, double top, double right, double bottom) {
    pl = left;
    pt = top;
    pr = right;
    pb = bottom;
    updateSize();
  }

  void* Text::getOSGNode() {
    return dynamic_cast<osg::Node*>(transform.get());
  }

  void Text::setFixedWidth(double w) {
    fixedWidth = w;
    updateSize();
  }

  void Text::setFixedHeight(double h) {
     fixedHeight = h;
     updateSize();
  }

  void Text::setPosition(double x, double y) {
    posX = x;
    posY = y;
    updatePosition();
    updateBackgroundPos();
    updateBorderPos();
  }

  // private methods

  void Text::updateSize() {
    updateBoundingBox();
    updatePosition();
    updateBackgroundPos();
    updateBorderPos();
  }

  void Text::updateBoundingBox() {
    osg::BoundingBox bb;
    bb.expandBy(labelText->getBound());
    //d = bb.zMin()-2.0;
    width = resolutionCorrectionX*(bb.xMax() - bb.xMin()) + pl + pr;
    height = resolutionCorrectionY*(bb.yMax() - bb.yMin()) + pt + pb;
  }

  void Text::updatePosition() {
    if(fixedWidth > 0.0) {
      w = fixedWidth;
    }
    else {
      w = width;
    }
    if(fixedHeight > 0.0) {
      h = fixedHeight;
    }
    else {
      h = height;
    }
    if(textAlign == ALIGN_LEFT) {
      posXB = posXI = 0.0;
    }
    else if (textAlign == ALIGN_CENTER) {
      posXI = - width*0.5;
      posXB = - w*0.5;
    }
    else {
      posXI = - width;
      posXB = - w;
    }
    resolutionCorrection->setMatrix(osg::Matrix::translate(posXI, 0.0, 0.0f));
    resolutionCorrection->preMult(osg::Matrix::scale(resolutionCorrectionX,
                                                     resolutionCorrectionY, 1.));
    //labelText->setPosition(osg::Vec3(posXI, 0.0, 0.0f));
    transform->setPosition(osg::Vec3(posX, posY, -1.501f));
  }

  void Text::createBackground(Color bgColor) {
    backgroundGeom = new osg::Geometry;
    backgroundVertices = new osg::Vec3Array;
    backgroundVertices->push_back(osg::Vec3(posXB-pl, -h+pt, 0.));
    backgroundVertices->push_back(osg::Vec3(posXB-pl, pt, 0.));
    backgroundVertices->push_back(osg::Vec3(posXB+w-pl, pt, 0.));
    backgroundVertices->push_back(osg::Vec3(posXB+w-pl, -h+pt, 0.));
    backgroundGeom->setVertexArray(backgroundVertices);

    osg::Vec3Array* normals = new osg::Vec3Array;
    normals->push_back(osg::Vec3(0.0f,0.0f,1.0f));
    backgroundGeom->setNormalArray(normals);
    backgroundGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);

    backgroundColor = new osg::Vec4Array;
    backgroundColor->push_back(osg::Vec4(bgColor.r, bgColor.g,
                                         bgColor.b, bgColor.a));
    backgroundGeom->setColorArray(backgroundColor);
    backgroundGeom->setColorBinding(osg::Geometry::BIND_OVERALL);

    backgroundGeom->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,4));

    osg::StateSet* stateset = backgroundGeom->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
    stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
    stateset->setRenderBinDetails(21, "RenderBin");

    backgroundGeode = new osg::Geode;
    backgroundGeode->addDrawable(backgroundGeom.get());
    if((*backgroundColor)[0][0] > 0.0001) {
      transform->addChild(backgroundGeode.get());
    }
  }

  void Text::updateBackgroundPos() {
    (*backgroundVertices)[0] = osg::Vec3(posXB-pl, -h+pt, 0.);
    (*backgroundVertices)[1] = osg::Vec3(posXB-pl, pt, 0.);
    (*backgroundVertices)[2] = osg::Vec3(posXB+w-pl, pt, 0.);
    (*backgroundVertices)[3] = osg::Vec3(posXB+w-pl, -h+pt, 0.);
    //backgroundVertices->dirty();
    //backgroundGeode->dirtyBound();
    backgroundGeom->dirtyDisplayList();
    //backgroundGeom->setVertexArray(backgroundVertices);
    //backgroundGeom->dirty();
  }

  void Text::createBorder(Color bColor) {
    borderGeom = new osg::Geometry;
    //nodemanager tempnode;

    borderVertices = new osg::Vec3Array(8);
    updateBorderPos();

    // pass the created vertex array to the points geometry object.
    borderGeom->setVertexArray(borderVertices);

    // set the colors as before, plus using the above
    borderColor = new osg::Vec4Array;
    borderColor->push_back(osg::Vec4(bColor.r, bColor.g, bColor.b, bColor.a));
    borderGeom->setColorArray(borderColor);
    borderGeom->setColorBinding(osg::Geometry::BIND_OVERALL);

    // set the normal in the same way color.
    osg::Vec3Array* normals = new osg::Vec3Array;
    normals->push_back(osg::Vec3(0.0f,0.0f,1.0f));
    borderGeom->setNormalArray(normals);
    borderGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);

    // since we know up front,
    borderGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,
                                                   0,8));

    osg::ref_ptr<osg::LineWidth> linew = new osg::LineWidth(borderWidth);
    borderGeom->getOrCreateStateSet()->setAttributeAndModes(linew.get(),
                                                           osg::StateAttribute::ON);
    borderGeom->getOrCreateStateSet()->setRenderBinDetails(20, "RenderBin");

    borderGeode = new osg::Geode;
    borderGeode->addDrawable(borderGeom.get());
    setBorderWidth(borderWidth);
  }

  void Text::setBorderWidth(double w) {
    borderWidth = w;
    if(transform->containsNode(borderGeode) && borderWidth < 0.00001) {
      transform->removeChild(borderGeode);
    }
    else if(!transform->containsNode(borderGeode)  && borderWidth > 0.00001) {
      transform->addChild(borderGeode);
    }
    osg::ref_ptr<osg::LineWidth> linew = new osg::LineWidth(borderWidth);
    borderGeom->getOrCreateStateSet()->setAttributeAndModes(linew.get(),
                                                            osg::StateAttribute::ON);
  }

  void Text::updateBorderPos() {
    (*borderVertices)[0].set(posXB-pl, -h+pt, 0.001);
    (*borderVertices)[1].set(posXB-pl, pt, 0.001);
    (*borderVertices)[2].set(posXB-pl, pt, 0.001);
    (*borderVertices)[3].set(posXB+w-pl, pt, 0.001);
    (*borderVertices)[4].set(posXB+w-pl, pt, 0.001);
    (*borderVertices)[5].set(posXB+w-pl, -h+pt, 0.001);
    (*borderVertices)[6].set(posXB+w-pl, -h+pt, 0.001);
    (*borderVertices)[7].set(posXB-pl, -h+pt, 0.001);
    //borderVertices->dirty();
    borderGeom->dirtyDisplayList();
    //borderGeom->setVertexArray(borderVertices);
    //borderGeom->dirty();
  }

  void Text::setFontResolution(int x, int y) {
    labelText->setFontResolution(round(x), round(y));
    //labelText->setCharacterSize(fontSize);
    osg::BoundingBox bb;
    bb.expandBy(labelText->getBound());
    //d = bb.zMin()-2.0;
    double w = bb.xMax() - bb.xMin();
    double h = bb.yMax() - bb.yMin();
    resolutionCorrectionX = (width - pl - pr)/w;
    resolutionCorrectionY = (height - pt - pb)/h;
    resolutionCorrection->setMatrix(osg::Matrix::translate(posXI, 0.0, 0.0f));
    resolutionCorrection->preMult(osg::Matrix::scale(resolutionCorrectionX,
                                                     resolutionCorrectionY, 1.));
  }

  void Text::getRectangle(double *left, double *right,
                          double *top, double *bottom) {
    *left = posX+posXB-pl;
    *right = *left+w;
    *top = posY+pt;
    *bottom = *top-h;
  }

  void Text::getPosition(double *x, double *y) {
    *x = posX;
    *y = posY;
  }

  std::string Text::getText() {
    return text_;
  }

  double Text::getFontsize() {
    return fontSize;
  }

  void Text::getPadding(double *pl, double *pt, double *pr, double *pb) {
    *pl = this->pl;
    *pt = this->pt;
    *pr = this->pr;
    *pb = this->pb;
  }

  TextAlign Text::getAlign() {
    return textAlign;
  }

  Color Text::getBackgroundColor() {
    Color c;
    c.r = (*backgroundColor)[0][0];
    c.g = (*backgroundColor)[0][1];
    c.b = (*backgroundColor)[0][2];
    c.a = (*backgroundColor)[0][3];
    return c;
  }

  Color Text::getBorderColor() {
    Color c;
    c.r = (*borderColor)[0][0];
    c.g = (*borderColor)[0][1];
    c.b = (*borderColor)[0][2];
    c.a = (*borderColor)[0][3];
    return c;
  }

  double Text::getBorderWidth() {
    return borderWidth;
  }

} // end of namespace: osg_text
