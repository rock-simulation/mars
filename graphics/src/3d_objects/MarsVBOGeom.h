/*
 *  Copyright 2012, DFKI GmbH Robotics Innovation Center
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

#ifndef MarsVBOGeom_H
#define MarsVBOGeom_H

#ifdef USE_MARS_VBO

// prohibit MSVC to create the "min" and "max" macros
#define NOMINMAX

#ifdef WIN32
#include <windows.h>
#include <GL/glew.h>
GLEW_FUN_EXPORT PFNGLGENBUFFERSPROC glGenBuffers;
GLEW_FUN_EXPORT PFNGLBINDBUFFERPROC glBindBuffer;
GLEW_FUN_EXPORT PFNGLDELETEBUFFERSPROC glDeleteBuffers;
GLEW_FUN_EXPORT PFNGLBUFFERDATAPROC glBufferData;
GLEW_FUN_EXPORT PFNGLMAPBUFFERPROC glMapBuffer;
GLEW_FUN_EXPORT PFNGLUNMAPBUFFERPROC glUnmapBuffer;
#endif

#include <GL/gl.h>
#include <GL/glext.h>
#include <map>
#include <list>
#include <stdio.h>
#include <osg/Drawable>

namespace mars {

  namespace graphics {

    struct VertexData;

    class MarsVBOGeom : public osg::Drawable {
    public:
      MarsVBOGeom();
      MarsVBOGeom(const MarsVBOGeom &pg,
                  const osg::CopyOp &copyop=osg::CopyOp::SHALLOW_COPY) {
        fprintf(stderr, "error: not implemented yet!!");
      }

      ~MarsVBOGeom();

      virtual osg::Object* cloneType() const {
        fprintf(stderr, "error: not implemented yet!!");
        return new MarsVBOGeom();
      }

      virtual osg::Object* clone(const osg::CopyOp& copyop) const {
        fprintf(stderr, "error: not implemented yet!!");
        return new MarsVBOGeom (*this, copyop);
      }

      virtual void drawImplementation(osg::RenderInfo& renderInfo) const;
      virtual osg::BoundingBox computeBound() const;
      void render();

      void setVertexArray(const std::vector<osg::Vec3> &vertices_) {
        vertices = vertices_;
      }
      void setNormalArray(const std::vector<osg::Vec3> &normals_) {
        normals = normals_;
      }
      void setTexCoordArray(const std::vector<osg::Vec2> &texcoords_) {
        texcoords = texcoords_;
      }

    protected:
      void clear();

    private:
      std::vector<osg::Vec3> vertices;
      std::vector<osg::Vec3> normals;
      std::vector<osg::Vec2> texcoords;

      GLuint *vboIds;
      bool isInitialized;

      VertexData *vData;
      GLuint *indices;

      int numVertices, numIndices;
      int indicesToDraw;

      bool wireframe, solid, haveTexcoords;

      void normalize(float *v);
      void initGeom();

      void initialize();

    }; // class MarsVBOGeom
  
  } // namespace graphics
} // namespace mars

#endif // USE_MARS_VBO

#endif /* MarsVBOGeom_H */

