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
 *  CapsuleDrawObject.cpp
 *  General CapsuleDrawObject to inherit from.
 *
 *  Created by Roemmermann on 21.10.09.
 */

#include "CapsuleDrawObject.h"
#include "CylinderDrawObject.h"
#include "SphereDrawObject.h"

#include <osg/Geometry>

namespace mars {
  namespace graphics {

    using mars::interfaces::sReal;
    using mars::utils::Vector;

    CapsuleDrawObject::CapsuleDrawObject(GraphicsManager *g)
      : DrawObject(g) {
    }

    std::list< osg::ref_ptr< osg::Geode > > CapsuleDrawObject::createGeometry() {
      osg::ref_ptr<osg::Vec3Array> vertices(new osg::Vec3Array());
      osg::ref_ptr<osg::Vec3Array> normals(new osg::Vec3Array());
      osg::ref_ptr<osg::Vec2Array> uv(new osg::Vec2Array());
      geom = new osg::Geometry();
      std::list< osg::ref_ptr< osg::Geode > > geodes;

      // create sphere caps
      osg::Vec3 sphereTopOffset(0.0f, 0.0f, 0.5f - 0.0018f);
      osg::Vec3 sphereBottomOffset(0.0f, 0.0f, -0.5f + 0.0018f);
      SphereDrawObject::createGeometry(vertices.get(), normals.get(), uv.get(),
                                       1.0, sphereTopOffset, sphereBottomOffset, false, 2);

      geom->setVertexArray(vertices.get());
      geom->setNormalArray(normals.get());
      geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
      geom->setTexCoordArray(DEFAULT_UV_UNIT, uv.get());
      geom->addPrimitiveSet(new osg::DrawArrays(
                                                osg::PrimitiveSet::TRIANGLES,
                                                0, // index of first vertex
                                                vertices->size()));

      geom->setUseDisplayList(false);
      geom->setUseVertexBufferObjects(true);
      osg::ref_ptr<osg::Geode> geode = new osg::Geode;
      geode->addDrawable(geom);
      geodes.push_back(geode);

      return geodes;
    }

    void CapsuleDrawObject::setScaledSize(const Vector &scaledSize) {
      /*
        setScale(Vector(
        2*scaledSize.x / geometrySize_.x,
        2*scaledSize.x / geometrySize_.y,
        (scaledSize.y + 2*scaledSize.x) / geometrySize_.z));
      */
      // create sphere caps
      double radius_ = scaledSize.x()*0.5;
      double height_ = scaledSize.y();// + 2*scaledSize.x();
      //fprintf(stderr, "capsule size: %g %g\n", scaledSize.x(), scaledSize.y());
      osg::Vec3 sphereTopOffset(0.0f, 0.0f, 0.5*height_ - 0.0018f);
      osg::Vec3 sphereBottomOffset(0.0f, 0.0f, -0.5*height_ + 0.0018f);
      SphereDrawObject::createGeometry(
                                       dynamic_cast<osg::Vec3Array*>(geom->getVertexArray()),
                                       dynamic_cast<osg::Vec3Array*>(geom->getNormalArray()),
                                       dynamic_cast<osg::Vec2Array*>(geom->getTexCoordArray(DEFAULT_UV_UNIT)),
                                       radius_, sphereTopOffset,
                                       sphereBottomOffset, false, 2);
      geom->dirtyDisplayList();
      geom->dirtyBound();
    }

  } // end of namespace graphics
} // end of namespace mars
