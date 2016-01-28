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
 *  SphereDrawObject.cpp
 *  General SphereDrawObject to inherit from.
 *
 *  Created by Roemmermann on 21.10.09.
 */

#include <osg/Geometry>

#include "SphereDrawObject.h"
#include "MarsMaterial.h"

namespace mars {
  namespace graphics {

    using namespace std;

    typedef struct {
      osg::Vec3 p1;
      osg::Vec3 p2;
      osg::Vec3 p3;
    } SphereFace;

    SphereDrawObject::SphereDrawObject(GraphicsManager *g)
      : DrawObject(g) {
    }


    SphereDrawObject::~SphereDrawObject() {

    }

    std::vector<SphereFace>* makeSphere(unsigned int levelOfDetail) {
      vector<SphereFace> *faces = new vector<SphereFace>(pow(4.0,(int)levelOfDetail) * 8);
      vector<SphereFace> &f = *faces;
      unsigned int i, j, numFaces=0, numNewFaces;
      osg::Vec3 pa,pb,pc;

      { // setup initial level
        double a = 1.0 / sqrt(2.0) + 0.001;
        osg::Vec3 p[6] = {
          osg::Vec3(0,0,1), osg::Vec3(0,0,-1), osg::Vec3(-a,-a,0),
          osg::Vec3(a,-a,0), osg::Vec3(a,a,0), osg::Vec3(-a,a,0)
        };
        f[0] = (SphereFace) { p[0], p[3], p[4] };
        f[1] = (SphereFace) { p[0], p[4], p[5] };
        f[2] = (SphereFace) { p[0], p[5], p[2] };
        f[3] = (SphereFace) { p[0], p[2], p[3] };
        f[4] = (SphereFace) { p[1], p[4], p[3] };
        f[5] = (SphereFace) { p[1], p[5], p[4] };
        f[6] = (SphereFace) { p[1], p[2], p[5] };
        f[7] = (SphereFace) { p[1], p[3], p[2] };
        numFaces = 8;
      }

      for (j=0; j<levelOfDetail; ++j) {
        numNewFaces = numFaces;
        for (i=0; i<numNewFaces; ++i) {
          pa = (f[i].p1 + f[i].p2)*0.5; pa.normalize();
          pb = (f[i].p2 + f[i].p3)*0.5; pb.normalize();
          pc = (f[i].p3 + f[i].p1)*0.5; pc.normalize();

          f[numFaces] = (SphereFace) { f[i].p1, pa, pc }; ++numFaces;
          f[numFaces] = (SphereFace) { pa, f[i].p2, pb }; ++numFaces;
          f[numFaces] = (SphereFace) { pb, f[i].p3, pc }; ++numFaces;

          f[i] = (SphereFace) { pa, pb, pc };
        }
      }

      return faces;
    }

    static void sphereUV(const osg::Vec3 &p,
                         float *s, float *t) {
      float x = p.x();
      float y = p.y();
      float length = sqrt(x*x+y*y);
      x /= length;
      y /= length;
      if(x > 1.0) x = 1.0;
      else if(x<-1.0) x = -1.0;
      *s = (asin(-x)/(M_PI*2))+0.25;
      if(y<0) *s = 1-*s;
      *t = (asin(p.z()) / (M_PI))+0.5;
      /*
       *s = atan2(p.x(), p.y()) / (2.0 * M_PI) + 0.5;
       *t = asin(p.z()) / M_PI;
       */
    }

    void SphereDrawObject::createGeometry(
                                          osg::Vec3Array *vertices,
                                          osg::Vec3Array *normals,
                                          osg::Vec2Array *uv,
                                          float radius,
                                          const osg::Vec3 &topOffset,
                                          const osg::Vec3 &bottomOffset,
                                          bool backfaces,
                                          unsigned int levelOfDetail) {

      osg::Vec3 zero(0.0f, 0.0f, 0.0f);
      vector<SphereFace> *faces = makeSphere(levelOfDetail);
      float s1, s2, s3, t1, t2, t3;
      vector<osg::Vec3> verts(3);
      vector<osg::Vec3> nors(3);
      vector<osg::Vec2> uvs(3);
      osg::Vec3 v;
      const osg::Vec3 *offset;
      osg::Vec3 halfOffset = (bottomOffset-topOffset)*0.5;
      float factor;
      int i, face=0;
  
      if(vertices->size() != faces->size()*3) {
        vertices->clear();
        vertices->resize(faces->size()*3);
        normals->clear();
        normals->resize(faces->size()*3);
        uv->clear();
        uv->resize(faces->size()*3);
      }

      for(vector<SphereFace>::iterator it = faces->begin();
          it != faces->end(); ++it, ++face) {

        // add triangle face vertices
        if(it->p1.z() >= 0.0 && it->p2.z() >= 0.0 && it->p3.z() >= 0.0) {
          offset = &topOffset;
          factor = 1.0;
        } else {
          offset = &bottomOffset;
          factor = -1.0;
        }

        verts[0] = it->p1 * radius + *offset;
        verts[1] = it->p2 * radius + *offset;
        verts[2] = it->p3 * radius + *offset;
        // normals are easy, sphere center is origin
        nors[0] = it->p1;
        nors[1] = it->p2;
        nors[2] = it->p3;

        // calculate uv coordinates for vertices
        sphereUV(it->p1, &s1, &t1);
        sphereUV(it->p2, &s2, &t2);
        if(s2 < 0.15 && s1 > 0.85) s2 += 1.0;
        else if(s2 > 0.85 && s1 < 0.15) s2 -= 1.0;
        sphereUV(it->p3, &s3, &t3);
        if(s3 < 0.15 && s2 > 0.85 && s1 > 0.85) s3 += 1.0;
        else if(s3 > 0.85 && s2 < 0.15 && s1 < 0.15) s3 -= 1.0;

        /*
          uvs[0] = osg::Vec2(s1*6.0, t1*3.0);
          uvs[1] = osg::Vec2(s2*6.0, t2*3.0);
          uvs[2] = osg::Vec2(s3*6.0, t3*3.0);
        */    
        uvs[0] = osg::Vec2(s1, t1);
        uvs[1] = osg::Vec2(s2, t2);
        uvs[2] = osg::Vec2(s3, t3);

        // make capsule
        // FIXME: uv coordinates have some glitches
        float buf; //, dz1, dz2, dz3;
        if(it->p1.z() == 0.0) {
          buf = verts[0].z();
          verts[0] += halfOffset*factor;
          uvs[0].y() += 2.0*(buf-verts[0].z())*factor;
        }
        if(it->p2.z() == 0.0) {
          buf = verts[1].z();
          verts[1] += halfOffset*factor;
          uvs[1].y() += 2.0*(buf-verts[1].z())*factor;
        }
        if(it->p3.z() == 0.0) {
          buf = verts[2].z();
          verts[2] += halfOffset*factor;
          uvs[2].y() += 2.0*(buf-verts[2].z())*factor;
        }

        if(backfaces) {
          for(i=3; i>0; --i) {
            vertices->at(face+i) = verts[i];
            normals->at(face+i) = nors[i];
            uv->at(face+i) = uvs[i];
          }
        } else {
          for(i=0; i<3; ++i) {
            vertices->at(3*face+i) = verts[i];
            normals->at(3*face+i) = nors[i];
            uv->at(3*face+i) = uvs[i];
          }
        }
      }
      delete faces;
    }

    std::list< osg::ref_ptr< osg::Geode > > SphereDrawObject::createGeometry() {
      osg::ref_ptr<osg::Vec3Array> vertices(new osg::Vec3Array());
      osg::ref_ptr<osg::Vec3Array> normals(new osg::Vec3Array());
      osg::ref_ptr<osg::Vec2Array> uv(new osg::Vec2Array());
      osg::Vec3 zero(0.0f, 0.0f, 0.0f);
      osg::Geometry *geom = new osg::Geometry();
      std::list< osg::ref_ptr< osg::Geode > > geodes;

      createGeometry(vertices.get(), normals.get(), uv.get(),
                     1.0, zero, zero, false, 2);

      geom->setVertexArray(vertices.get());
      geom->setNormalArray(normals.get());
      geom->setTexCoordArray(DEFAULT_UV_UNIT, uv.get());
      geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
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

    /*
    void SphereDrawObject::setScaledSize(const mars::utils::Vector &scaledSize) {
      setScale(mars::utils::Vector(scaledSize.x() / geometrySize_.x(),
                                   scaledSize.x() / geometrySize_.y(),
                                   scaledSize.x() / geometrySize_.z()));
    }
    */
  } // end of namespace graphics
} // end of namespace mars
