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
 * OSGDrawItem.cpp
 *
 *  Created on: 19.04.2011
 *      Author: daniel
 */

#include "OSGDrawItem.h"
#include "gui_helper_functions.h"

#include <cstring>
#include <cstdio>

#include <osg/Point>
#include <osg/LineWidth>
#include <osg/Texture2D>
#include <osg/Image>
#include <osg/AutoTransform>
#include <osg/ShapeDrawable>
#include <osgText/Text>
#include <osg/CullFace>


namespace mars {
  namespace graphics {

    using namespace std;
    using namespace mars::interfaces;
    using mars::utils::Vector;
    using mars::utils::Quaternion;
    using mars::utils::Color;

    OSGDrawItem::OSGDrawItem(GraphicsWidget *gw)
      : osg::Group(), gw_(gw), texture_(NULL) {

    }

    OSGDrawItem::OSGDrawItem(GraphicsWidget *gw, const draw_item &di,
                             std::string fontPath)
      : osg::Group(), type_(di.type), gw_(gw), texture_(NULL) {

      osg::StateSet *states = getOrCreateStateSet();

      // create geometry and set related states
      switch (type_) {
      case DRAW_LINE: {
        osg::ref_ptr<osg::LineWidth> linew = new osg::LineWidth(di.point_size);

        createLine(this, di.start, di.end, di.myColor);
        states->setAttributeAndModes(linew.get(), osg::StateAttribute::ON);
        break;
      }
      case DRAW_LINES: {
        osg::ref_ptr<osg::LineWidth> linew = new osg::LineWidth(di.point_size);

        createLines(this, di.vertices, di.myColor);
        states->setAttributeAndModes(linew.get(), osg::StateAttribute::ON);
        break;
      }
      case DRAW_TEXT:
        createLabel(this, di.pos, di.point_size, di.label, fontPath,
                    di.myColor.r, di.myColor.g,
                    di.myColor.b, di.myColor.a,
                    di.align_to_view);
        break;
      case DRAW_HUD_RECT:
        createHUDRectangle(this,
                           di.width, di.height,
                           di.offset, di.myColor);
        states->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        states->setMode(GL_BLEND,osg::StateAttribute::ON);
        break;
      case DRAW_TRIANGLE_SET: {
        createTriangleSet(this, di.vertices, di.myColor);
        osg::ref_ptr<osg::CullFace> cull = new osg::CullFace();
        cull->setMode(osg::CullFace::BACK);
        states->setAttributeAndModes(cull.get(), osg::StateAttribute::OFF); }
        break;
      case DRAW_POINTS: {
        osg::ref_ptr<osg::Point> point = new osg::Point();

        createPoints(this, di.vertices, di.myColor);
        point->setSize(di.point_size);
        states->setAttribute(point.get());
        break;
      }
      case DRAW_ARROW:
        createLine(this, di.start, di.end, di.myColor);
        break;
      case DRAW_RECTANGLE: {
        osg::ref_ptr<osg::LineWidth> linew = new osg::LineWidth(di.point_size);

        createRectangle(this,
                        di.start, di.end, di.pos,
                        di.myColor, di.borderColor,
                        di.align_to_view);
        states->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        states->setMode(GL_BLEND,osg::StateAttribute::ON);
        states->setAttributeAndModes(linew.get(), osg::StateAttribute::ON);
        osg::ref_ptr<osg::CullFace> cull = new osg::CullFace();
        cull->setMode(osg::CullFace::BACK);
        states->setAttributeAndModes(cull.get(), osg::StateAttribute::OFF);
        break;
      }
      case DRAW_CIRCLE: {
        osg::ref_ptr<osg::LineWidth> linew = new osg::LineWidth(di.point_size);

        createCircle(this,
                     di.pos, di.start,
                     di.axis, di.myColor,
                     di.borderColor, di.resolution);
        states->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        states->setMode(GL_BLEND,osg::StateAttribute::ON);
        states->setAttributeAndModes(linew.get(), osg::StateAttribute::ON);
        break;
      }
      case DRAW_POINT:
      case DRAW_LINE_STRIP:
        // FIXME: howto create....
        break;
      case DRAW_UNKNOWN:
      case DRAW_LAST:
        break;
      }

      // turn off lighting if requested
      if(di.get_light == 0) {
        states->setMode(GL_LIGHTING,
                        osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
        states->setMode(GL_FOG, osg::StateAttribute::OFF);
      }

      // create texture for this draw item
      if (di.texture != "") {
        texture_ = GuiHelper::loadTexture(di.texture).get();
        states->setTextureAttributeAndModes(0, texture_,
                                            osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
      } else if (di.t_height > 0 && di.t_width > 0) {
        osg::Image* image = new osg::Image();
        image->setImage(di.t_height, di.t_height, 1, GL_RGBA,
                        GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
                        (unsigned char*)di.t_data, osg::Image::NO_DELETE);
        texture_ = new osg::Texture2D();
        texture_->setDataVariance(osg::Object::DYNAMIC);
        texture_->setImage(image);
        states->setTextureAttributeAndModes(0, texture_,
                                            osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
      }
    }

    void OSGDrawItem::update(draw_item &di) {
      osg::StateSet *states = getOrCreateStateSet();

      switch (type_) {
      case DRAW_LINE: {
        removeChildren(0, getNumChildren());
        createLine(this, di.start, di.end, di.myColor);
        break;
      }
      case DRAW_TEXT:
        updateLabel(this,
                    di.pos, di.point_size, di.label,
                    di.myColor.r, di.myColor.g,
                    di.myColor.b, di.myColor.a,
                    di.align_to_view);
        break;
      case DRAW_HUD_RECT:
        updateHUDRectangle(this,
                           di.width, di.height,
                           di.offset, di.myColor);
        if (di.t_width > 1 && di.t_height > 0) {
          di.t_width = 1;
          memcpy(texture_->getImage()->data(), di.t_data,
                 di.t_height*di.t_height*4);
          texture_->getImage()->dirty();
        } else if (di.t_width == 0 && di.t_height == 0) {
          states->setTextureAttributeAndModes(0,
                                              GuiHelper::loadTexture(di.texture).get(),
                                              osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
        }
        break;
      case DRAW_TRIANGLE_SET:
        addToTriangleSet(this, di.vertices, di.myColor);
        break;
      case DRAW_POINTS:
        updatePoints(this, di.vertices);
        break;
      case DRAW_POINT:
      case DRAW_ARROW:
      case DRAW_RECTANGLE:
      case DRAW_CIRCLE:
      case DRAW_LINE_STRIP:
        // TODO: howto update....
        break;
      case DRAW_UNKNOWN:
      case DRAW_LINES:
      case DRAW_LAST:
        break;
      }
    }

    ///////

    void OSGDrawItem::createSphere(OSGDrawItem *osgNode, double rad) {
      osg::ref_ptr<osg::Geode> geode = new osg::Geode;
      osg::ref_ptr<osg::ShapeDrawable> shape;
      osg::ref_ptr<osg::TessellationHints> hints = new osg::TessellationHints;
      osg::ref_ptr<osg::PositionAttitudeTransform> transform =
        new osg::PositionAttitudeTransform();

      hints->setDetailRatio(1.0f);

      // TODO: do not use ShapeDrawable !
      shape = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0.0,0.0,0.0), rad),hints.get());

      geode->addDrawable(shape.get());
      transform->addChild(geode.get());
      osgNode->addChild(transform.get());
    }

    /**
     * creates an arrow between start and end directed to "end"
     */
    void OSGDrawItem::createArrow(OSGDrawItem *osgNode,
                                  Vector start, Vector end, Color myColor) {
      osg::ref_ptr<osg::TessellationHints> hints = new osg::TessellationHints;
      osg::ref_ptr<osg::Geode> geode = new osg::Geode;
      osg::ref_ptr<osg::ShapeDrawable> shape;
      osg::ref_ptr<osg::PositionAttitudeTransform> transform =
        new osg::PositionAttitudeTransform();
      osg::Quat oquat;

      oquat.makeRotate(osg::Vec3(0,0,1), osg::Vec3(end.x()-start.x(),end.y()-start.y(),end.z()-start.z()));
      hints->setDetailRatio(1.0f);

      // create the arrow line
      createLine(osgNode, start, end, myColor);

      geode->addDrawable(shape.get());

      // TODO: do not use ShapeDrawable!
      shape = new osg::ShapeDrawable(new osg::Cone(osg::Vec3(0,0,0), 0.005f,0.01f), hints.get());
      shape->setColor(osg::Vec4(myColor.r, myColor.g, myColor.b, myColor.a));

      transform->addChild(geode.get());
      transform->setPosition(osg::Vec3(end.x(),end.y(),end.z()));
      transform->setAttitude(oquat);

      osgNode->addChild(transform.get());
    }

    void OSGDrawItem::createLine(OSGDrawItem *osgNode,
                                 Vector start, Vector end, Color myColor) {
      osg::ref_ptr<osg::Geometry> linesGeom = new osg::Geometry;
      osg::ref_ptr<osg::Geode> geode = new osg::Geode;

      osg::Vec3Array* vertices = new osg::Vec3Array(2);
      (*vertices)[0].set(start.x(), start.y(), start.z());
      (*vertices)[1].set(end.x(), end.y(), end.z());

      osg::Vec4Array* colors = new osg::Vec4Array;
      colors->push_back(osg::Vec4(myColor.r,myColor.g,myColor.b,myColor.a));

      osg::Vec3Array* normals = new osg::Vec3Array;
      normals->push_back(osg::Vec3(0.0f,1.0f,0.0f));

      linesGeom->setVertexArray(vertices);
      linesGeom->setColorArray(colors);
      linesGeom->setColorBinding(osg::Geometry::BIND_OVERALL);
      linesGeom->setNormalArray(normals);
      linesGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);
      linesGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,2));

      // add the points geometry to the geode.
      geode->addDrawable(linesGeom.get());
      osgNode->addChild(geode.get());
    }

    void OSGDrawItem::createLines(OSGDrawItem *osgNode,
                                  vector<float> _vertices, Color myColor) {
      osg::ref_ptr<osg::Geometry> linesGeom = new osg::Geometry;
      osg::ref_ptr<osg::Geode> geode = new osg::Geode;
      int num_lines = _vertices.size() / 6;
      osg::Vec3Array* vertices = new osg::Vec3Array(num_lines*2);
      osg::Vec4Array* colors = new osg::Vec4Array;

      for(int i=0; i<num_lines; i++) {
        (*vertices)[i*2].set(_vertices[i*6], _vertices[i*6+1], _vertices[i*6+2]);
        (*vertices)[i*2+1].set(_vertices[i*6+3], _vertices[i*6+4],
                               _vertices[i*6+5]);
        colors->push_back(osg::Vec4(myColor.r,myColor.g,myColor.b,myColor.a));
        colors->push_back(osg::Vec4(1.0,0.0,myColor.b,myColor.a));
      }

      osg::Vec3Array* normals = new osg::Vec3Array;
      normals->push_back(osg::Vec3(0.0f,1.0f,0.0f));

      linesGeom->setVertexArray(vertices);
      linesGeom->setColorArray(colors);
      linesGeom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
      linesGeom->setNormalArray(normals);
      linesGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);
      linesGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,num_lines*2));

      // add the points geometry to the geode.
      geode->addDrawable(linesGeom.get());
      osgNode->addChild(geode.get());
    }

    void OSGDrawItem::createTriangleSet(OSGDrawItem *osgNode,
                                        vector<float> vertices, Color myColor) {
      osg::ref_ptr<osg::Geode> geode = new osg::Geode;
      osg::ref_ptr<osg::Geometry> geometry(new osg::Geometry());

      osg::ref_ptr<osg::Vec3Array> v(new osg::Vec3Array());
      v->setDataVariance(osg::Object::DYNAMIC);
      for(unsigned int i=0; i<vertices.size()/3; i++) {
        v->push_back(osg::Vec3(vertices[i*3], vertices[(i*3)+1],
                               vertices[(i*3)+2]));
      }

      osg::ref_ptr<osg::Vec3Array> normals(new osg::Vec3Array());
      normals->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));

      osg::Vec4Array* colors = new osg::Vec4Array(1);
      (*colors)[0].set(myColor.r, myColor.g, myColor.b, myColor.a);

      geometry->setVertexArray(v.get());
      geometry->setDataVariance(osg::Object::DYNAMIC);
      geometry->setNormalArray(normals.get());
      geometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
      geometry->setColorArray(colors);
      geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
      geometry->addPrimitiveSet(new osg::DrawArrays(
                                                    osg::PrimitiveSet::TRIANGLES, 0, v->size()));

      geode->addDrawable(geometry.get());
      osgNode->addChild(geode.get());
    }
    void OSGDrawItem::addToTriangleSet(OSGDrawItem *di,
                                       vector<float> vertices, Color myColor) {
      osg::Node* node = di->asGroup()->getChild(0);
      if(node == NULL) return;
      osg::Geode* geode = dynamic_cast<osg::Geode*>(node);
      if(geode == NULL) return;
      osg::Drawable* drawable = geode->getDrawable(0);
      if(drawable == NULL) return;
      osg::Geometry* geometry = drawable->asGeometry();
      if(geometry == NULL) return;

      osg::Array* v;
      osg::Vec3Array* v2;
      osg::PrimitiveSet* primitive;
      osg::DrawArrays* draw_arrays;

      primitive = geometry->getPrimitiveSet(0);
      draw_arrays = dynamic_cast<osg::DrawArrays*>(primitive);
      if((v = geometry->getVertexArray())) {
        // TODO: broken ?
        // kann nicht klappen
        v2 = dynamic_cast<osg::Vec3Array*>(v);
        for(unsigned int i=0; i<vertices.size()/3; i++) {
          v2->push_back(osg::Vec3(vertices[i*3], vertices[(i*3)+1],
                                  vertices[(i*3)+2]));
        }
        draw_arrays->setCount(v2->size());
        osg::Vec4Array* colours = new osg::Vec4Array(1);
        (*colours)[0].set(myColor.r, myColor.g, myColor.b, myColor.a);
        geometry->setColorArray(colours);

        v2->dirty();
        geometry->dirtyDisplayList();
        geometry->dirtyBound();
      }
    }
    void OSGDrawItem::updateTriangleSet(OSGDrawItem *di, vector<float> vertices) {
      osg::Node* node = di->asGroup()->getChild(0);
      if(node == NULL) return;
      osg::Geode* geode = dynamic_cast<osg::Geode*>(node);
      if(geode == NULL) return;
      osg::Drawable* drawable = geode->getDrawable(0);
      if(drawable == NULL) return;
      osg::Geometry* geometry = drawable->asGeometry();
      if(geometry == NULL) return;

      osg::ref_ptr<osg::Vec3Array> v(new osg::Vec3Array());
      for(unsigned int i=0; i<vertices.size()/3; i++) {
        v->push_back(osg::Vec3(vertices[i*3], vertices[(i*3)+1],
                               vertices[(i*3)+2]));
      }
      geometry->setVertexArray(v.get());
      geometry->dirtyDisplayList();
      geometry->dirtyBound();
    }

    void OSGDrawItem::createRectangle(OSGDrawItem *osgNode,
                                      Vector a, Vector b,
                                      Vector q, Color myColor,
                                      Color borderColor,
                                      int align_to_view) {
      osg::ref_ptr<osg::Geometry> quadGeom = new osg::Geometry;
      osg::ref_ptr<osg::Geometry> linesGeom = new osg::Geometry;
      osg::ref_ptr<osg::Geode> geode = new osg::Geode;
      Vector n;

      a*=0.5;
      b*=0.5;

      osg::Vec3Array* corners = new osg::Vec3Array(5);
      (*corners)[0].set(-(a.x()+b.x()), -(a.y()+b.y()), -(a.z()+b.z()));
      (*corners)[1].set(a.x()-b.x(), a.y()-b.y(), a.z()-b.z());
      (*corners)[3].set(b.x()-a.x(), b.y()-a.y(), b.z()-a.z());
      (*corners)[2].set(a.x()+b.x(), a.y()+b.y(), a.z()+b.z());
      (*corners)[4].set(-(a.x()+b.x()), -(a.y()+b.y()), -(a.z()+b.z()));

      osg::ref_ptr<osg::Vec2Array> texcoords(new osg::Vec2Array());
      texcoords->push_back(osg::Vec2(0.0f, 0.0f));
      texcoords->push_back(osg::Vec2(1.0f, 0.0f));
      texcoords->push_back(osg::Vec2(1.0f, 1.0f));
      texcoords->push_back(osg::Vec2(0.0f, 1.0f));

      osg::Vec4Array* quadColors = new osg::Vec4Array(1);
      (*quadColors)[0].set(myColor.r, myColor.g, myColor.b, myColor.a);

      osg::Vec4Array* lineColors = new osg::Vec4Array(1);
      (*lineColors)[0].set(borderColor.r, borderColor.g,
                           borderColor.b, borderColor.a);

      osg::Vec3Array* normals = new osg::Vec3Array(1);
      n = a.cross(b);
      (*normals)[0].set(n.x(), n.y(), n.z());
      (*normals)[0].normalize();

      quadGeom->setVertexArray(corners);
      quadGeom->setTexCoordArray(0,texcoords.get());
      quadGeom->setColorArray(quadColors);
      quadGeom->setColorBinding(osg::Geometry::BIND_OVERALL);
      quadGeom->setNormalArray(normals);
      quadGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);
      quadGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));

      linesGeom->setVertexArray(corners);
      linesGeom->setColorArray(lineColors);
      linesGeom->setColorBinding(osg::Geometry::BIND_OVERALL);
      linesGeom->setNormalArray(normals);
      linesGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);
      linesGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, 5));

      geode->addDrawable(quadGeom.get());
      geode->addDrawable(linesGeom.get());

      if(align_to_view == 1) {
        osg::ref_ptr<osg::AutoTransform> at = new osg::AutoTransform;
        at->setPosition(osg::Vec3(q.x(), q.y(), q.z()));
        at->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_SCREEN);
        at->addChild(geode.get());
        osgNode->addChild(at.get());
      } else {
        osg::ref_ptr<osg::PositionAttitudeTransform> at = new osg::PositionAttitudeTransform;
        at->setPosition(osg::Vec3(q.x(),q.y(),q.z()));
        at->addChild(geode.get());
        osgNode->addChild(at.get());
      }
    }

    void OSGDrawItem::createCircle(OSGDrawItem *osgNode,
                                   Vector a, Vector b,
                                   Vector q, Color myColor,
                                   Color borderColor,
                                   int resolution) {
      osg::ref_ptr<osg::PositionAttitudeTransform> at = new osg::PositionAttitudeTransform;
      osg::ref_ptr<osg::Geometry> circleGeom = new osg::Geometry;
      osg::ref_ptr<osg::Geometry> linesGeom = new osg::Geometry;
      osg::ref_ptr<osg::Geode> geode = new osg::Geode;
      Quaternion rot = mars::utils::angleAxisToQuaternion(2.0*M_PI/resolution, q);
      //Quaternion rot(q, 6.2831853/((double)resolution));
      Vector tmp = b;

      // create points
      osg::Vec3Array* vertices = new osg::Vec3Array(resolution+2);
      (*vertices)[0].set(a.x(), a.y(), a.z());
      (*vertices)[1].set(a.x()+b.x(), a.y()+b.y(), a.z()+b.z());
      for(int i=0; i<resolution-1; ++i) {
        tmp = rot *tmp;
        (*vertices)[i+2].set(a.x()+tmp.x(), a.y()+tmp.y(), a.z()+tmp.z());
      }
      (*vertices)[resolution+1].set(a.x()+b.x(), a.y()+b.y(), a.z()+b.z());

      osg::Vec4Array* circleColours = new osg::Vec4Array(1);
      (*circleColours)[0].set(myColor.r, myColor.g, myColor.b, myColor.a);

      osg::Vec4Array* lineColours = new osg::Vec4Array(1);
      (*lineColours)[0].set(borderColor.r, borderColor.g,
                            borderColor.b, borderColor.a);

      osg::Vec3Array* normals = new osg::Vec3Array(1);
      (*normals)[0].set(q.x(), q.y(), q.z());
      (*normals)[0].set(0.0, 0.0, 1.0);
      (*normals)[0].normalize();

      circleGeom->setVertexArray(vertices);
      circleGeom->setColorArray(circleColours);
      circleGeom->setColorBinding(osg::Geometry::BIND_OVERALL);
      circleGeom->setNormalArray(normals);
      circleGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);
      circleGeom->addPrimitiveSet(new osg::DrawArrays(
                                                      osg::PrimitiveSet::TRIANGLE_FAN, 0, resolution+2));

      linesGeom->setVertexArray(vertices);
      linesGeom->setColorArray(lineColours);
      linesGeom->setColorBinding(osg::Geometry::BIND_OVERALL);
      linesGeom->setNormalArray(normals);
      linesGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);
      linesGeom->addPrimitiveSet(new osg::DrawArrays(
                                                     osg::PrimitiveSet::LINE_STRIP, 0, resolution+2));

      geode->addDrawable(circleGeom.get());
      geode->addDrawable(linesGeom.get());

      at->setPosition(osg::Vec3(0.0, 0.0, 0.0));
      at->addChild(geode.get());
      osgNode->addChild(at.get());
    }

    void OSGDrawItem::createHUDRectangle(OSGDrawItem *osgNode,
                                         sReal height, sReal width, Vector myoff, Color myColor) {
      osg::ref_ptr<osg::Geometry> quadGeom = new osg::Geometry;
      osg::ref_ptr<osg::Geode> geode = new osg::Geode;
      osg::ref_ptr<osg::Camera> curCam = osgNode->gw()->getMainCamera();
      osg::Matrixd orientation = curCam->getInverseViewMatrix();
      osg::Vec3 eye, up, center;
      // offset to move HUD element down z achsis of camera
      osg::Vec3 offset(myoff.x(), myoff.y(), myoff.z());

      // extract current cam data
      curCam->getViewMatrixAsLookAt(eye, up, center, 1);

      osg::ref_ptr<osg::Vec3Array> corners (new osg::Vec3Array(4));
      (*corners)[0].set(-height, -width, 0);
      (*corners)[1].set(height, -width, 0);
      (*corners)[2].set(height, width, 0);
      (*corners)[3].set(-height, width, 0);
      for(int i=0; i<4; i++) {
        (*corners)[i] = (*corners)[i] + offset;
        (*corners)[i] = (*corners)[i] * orientation;
      }

      osg::Vec4Array* colours = new osg::Vec4Array(1);
      (*colours)[0].set(myColor.r, myColor.g, myColor.b, myColor.a);

      osg::Vec3Array* normals = new osg::Vec3Array(1);
      (*normals)[0].set(0.0, 0.0, 1.0);
      (*normals)[0] = (*normals)[0] * orientation;

      osg::Vec2Array* texcoords = new osg::Vec2Array(4);
      (*texcoords)[0].set(0.0f,0.0f);
      (*texcoords)[1].set(1.0f,0.0f);
      (*texcoords)[2].set(1.0f,1.0f);
      (*texcoords)[3].set(0.0f,1.0f);

      quadGeom->setDataVariance(osg::Object::DYNAMIC);
      quadGeom->setVertexArray(corners.get());
      quadGeom->setColorArray(colours);
      quadGeom->setColorBinding(osg::Geometry::BIND_OVERALL);
      quadGeom->setNormalArray(normals);
      quadGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);
      quadGeom->setTexCoordArray(0,texcoords);
      quadGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));

      geode->addDrawable(quadGeom.get());
      // TODO: what does 11 mean ??
      geode->getOrCreateStateSet()->setRenderBinDetails( 11, "RenderBin");

      osgNode->addChild(geode.get());
    }

    void OSGDrawItem::updateHUDRectangle(OSGDrawItem *di,
                                         sReal height, sReal width,
                                         Vector myoff, Color myColor) {

      (void)myColor;

      osg::Node* node = di->asGroup()->getChild(0);
      if(node == NULL) return;
      osg::Geode* geode = dynamic_cast<osg::Geode*>(node);
      if(geode == NULL) return;
      osg::Drawable* drawable = geode->getDrawable(0);
      if(drawable == NULL) return;
      osg::Geometry* geometry = drawable->asGeometry();
      if(geometry == NULL) return;

      osg::ref_ptr<osg::Camera> curCam = di->gw()->getMainCamera();
      osg::Vec3d eye, up, center;
      osg::Matrixd orientation;
      osg::Vec3d offset(myoff.x(), myoff.y(), myoff.z());
      osg::Quat t_q = curCam->getInverseViewMatrix().getRotate();

      orientation.makeRotate(t_q);
      eye = curCam->getInverseViewMatrix().getTrans();

      osg::ref_ptr<osg::Vec3Array> corners = new osg::Vec3Array(4);
      (*corners)[0].set(-height, -width, 0);
      (*corners)[1].set(height, -width, 0);
      (*corners)[2].set(height, width, 0);
      (*corners)[3].set(-height, width, 0);
      for(int i=0; i<4; i++) {
        (*corners)[i] = (*corners)[i] + offset;
        (*corners)[i] = (*corners)[i] * orientation;
        (*corners)[i] = (*corners)[i] + eye;
      }

      geometry->setVertexArray(corners.get());
      geometry->dirtyDisplayList();
      geometry->dirtyBound();
    }

    void OSGDrawItem::createLabel(OSGDrawItem *osgNode,
                                  Vector pos, sReal size, std::string label,
                                  std::string fontPath,
                                  double r, double g, double b, double a,
                                  int align_to_view) {

      osg::ref_ptr<osg::Geode> geode = new osg::Geode();

      string timesFont = fontPath;
      timesFont.append("/arial.ttf");
  
      osgText::Text* text = new  osgText::Text;
      geode->addDrawable( text);
      text->setFont(timesFont);
      text->setPosition(osg::Vec3(0.0f,0.0f,0.0f));
      text->setCharacterSize(size);
      text->setAxisAlignment(osgText::Text::XY_PLANE);
      text->setAlignment(osgText::Text::CENTER_CENTER);
      text->setText(label);
      text->setColor(osg::Vec4(r, g, b, a));
      if(align_to_view == 1) {
        osg::ref_ptr<osg::AutoTransform> at = new osg::AutoTransform;
        at->setPosition(osg::Vec3(pos.x(),pos.y(),pos.z()));
        at->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_SCREEN);
        at->addChild(geode.get());
        osgNode->addChild(at.get());
      }
      else {
        osg::ref_ptr<osg::PositionAttitudeTransform> at = new osg::PositionAttitudeTransform;
        at->setPosition(osg::Vec3(pos.x(),pos.y(),pos.z()));
        at->addChild(geode.get());
        osgNode->addChild(at.get());
      }
    }
    void OSGDrawItem::updateLabel(OSGDrawItem *di,
                                  Vector pos, sReal size, string label,
                                  double r, double g, double b, double a,
                                  int align_to_view) {
      (void)size;
      osg::Node *node = di->asGroup()->getChild(0);
      osg::Node *transformNode = node->asTransform()->getChild(0);
      osgText::Text *text = (osgText::Text*) (transformNode->asGeode()->getDrawable(0));
      text->setText(label);
      text->setColor(osg::Vec4(r, g, b, a));
      if (align_to_view == 1) {
        osg::AutoTransform *transform = (osg::AutoTransform*)(node->asTransform());
        transform->setPosition(osg::Vec3(pos.x(),pos.y(),pos.z()));
      } else {
        node->asTransform()->asPositionAttitudeTransform()->setPosition(osg::Vec3(pos.x(),pos.y(),pos.z()));
      }
    }

    void OSGDrawItem::createPlane(OSGDrawItem *osgNode,
                                  double w, double l, double h, double res) {
      osg::ref_ptr<osg::Geode> geode = new osg::Geode;
      osg::ref_ptr<osg::Geometry> geometry(new osg::Geometry());
      double iStep = w/res;
      double jStep = l/res;

      // create plane vertices
      osg::ref_ptr<osg::Vec3Array> vertices(new osg::Vec3Array());
      osg::ref_ptr<osg::Vec2Array> texcoords(new osg::Vec2Array());
      for (double i =-(w/2); i<(w/2);) {
        for (double j=-(l/2); j<(l/2);) {
          vertices->push_back(osg::Vec3(i, j, h));
          texcoords->push_back(osg::Vec2(0.0f, 0.0f));
          vertices->push_back(osg::Vec3(i+iStep, j, h));
          texcoords->push_back(osg::Vec2(0.5f, 0.0f));
          vertices->push_back(osg::Vec3(i+iStep, j+jStep, h));
          texcoords->push_back(osg::Vec2(0.5f, 0.5f));
          vertices->push_back(osg::Vec3(i, j, h));
          texcoords->push_back(osg::Vec2(0.0f, 0.0f));
          vertices->push_back(osg::Vec3(i+iStep, j+jStep, h));
          texcoords->push_back(osg::Vec2(0.5f, 0.5f));
          vertices->push_back(osg::Vec3(i, j+jStep, h));
          texcoords->push_back(osg::Vec2(0.0f, 0.5f));
          j+=jStep;
        }
        i+=iStep;
      }

      osg::ref_ptr<osg::Vec3Array> normals(new osg::Vec3Array());
      normals->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));

      geometry->setVertexArray(vertices.get());
      geometry->setTexCoordArray(0,texcoords.get());
      geometry->setNormalArray(normals.get());
      geometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
      geometry->addPrimitiveSet(new osg::DrawArrays(
                                                    osg::PrimitiveSet::TRIANGLES, 0, vertices->size()));

      geode->addDrawable(geometry.get());
      osgNode->addChild(geode.get());
    }

    void OSGDrawItem::createPoints(OSGDrawItem *osgNode,
                                   vector<float> vertices, Color myColor) {
      osg::ref_ptr<osg::Geode> geode = new osg::Geode;
      osg::ref_ptr<osg::Geometry> geometry(new osg::Geometry());

      osg::ref_ptr<osg::Vec3Array> v(new osg::Vec3Array());
      v->setDataVariance(osg::Object::DYNAMIC);
      for(unsigned int i=0; i<vertices.size()/3; i++) {
        v->push_back(osg::Vec3(vertices[i*3], vertices[(i*3)+1], vertices[(i*3)+2]));
      }

      osg::ref_ptr<osg::Vec3Array> normals(new osg::Vec3Array());
      normals->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));

      osg::Vec4Array* colours = new osg::Vec4Array(1);
      (*colours)[0].set(myColor.r, myColor.g, myColor.b, myColor.a);

      geometry->setVertexArray(v.get());
      geometry->setDataVariance(osg::Object::DYNAMIC);
      geometry->setNormalArray(normals.get());
      geometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
      geometry->setColorArray(colours);
      geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
      geometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, v->size()));

      geode->addDrawable(geometry.get());
      osgNode->addChild(geode.get());
    }

    void OSGDrawItem::updatePoints(OSGDrawItem *di, vector<float> vertices) {
      osg::Node* node = di->asGroup()->getChild(0);
      if(node == NULL) return;
      osg::Geode* geode = dynamic_cast<osg::Geode*>(node);
      if(geode == NULL) return;
      osg::Drawable* drawable = geode->getDrawable(0);
      if(drawable == NULL) return;
      osg::Geometry* geometry = drawable->asGeometry();
      if(geometry == NULL) return;

      osg::ref_ptr<osg::Vec3Array> v(new osg::Vec3Array());
      for(unsigned int i=0; i<vertices.size()/3; i++) {
        v->push_back(osg::Vec3(vertices[i*3], vertices[(i*3)+1],
                               vertices[(i*3)+2]));
      }
      geometry->setVertexArray(v.get());
      geometry->dirtyDisplayList();
      geometry->dirtyBound();
      (dynamic_cast<osg::DrawArrays*>(geometry->getPrimitiveSet(0)))->setCount(vertices.size()/3);
    }

  } // end of namespace graphics
} // end of namespace mars
