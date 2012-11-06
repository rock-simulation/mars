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

#ifdef USE_MARS_VBO

/* The prototypes are only declared in GL/glext.h if this is defined */
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1 //for glGenBuffers, glBindBuffer,
                              //glDeleteBuffers
#endif

#include "MarsVBOGeom.h"
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <string.h>


namespace mars {

  namespace graphics {

    inline double sqr(double x) {
      return x*x;
    }

    struct VertexData {
      float position[3];
      float normal[3];
      float tangent[4];
      float texCoord[2];
    };

    MarsVBOGeom::MarsVBOGeom()
      : vboIds(new GLuint[2]), isInitialized(false) {

      wireframe = false;
      solid = true;
    }

    MarsVBOGeom::~MarsVBOGeom() {
      clear();
      delete[] vboIds;
      vboIds = NULL;
    }

    void MarsVBOGeom::initialize() {

      if(vertices.size() == 0) {
        fprintf(stderr, "MarsVBOGeom::initialize: no vertices\n");
        return;
      }
      if(vertices.size() != normals.size()) {
        fprintf(stderr, "MarsVBOGeom::initialize: not enough normals %u %u\n",
                vertices.size(), normals.size());
        return;
      }
      if(vertices.size() != texcoords.size()) {
        if(texcoords.size() != 0) {
          fprintf(stderr,
                  "MarsVBOGeom::initialize: wrong number texcoords %u\n",
                  texcoords.size());
          texcoords.clear();
        }
        haveTexcoords = false;
      }
      else haveTexcoords = true;

#ifdef WIN32
      // get the pointer to the GL functions
      glGenBuffers = (PFNGLGENBUFFERSPROC) wglGetProcAddress("glGenBuffers");
      glBindBuffer = (PFNGLBINDBUFFERPROC) wglGetProcAddress("glBindBuffer");
      glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) wglGetProcAddress("glDeleteBuffers");
      glBufferData = (PFNGLBUFFERDATAPROC) wglGetProcAddress("glBufferData");
      glMapBuffer = (PFNGLMAPBUFFERPROC) wglGetProcAddress("glMapBuffer");
      glUnmapBuffer = (PFNGLUNMAPBUFFERPROC) wglGetProcAddress("glUnmapBuffer");
#endif

      // Generate 2 VBOs
      glGenBuffers(2, vboIds);

      isInitialized = true;

      initGeom();
    }

    void MarsVBOGeom::clear() {

      if(isInitialized) {
        free (vData);
        vData = NULL;
        free (indices);
        indices = NULL;
        glBindBuffer(GL_ARRAY_BUFFER, vboIds[0]);
        glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);
        glDeleteBuffers(2, vboIds);
      }

    }

    void MarsVBOGeom::render() {

      if(!isInitialized) {
        initialize();
      }
      else {
        // Tell OpenGL which VBOs to use
        glBindBuffer(GL_ARRAY_BUFFER, vboIds[0]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[1]);

        // do same as vertex array except pointer
        // activate vertex coords array
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);

        // last param is offset, not ptr
        glVertexPointer(3, GL_FLOAT, sizeof(VertexData), 0);
        // last param is offset, not ptr
        glNormalPointer(GL_FLOAT, sizeof(VertexData),
                        (GLvoid*)( (char*)NULL + 3*sizeof(GLfloat)));

        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData),
                              (GLvoid*)( (char*)NULL + 6*sizeof(GLfloat)));

        glClientActiveTextureARB(GL_TEXTURE0);
        glTexCoordPointer(2, GL_FLOAT, sizeof(VertexData),
                          (GLvoid*)( (char*)NULL + 10*sizeof(GLfloat)));
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        if(solid) {
          glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
          glDrawElements(GL_TRIANGLES, indicesToDraw, GL_UNSIGNED_INT, 0);
        }

        //GLfloat mat_diffuse[] = { 1.0, 0.0, 0.0, 1.0 };
        //glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);

        if(wireframe) {
          glLineWidth(2.0);
        
          glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
          glDrawElements(GL_TRIANGLES, indicesToDraw, GL_UNSIGNED_INT, 0);
          glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        }

        // deactivate vertex array
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);

        //glClientActiveTextureARB(GL_TEXTURE0);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);

        //glClientActiveTextureARB(GL_TEXTURE1);
        //glDisableClientState(GL_TEXTURE_COORD_ARRAY);

        glDisableVertexAttribArray(7);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      }
    }

    void MarsVBOGeom::initGeom() {

      VertexData* vData;
      GLuint *indices;
      unsigned long numVertices, numIndices;

      numVertices = vertices.size();
      numIndices = vertices.size();

      vData = (VertexData*)malloc(numVertices*sizeof(VertexData));
      if(!vData) {
        return;
      }
      indices = (GLuint*)calloc(numIndices, sizeof(GLuint));
      if(!indices) {
        free(vData);
        return;
      }

      indicesToDraw = numIndices;

      for(unsigned long i=0; i<numVertices; ++i) {
        vData[i].position[0] = vertices[i][0];
        vData[i].position[1] = vertices[i][1];
        vData[i].position[2] = vertices[i][2];
        if(haveTexcoords) {
          vData[i].texCoord[0] = texcoords[i][0];
          vData[i].texCoord[1] = texcoords[i][1];
        }
        else {
          vData[i].texCoord[0] = vertices[i][0];
          vData[i].texCoord[1] = vertices[i][1];
        }

        vData[i].normal[0] = normals[i][0];
        vData[i].normal[1] = normals[i][1];
        vData[i].normal[2] = normals[i][2];
        // todo: calculate usefull tangent
        vData[i].tangent[0] = 0.0;
        vData[i].tangent[1] = 1.0;
        vData[i].tangent[2] = 0.0;
        indices[i] = i;
      }

      glBindBuffer(GL_ARRAY_BUFFER, vboIds[0]);
      glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(VertexData), vData,
                   GL_STATIC_DRAW);

      // Transfer index data to VBO 1
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[1]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(GLuint), indices,
                   GL_STATIC_DRAW);

    }

    void MarsVBOGeom::normalize(float *v) {
      double length = sqrt(v[0]*v[0]+ v[1]*v[1]+ v[2]*v[2]);
      v[0] /= length;
      v[1] /= length;
      v[2] /= length;
    }

    void MarsVBOGeom::drawImplementation(osg::RenderInfo& renderInfo) const{
      osg::State& state = *renderInfo.getState();
      state.disableAllVertexArrays();
      osg::ArrayDispatchers& arrayDispatchers = state.getArrayDispatchers();

      arrayDispatchers.reset();
      //arrayDispatchers.dispatch(osg::Geometry::BIND_OVERALL,0);

      state.lazyDisablingOfVertexAttributes();
      state.applyDisablingOfVertexAttributes();

      MarsVBOGeom *da = const_cast<MarsVBOGeom*>(this);
      da->render();
    }

    osg::BoundingBox MarsVBOGeom::computeBound() const {
      if(vertices.size() == 0)
        return osg::BoundingBox(-0.1, -0.1, -0.1, 0.1, 0.1, 0.1);

      double minx = vertices[0][0];
      double miny = vertices[0][1];
      double minz = vertices[0][2];
      double maxx = minx;
      double maxy = miny;
      double maxz = minz;

      for(unsigned long i=1; i<vertices.size(); ++i) {
        if(minx > vertices[i][0]) minx = vertices[i][0];
        if(maxx < vertices[i][0]) maxx = vertices[i][0];
        if(miny > vertices[i][1]) miny = vertices[i][1];
        if(maxy < vertices[i][1]) maxy = vertices[i][1];
        if(minz > vertices[i][2]) minz = vertices[i][2];
        if(maxz < vertices[i][2]) maxz = vertices[i][2];
      }
      return osg::BoundingBox(minx, miny, minz, maxx, maxy, maxz);
    }

  } // namespace graphics
} // namespace mars

#endif
