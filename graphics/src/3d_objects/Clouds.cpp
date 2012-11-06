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
 * Clouds.cpp
 *
 *  Created on: 19.04.2011
 *      Author: daniel
 */

#include "Clouds.h"
#include "SphereDrawObject.h"

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/TextureRectangle>
#include <osg/Matrix>
#include <osgDB/ReadFile>

namespace mars {
  namespace graphics {

    using namespace std;

    Clouds::Clouds(const string &texture_path) : osg::Group() {

      osg::ref_ptr<osg::Vec3Array> vertices(new osg::Vec3Array());
      osg::ref_ptr<osg::Vec3Array> normals(new osg::Vec3Array());
      osg::ref_ptr<osg::Vec2Array> uv(new osg::Vec2Array());
      osg::ref_ptr<osg::Geode> geode = new osg::Geode;
      osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
      osg::Vec3 zero(0.0f, 0.0f, 0.0f);
      osg::Vec4 diffuse(0.6,0.6,0.6,1);
      osg::Vec4 emission(0.7,0.7,0.7,1);
      osg::StateSet *states = getOrCreateStateSet();

      SphereDrawObject::createGeometry(
                                       vertices, normals, uv,
                                       50.0, zero, zero, false, 6); // TODO: set LOD?

      geom->setVertexArray(vertices.get());
      geom->setNormalArray(normals.get());
      geom->setTexCoordArray(DEFAULT_UV_UNIT, uv);
      geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
      geom->addPrimitiveSet(new osg::DrawArrays(
                                                osg::PrimitiveSet::TRIANGLES,
                                                0, // index of first vertex
                                                vertices->size()));

      geode->addDrawable(geom.get());

      osg::ref_ptr<osg::MatrixTransform> tx = new osg::MatrixTransform;
      tx->setMatrix(osg::Matrix::rotate(1.570796326794897, 0, 1, 0)*
                    osg::Matrix::scale(1.0, 1.0, 0.2)*
                    osg::Matrix::translate(0.0, 0.0, -2.0));
      tx->setDataVariance(osg::Node::STATIC);
      tx->addChild(geode.get());

      addChild(tx.get());

      //read in standard texture
      string texpath = texture_path;
      texpath.append("/clouds.jpg");
      osg::Image* cloudsTextureImage = osgDB::readImageFile(texpath);

      //load and set texture
      osg::Texture2D *texture = new osg::Texture2D(cloudsTextureImage);
      texture->setDataVariance(osg::Object::DYNAMIC);
      texture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
      texture->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
      states->setTextureAttributeAndModes(0, texture,
                                          osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

      texmat = new osg::TexMat;
      texmat->setScaleByTextureRectangleSize(true);
      states->setTextureAttributeAndModes(0, texmat.get(),
                                          osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

      states->setMode(GL_LIGHTING,
                      osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
      states->setMode(GL_FOG, osg::StateAttribute::OFF);

      //tex_x = 0;
      //tex_y = 0;
    }

    /*
      void Clouds::move() {
      (tex_x<512.0) ? tex_x += 0.15 : tex_x = 0;
      texmat->setMatrix(osg::Matrix::translate(tex_x,1.0,1.0)*
      osg::Matrix::scale(0.5, 0.5, 1.0));
      }
    */

  } // end of namespace graphics
} // end of namespace mars
