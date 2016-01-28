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
 *  CylinderDrawObject.cpp
 *  General CylinderDrawObject to inherit from.
 *
 *  Created by Roemmermann on 21.10.09.
 */

#include <osg/Geometry>

#include "CylinderDrawObject.h"
#include "MarsMaterial.h"

namespace mars {
  namespace graphics {

    using mars::utils::Vector;
    using mars::interfaces::sReal;

    CylinderDrawObject::CylinderDrawObject(GraphicsManager *g,
                                           sReal radius, sReal height)
      : DrawObject(g), radius_(radius), height_(height) {
      geometrySize_.y() = geometrySize_.x() = radius_*2;
      geometrySize_.z() = height_;
    }

    void CylinderDrawObject::createShellGeometry(osg::Vec3Array *vertices,
                                                 osg::Vec3Array *normals,
                                                 osg::Vec2Array *uv,
                                                 float radius,
                                                 float height,
                                                 float theta) {
      osg::Vec3 heightOffset1(0.0f, 0.0f, -0.5*height);
      osg::Vec3 heightOffset2(0.0f, 0.0f, 0.5*height);
      osg::Vec3 currPoint, v;
      osg::Vec2 v2;
      bool done = false;

#define CYLINDER_POINT_(p,n,u)                  \
      v = p; v2.set(u, v.z());                  \
      vertices->push_back(v);                   \
      v = n; v.normalize();                     \
      normals->push_back(v);                    \
      uv->push_back(v2);

      for(float alpha=0.0f; !done; alpha+=theta) {
        if(alpha>=2.0*M_PI) {
          alpha = 2.0*M_PI;
          done = true;
        }

        float sa = radius*sin(alpha);
        float ca = radius*cos(alpha);

        currPoint.set( sa, ca, 0.0f );

        CYLINDER_POINT_(currPoint + heightOffset1, currPoint, alpha);
        CYLINDER_POINT_(currPoint + heightOffset2, currPoint, alpha);
      }
#undef CYLINDER_POINT_
    }

    void createCircleGeometry(osg::Vec3Array *vertices,
                              osg::Vec3Array *normals,
                              osg::Vec2Array *uv,
                              float radius,
                              float theta,
                              osg::Vec3 offset,
                              bool switchFace) {
      osg::Vec3 topVec(0.0f, 0.0f, 1.0f);
      osg::Vec3 p(0.0f, 0.0f, 0.0f);
      osg::Vec2 v2;
      bool done = false;

      for(float alpha=0.0f; !done; alpha+=theta) {
        if(alpha>=2.0*M_PI) {
          alpha = 2.0*M_PI;
          done = true;
        }

        if(!switchFace) {
          p.set(0.0f, 0.0f, 0.0f);
          normals->push_back(-topVec);
        } else {
          p.set(radius*sin(alpha), radius*cos(alpha), 0.0f);
          normals->push_back(topVec);
        }
        vertices->push_back(p + offset);
        v2.set(p.x(), p.y());
        uv->push_back(v2);

        if(!switchFace) {
          p.set(radius*sin(alpha), radius*cos(alpha), 0.0f);
          normals->push_back(-topVec);
        } else {
          p.set(0.0f, 0.0f, 0.0f);
          normals->push_back(topVec);
        }
        vertices->push_back(p + offset);
        v2.set(p.x(), p.y());
        uv->push_back(v2);
      }
    }

    std::list< osg::ref_ptr< osg::Geode > > CylinderDrawObject::createGeometry() {
      osg::ref_ptr<osg::Vec3Array> vertices(new osg::Vec3Array());
      osg::ref_ptr<osg::Vec3Array> normals(new osg::Vec3Array());
      osg::ref_ptr<osg::Vec2Array> uv(new osg::Vec2Array());
      osg::Geometry *geom = new osg::Geometry();
      std::list< osg::ref_ptr< osg::Geode > > geodes;

      double step = 0.4;
      // top cap geom
      createCircleGeometry(
                           vertices, normals, uv,
                           radius_, step,
                           osg::Vec3(0.0f, 0.0f, 0.5*height_),
                           true);
      // cylinder shell geom
      CylinderDrawObject::createShellGeometry(
                                              vertices, normals, uv,
                                              radius_, height_, step);
      // bottom cap geom
      createCircleGeometry(
                           vertices, normals, uv,
                           radius_, step,
                           osg::Vec3(0.0f, 0.0f, -0.5*height_),
                           false);

      geom->setVertexArray(vertices.get());
      geom->setNormalArray(normals.get());
      geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
      geom->setTexCoordArray(DEFAULT_UV_UNIT,uv.get());
      geom->addPrimitiveSet(new osg::DrawArrays(
                                                osg::PrimitiveSet::TRIANGLE_STRIP,
                                                0, // index of first vertex
                                                vertices->size()));

      geom->setUseDisplayList(false);
      geom->setUseVertexBufferObjects(true);
      osg::ref_ptr<osg::Geode> geode = new osg::Geode;
      geode->addDrawable(geom);
      geodes.push_back(geode);

      return geodes;
    }

    void CylinderDrawObject::setScaledSize(const Vector &scaledSize) {
      setScale(Vector(scaledSize.x() / geometrySize_.x(),
                      scaledSize.y() / geometrySize_.y(),
                      scaledSize.z() / geometrySize_.z()));
    }

  } // end of namespace graphics
} // end of namespace mars
