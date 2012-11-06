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

#ifdef USE_VERTEX_BUFFER

/* The prototypes are only declared in GL/glext.h if this is defined */
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1 //for glGenBuffers, glBindBuffer,
                              //glDeleteBuffers
#endif

#include "MultiResHeightMapRenderer.h"
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <cstring>
#include <stdio.h>

namespace mars {

  inline double sqr(double x) {
    return x*x;
  }

  struct VertexData {
    float position[3];
    float normal[3];
    float tangent[4];
    float texCoord[2];
  };

  MultiResHeightMapRenderer::MultiResHeightMapRenderer(int gridW, int gridH,
                                                       double visualW,
                                                       double visualH,
                                                       double scaleX,
                                                       double scaleY,
                                                       double scaleZ,
                                                       double texScaleX,
                                                       double texScaleY)
    : vboIds(new GLuint[4]), isInitialized(false), highIsInitialized(false),
      targetWidth(visualW), targetHeight(visualH), width(gridW), height(gridH),
      scaleX(scaleX), scaleY(scaleY), scaleZ(scaleZ), texScaleX(texScaleX),
      texScaleY(texScaleY) {

    maxNumSubTiles = 100;
    heightData = NULL;
    numSubTiles = 0;
    prepare();

    for(int i = 0; i < 3; ++i)
      offset[i] = 0.;

    dirty = true;
    wireframe = false;
    highWireframe = false;
    solid = true;
    highSolid = true;
  }

  MultiResHeightMapRenderer::~MultiResHeightMapRenderer() {
    clear();
    delete[] vboIds;
    vboIds = NULL;
  }

  void MultiResHeightMapRenderer::initialize() {
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
    if(!vboIds[0] || !vboIds[1]) {
      fprintf(stderr, "MultiResHeightMapRenderer::initialize error while generating buffers\n");
    }
    // Initializes geometry and transfers it to VBOs
    isInitialized = initPlane(false);
  }

  void MultiResHeightMapRenderer::highInitialize() {
    if(!isInitialized) return;

    // Generate 2 VBOs
    glGenBuffers(2, vboIds+2);
    if(!vboIds[2] || !vboIds[3]) {
      fprintf(stderr, "MultiResHeightMapRenderer::highInitialize error while generating buffers\n");
    }
    // Initializes cube geometry and transfers it to VBOs
    highIsInitialized = initPlane(true);
  }


  void MultiResHeightMapRenderer::prepare() {
    recalcSteps();
    heightData = new double*[getLowResVertexCntY()];
    for(int y = 0; y < getLowResVertexCntY(); ++y) {
      heightData[y] = new double[getLowResVertexCntX()];
      for(int x = 0; x < getLowResVertexCntX(); ++x) {
        heightData[y][x] = -1;
      }
    }
    numVertices = getLowResVertexCntX()*getLowResVertexCntY();
    highNumVertices = maxNumSubTiles*getHighResVertexCntX()*getHighResVertexCntY();
    numIndices = getLowResCellCntX()*getLowResCellCntY()*6;
    highNumIndices = maxNumSubTiles*getHighResCellCntX()*getHighResCellCntY()*6;
    indicesToDraw = getLowResCellCntX()*getLowResCellCntY()*6;
    highIndicesToDraw = 0;
    newIndicesPos = 0;//(getLowResCellCntY())*(getLowResCellCntX())*6;
    newVerticesPos = 0;//getLowResVertexCntY()*getLowResVertexCntX();
  }

  void MultiResHeightMapRenderer::clear() {

    if(isInitialized) {
      free (vertices);
      vertices = NULL;
      free (indices);
      indices = NULL;
      glBindBuffer(GL_ARRAY_BUFFER, vboIds[0]);
      glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[1]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);
      glDeleteBuffers(2, vboIds);
    }

    if(highIsInitialized) {
      free (highVertices);
      highVertices = NULL;
      free (highIndices);
      highIndices = NULL;
      glBindBuffer(GL_ARRAY_BUFFER, vboIds[2]);
      glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[3]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);
      glDeleteBuffers(2, vboIds+2);
    }

    while(!listSubTiles.empty()) {
      SubTile *toRemove = listSubTiles.front();
      listSubTiles.pop_front();
      // this is probably wrong. because the lastTile is not updated. comment out for now
      //copyLast(toRemove->indicesArrayOffset, toRemove->verticesArrayOffset);
      subTiles.erase(toRemove->mapIndex);
      if(toRemove->heightData) {
        for(int i = 0; i < getHighResVertexCntY(); ++i) {
          delete toRemove->heightData[i];
        }
        delete toRemove->heightData;
      }
      delete toRemove;
      --numSubTiles;
      newIndicesPos -= getHighResCellCntX() * getHighResCellCntY() * 6;
      newVerticesPos -= getHighResVertexCntX() * getHighResVertexCntY();
    }
    if(heightData) {
      for(int i = 0; i < getLowResVertexCntY(); ++i) {
        delete[] heightData[i];
      }
      delete[] heightData;
      heightData = NULL;
    }
  }

  void MultiResHeightMapRenderer::recalcSteps() {
    highStepX = stepX = targetWidth / double(getLowResCellCntX());
    highStepY = stepY = targetHeight / double(getLowResCellCntY());

    highWidth = highHeight = 1;
    double radius = 0.05;
    double desiredStep = radius * 0.2;

    while(highStepX > desiredStep) {
      highStepX *= 0.5;
    }
    while(highStepY > desiredStep) {
      highStepY *= 0.5;
    }

    highWidth = stepX / highStepX;
    highHeight = stepY / highStepY;

    dirty = true;
  }

  void MultiResHeightMapRenderer::render() {

    if(!isInitialized) initialize();

    if(dirty) {
      glBindBuffer(GL_ARRAY_BUFFER, vboIds[0]);
      VertexData *vertices = (VertexData*)glMapBuffer(GL_ARRAY_BUFFER,
                                                      GL_WRITE_ONLY);
      int index;
      for(int y = 0; y < getLowResVertexCntY(); ++y) {
        for(int x = 0; x < getLowResVertexCntX(); ++x) {
          index = y*getLowResVertexCntX() + x;
          vertices[index].position[0] = x * stepX * scaleX;
          vertices[index].position[1] = y * stepY * scaleY;
          vertices[index].position[2] = heightData[y][x] * scaleZ;
          vertices[index].texCoord[0] = x * stepX * scaleX*texScaleX;
          vertices[index].texCoord[1] = y * stepY * scaleY*texScaleY;
          getNormal(x, y, getLowResVertexCntX(), getLowResVertexCntY(),
                    stepX, stepY, heightData,
                    vertices[index].normal,
                    vertices[index].tangent, true);
        }
      }
      glUnmapBuffer(GL_ARRAY_BUFFER);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      dirty = false;
    }

    // handle new footprints
    while(!footPrints.empty()) {
      FootPrint handlePrint = footPrints.front();
      footPrints.pop_front();
      collideSphereI(handlePrint.x, handlePrint.y,
                     handlePrint.z, handlePrint.r);
    }

    render(false);

    if(highIsInitialized && highIndicesToDraw) {
      render(true);
    }
  }

  bool MultiResHeightMapRenderer::initPlane(bool highRes) {

    VertexData* vertices;
    GLuint *indices;
    int numVertices, numIndices;
    int vboId;
    int height, width;

    if(highRes) {
      numVertices = highNumVertices;
      numIndices = highNumIndices;
      vboId = 2;
      height = getHighResVertexCntY();
      width = getHighResVertexCntX();
    }
    else {
      numVertices = this->numVertices;
      numIndices = this->numIndices;
      vboId = 0;
      height = getLowResVertexCntY();
      width = getLowResVertexCntX();
    }

    vertices = (VertexData*)malloc(numVertices*sizeof(VertexData));
    if(!vertices) {
      fprintf(stderr, "MultiResHeightMapRenderer::initPlane error while allocating memory for vertices %lu\n", numVertices);
      return false;
    }
    for(int y = 0; y < height; ++y) {
      for(int x = 0; x < width; ++x) {
        int index = y*width+x;
        vertices[index].position[0] = x * stepX * scaleX;
        vertices[index].position[1] = y * stepY * scaleY;
        vertices[index].position[2] = heightData[y][x] * scaleZ;
        vertices[index].texCoord[0] = x * stepX * scaleX*texScaleX;
        vertices[index].texCoord[1] = y * stepY * scaleY*texScaleY;

        vertices[index].normal[0] = 0.0;
        vertices[index].normal[1] = 0.0;
        vertices[index].normal[2] = 1.0;
        vertices[index].tangent[0] = 0.0;
        vertices[index].tangent[1] = 1.0;
        vertices[index].tangent[2] = 0.0;
      }
    }

    indices = (GLuint*)calloc(numIndices, sizeof(GLuint));
    if(!indices) {
      fprintf(stderr, "MultiResHeightMapRenderer::initPlane error while allocating memory for indices %lu\n", numIndices);
      free(vertices);
      return false;
    }
    for(int y=0; y<height-1; ++y) {
      for(int x=0; x<width-1; ++x) {
        int indexOffset = y*(width-1)*6+x*6;
        indices[indexOffset+0] = (y+1) * width + x;
        indices[indexOffset+1] =  y    * width + x;
        indices[indexOffset+2] = (y+1) * width + x+1;

        indices[indexOffset+3] = (y+1) * width + x+1;
        indices[indexOffset+4] =  y    * width + x;
        indices[indexOffset+5] =  y    * width + x+1;
      }
    }

    glBindBuffer(GL_ARRAY_BUFFER, vboIds[vboId]);
    glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(VertexData), vertices,
                 GL_DYNAMIC_DRAW);

    // Transfer index data to VBO 1
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[vboId+1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(GLuint), indices,
                 GL_DYNAMIC_DRAW);

    if(highRes) {
      highVertices = vertices;
      highIndices = indices;
    }
    else {
      this->vertices = vertices;
      this->indices = indices;
    }
    return true;
  }


  void MultiResHeightMapRenderer::cutHole(int x, int y) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[1]);
    GLuint *indices = (GLuint*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER,
                                           GL_WRITE_ONLY);
    if(indices) {
      for(int i = 0; i < 6; ++i)
        indices[y*getLowResCellCntX()*6+x*6+i] = 0;
    }
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  void MultiResHeightMapRenderer::fillOriginal(int x, int y) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[1]);
    GLuint *indices = (GLuint*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER,
                                           GL_WRITE_ONLY);
    if(indices) {
      int indexOffset = y*getLowResCellCntX()*6+x*6;
      indices[indexOffset+0] = (y+1) * getLowResVertexCntX() + x;
      indices[indexOffset+1] =  y    * getLowResVertexCntX() + x;
      indices[indexOffset+2] = (y+1) * getLowResVertexCntX() + x+1;

      indices[indexOffset+3] = (y+1) * getLowResVertexCntX() + x+1;
      indices[indexOffset+4] =  y    * getLowResVertexCntX() + x;
      indices[indexOffset+5] =  y    * getLowResVertexCntX() + x+1;
    }
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // check for subtile neighbours
    std::map<int, SubTile*>::iterator it;
    double val;
    if(y > 0) {
      it = subTiles.find((y-1)*(getLowResCellCntX())+x);
      if(it != subTiles.end()) {
        for(int x1 = 0; x1 < getHighResVertexCntX(); ++x1) {
          val = getHeight(x1, getHighResCellCntY(), it->second);
          it->second->heightData[getHighResCellCntY()][x1] = val;
        }
        drawSubTile(it->second);
      }
    }

    if(y > 0 && x > 0) {
      it = subTiles.find((y-1)*getLowResCellCntX()+x-1);
      if(it != subTiles.end()) {
        val = getHeight(getHighResCellCntX(), getHighResCellCntY(), it->second);
        it->second->heightData[getHighResCellCntY()][getHighResCellCntX()] = val;
        drawSubTile(it->second);
      }
    }

    if(x > 0) {
      it = subTiles.find(y*getLowResCellCntX()+x-1);
      if(it != subTiles.end()) {
        for(int y1 = 0; y1 < getHighResVertexCntY(); ++y1) {
          val = getHeight(getHighResCellCntX(), y1, it->second);
          it->second->heightData[y1][getHighResCellCntX()] = val;
        }
        drawSubTile(it->second);
      }
    }

    if(y < getLowResCellCntY()) {
      it = subTiles.find((y+1)*(getLowResCellCntX())+x);
      if(it != subTiles.end()) {
        for(int x1 = 0; x1 < getHighResVertexCntX(); ++x1) {
          it->second->heightData[0][x1] = getHeight(x1, 0, it->second);
        }
        drawSubTile(it->second);
      }
    }

    if(y < getLowResCellCntY() && x < getLowResCellCntX()) {
      it = subTiles.find((y+1)*(getLowResCellCntX())+x+1);
      if(it != subTiles.end()) {
        it->second->heightData[0][0] = getHeight(0, 0, it->second);
        drawSubTile(it->second);
      }
    }

    if(x < getLowResCellCntX()) {
      it = subTiles.find((y)*(getLowResCellCntX())+x+1);
      if(it != subTiles.end()) {
        for(int y1 = 0; y1 < getHighResVertexCntY(); ++y1)
          it->second->heightData[y1][0] = getHeight(0, y1, it->second);
        drawSubTile(it->second);
      }
    }

    if(y > 0 && x < getLowResCellCntX()) {
      it = subTiles.find((y-1)*(getLowResCellCntX())+x+1);
      if(it != subTiles.end()) {
        val = getHeight(0, getHighResCellCntY(), it->second);
        it->second->heightData[getHighResCellCntY()][0] = val;
        drawSubTile(it->second);
      }
    }

    if(y < getLowResCellCntY() && x > 0) {
      it = subTiles.find((y+1)*(getLowResCellCntX())+x-1);
      if(it != subTiles.end()) {
        val = getHeight(getHighResCellCntX(), 0, it->second);
        it->second->heightData[0][getHighResCellCntX()] = val;
        drawSubTile(it->second);
      }
    }
  }


  /*
    void MultiResHeightMapRenderer::addCell(int x, int y) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[1]);
    GLuint *indices = (GLuint*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER,
    GL_WRITE_ONLY);
    if(indices) {
    indices[y*(getLowResCellCntX())*6+x*6+0] = y * getLowResVertexCntX() + x;
    indices[y*(getLowResCellCntX())*6+x*6+1] = (y+1) * getLowResVertexCntX() + x;
    indices[y*(getLowResCellCntX())*6+x*6+2] = (y+1) * getLowResVertexCntX()+ x+1;

    indices[y*(getLowResCellCntX())*6+x*6+3] = y * getLowResVertexCntX() + x;
    indices[y*(getLowResCellCntX())*6+x*6+4] = (y+1) * getLowResVertexCntX() + x+1;
    indices[y*(getLowResCellCntX())*6+x*6+5] = y * getLowResVertexCntX() + x+1;
    }
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
  */

  double MultiResHeightMapRenderer::interpolateCell(int gridX, int gridY,
                                                    double x, double y) {
    double dx = (x - gridX*stepX) / stepX;
    double dy = (y - gridY*stepY) / stepY;
    double cornerHeights[4];
    assert((gridY < getLowResCellCntY()) && (gridX < getLowResCellCntX()));
    cornerHeights[0] = heightData[gridY][gridX];
    cornerHeights[1] = heightData[gridY+1][gridX];
    cornerHeights[2] = heightData[gridY][gridX+1];
    cornerHeights[3] = heightData[gridY+1][gridX+1];
    double height = (cornerHeights[0] * (1-dx) * (1-dy) +
                     cornerHeights[1] * (1-dx) * dy +
                     cornerHeights[2] * dx * (1-dy) +
                     cornerHeights[3] * dx * dy);
    return height;
  }

  void MultiResHeightMapRenderer::fillCell(SubTile *tile) {

    // use highResBuffer
    glBindBuffer(GL_ARRAY_BUFFER, vboIds[2]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[3]);
    VertexData *vertices = (VertexData*)glMapBuffer(GL_ARRAY_BUFFER,
                                                    GL_WRITE_ONLY);

    GLuint *indices = (GLuint*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER,
                                           GL_WRITE_ONLY);

    if(vertices) {
      int x2 = tile->verticesArrayOffset;
      int index;

      for(int iy = 0; iy < getHighResVertexCntY(); ++iy) {
        for(int ix = 0; ix < getHighResVertexCntX(); ++ix) {
          index = x2 + iy*getHighResVertexCntX() + ix;
          double x = tile->x*stepX + ix*highStepX;
          double y = tile->y*stepY + iy*highStepY;
          double z = interpolateCell(tile->x, tile->y, x, y);
          tile->heightData[iy][ix] = z;
          vertices[index].position[0] = x * scaleX;
          vertices[index].position[1] = y * scaleY;
          vertices[index].position[2] = z * scaleZ;
          vertices[index].texCoord[0] = x * scaleX*texScaleX;
          vertices[index].texCoord[1] = y * scaleY*texScaleY;
        }
      }
      // calculate normals in a separate loop because the calculation depends on the neighbours
      for(int iy = 0; iy < getHighResVertexCntY(); ++iy) {
        for(int ix = 0; ix < getHighResVertexCntX(); ++ix) {
          index = x2 + iy*getHighResVertexCntX() + ix;
          getNormal(ix, iy, getHighResCellCntX(), getHighResCellCntY(),
                    highStepX, highStepY,
                    tile->heightData,
                    vertices[index].normal,
                    vertices[index].tangent, true);
        }
      }
    }

    if(indices) {
      int offset = tile->indicesArrayOffset;
      int x2 = tile->verticesArrayOffset;

      for(int iy = 0; iy < getHighResCellCntY(); ++iy) {
        for(int ix = 0; ix < getHighResCellCntX(); ++ix) {
          int indexOffset = offset+iy*getHighResCellCntX()*6+ix*6;
          indices[indexOffset+0] = x2+((iy+1)*getHighResVertexCntX())+ix;
          indices[indexOffset+1] = x2+( iy   *getHighResVertexCntX())+ix;
          indices[indexOffset+2] = x2+((iy+1)*getHighResVertexCntX())+ix+1;

          indices[indexOffset+3] = x2+((iy+1)*getHighResVertexCntX())+ix+1;
          indices[indexOffset+4] = x2+( iy   *getHighResVertexCntX())+ix;
          indices[indexOffset+5] = x2+( iy   *getHighResVertexCntX())+ix+1;
        }
      }
    }
    highIndicesToDraw += getHighResCellCntY()*getHighResCellCntX()*6;

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  void MultiResHeightMapRenderer::collideSphere(double xPos, double yPos,
                                                double zPos, double radius) {
    FootPrint newFootPrint = {xPos, yPos, zPos, radius};
    footPrints.push_back(newFootPrint);
  }

  void MultiResHeightMapRenderer::collideSphereI(double xPos, double yPos,
                                                 double zPos, double radius) {

    if(!isInitialized) {
      return;
    }

    // first check if we are within the current heightmap
    if((xPos < 0 || yPos < 0) ||
       (xPos > targetWidth || yPos > targetHeight)) {
      return;
    }

    if(!highIsInitialized) highInitialize();
    if(!highIsInitialized) return;

    std::list<SubTile*> toProcess;
    std::list<SubTile*>::iterator iter;

    int x1 = floor((xPos-radius*1.2) / stepX);
    int y1 = floor((yPos-radius*1.2) / stepY);
    int x2 = ceil((xPos+radius*1.2) / stepX);
    int y2 = ceil((yPos+radius*1.2) / stepY);

    if(y1 < 0) y1 = 0;
    if(x1 < 0) x1 = 0;
    if(y2 > getLowResCellCntY()) y2 = getLowResCellCntY();
    if(x2 > getLowResCellCntX()) x2 = getLowResCellCntX();

    std::map<int, SubTile*>::iterator it;

    // code to remove a SubTile without replacing it
    if(0) {
      SubTile *toRemove = listSubTiles.front();
      listSubTiles.pop_front();
      numSubTiles--;
      highIndicesToDraw -= getHighResCellCntY()*getHighResCellCntX()*6;
      copyLast(toRemove->indicesArrayOffset, toRemove->verticesArrayOffset);
      subTiles.erase(toRemove->mapIndex);
      fillOriginal(toRemove->x, toRemove->y);

      // free memory
      for(int l = 0; l < getHighResVertexCntY(); ++l)
        delete[] toRemove->heightData[l];
      delete[] toRemove->heightData;
      delete toRemove;
    }

    for(int iy = y1; iy < y2; ++iy) {
      for(int ix = x1; ix < x2; ++ix) {
        it = subTiles.find(iy*(getLowResCellCntX())+ix);
        if(it != subTiles.end()) {
          toProcess.push_back(it->second);
        } else {
          if(numSubTiles >= maxNumSubTiles) {
            SubTile *toRemove = listSubTiles.front();
            listSubTiles.pop_front();
            numSubTiles--;
            highIndicesToDraw -= getHighResCellCntY()*getHighResCellCntX()*6;
            newIndicesPos = toRemove->indicesArrayOffset;
            newVerticesPos = toRemove->verticesArrayOffset;
            subTiles.erase(toRemove->mapIndex);
            fillOriginal(toRemove->x, toRemove->y);

            // free memory
            for(int l = 0; l < getHighResVertexCntY(); ++l)
              delete[] toRemove->heightData[l];
            delete[] toRemove->heightData;
            delete toRemove;
          }
          cutHole(ix, iy);
          SubTile *newSubTile = new SubTile;
          newSubTile->x = ix;
          newSubTile->y = iy;
          newSubTile->xPos = ix*stepX;
          newSubTile->yPos = iy*stepY;
          newSubTile->indicesArrayOffset = newIndicesPos;
          newSubTile->verticesArrayOffset = newVerticesPos;
          newIndicesPos += getHighResCellCntX()*getHighResCellCntY()*6;
          newVerticesPos += getHighResVertexCntX()*getHighResVertexCntY();
          newSubTile->heightData = new double*[getHighResVertexCntY()];
          for(int l = 0; l < getHighResVertexCntY(); ++l) {
            newSubTile->heightData[l] = new double[getHighResVertexCntX()];
          }
          for(int n = 0; n < getHighResVertexCntY(); ++n)
            for(int m = 0; m < getHighResVertexCntX(); ++m)
              // TODO: Should we interpolate here?
              newSubTile->heightData[n][m] = heightData[iy][ix];

          fillCell(newSubTile);

          newSubTile->mapIndex = iy*(getLowResCellCntX())+ix;
          subTiles[newSubTile->mapIndex] = newSubTile;
          listSubTiles.push_back(newSubTile);
          toProcess.push_back(newSubTile);
          numSubTiles++;
        }
      }
    }

    std::list<SubTile*>::iterator listIter;
    bool found;
    for(iter=toProcess.begin(); iter!=toProcess.end(); ++iter) {
      found = false;
      for(listIter=listSubTiles.begin(); listIter!=listSubTiles.end();
          ++listIter) {
        if(*listIter == *iter) {
          listSubTiles.erase(listIter);
          listSubTiles.push_back(*iter);
          found = true;
          break;
        }
      }
      if(found)
        adaptSubTile(*iter, xPos, yPos, zPos, radius);
    }
  }

  void MultiResHeightMapRenderer::copyLast(int indicesOffsetPos,
                                           int verticesOffsetPos) {
    glBindBuffer(GL_ARRAY_BUFFER, vboIds[2]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[3]);
    VertexData *vertices = (VertexData*)glMapBuffer(GL_ARRAY_BUFFER,
                                                    GL_WRITE_ONLY);

    GLuint *indices = (GLuint*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER,
                                           GL_WRITE_ONLY);

    SubTile* lastSubTile = listSubTiles.back();

    if(vertices) {
      int offset = verticesOffsetPos;
      int offset2 = lastSubTile->verticesArrayOffset;

      //            memcpy(vertices+offset, vertices+offset2, getHighResVertexCntY()*getHighResVertexCntX());
      memcpy(vertices+offset, vertices+offset2, getHighResVertexCntY()*getHighResVertexCntX()*sizeof(VertexData));
    }
    if(indices) {
      int offset = indicesOffsetPos;
      int offset2 = lastSubTile->indicesArrayOffset;

      //            memcpy(indices+offset, indices+offset2, getHighResCellCntY()*getHighResCellCntX()*6);
      memcpy(indices+offset, indices+offset2, getHighResCellCntY()*getHighResCellCntX()*6*sizeof(GLuint));
    }

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

    newIndicesPos = lastSubTile->indicesArrayOffset;
    newVerticesPos = lastSubTile->verticesArrayOffset;
    lastSubTile->indicesArrayOffset = indicesOffsetPos;
    lastSubTile->verticesArrayOffset = verticesOffsetPos;

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  void MultiResHeightMapRenderer::adaptSubTile(SubTile* tile, double xPos,
                                               double yPos, double zPos,
                                               double radius) {

    //if(zPos < heightData[tile->x][tile->y]) radius += fabs(zPos-heightData[tile->x][tile->y]) * 0.2;

    int x1 = floor((xPos - tile->xPos - radius) / highStepX)-2;
    int y1 = floor((yPos - tile->yPos - radius) / highStepY)-2;
    int x2 = ceil((xPos - tile->xPos + radius) / highStepX)+2;
    int y2 = ceil((yPos - tile->yPos + radius) / highStepY)+2;
    if(x1 < 0) x1 = 0;
    if(y1 < 0) y1 = 0;
    if(x2 > getHighResVertexCntX()) x2 = getHighResVertexCntX();
    if(y2 > getHighResVertexCntY()) y2 = getHighResVertexCntY();
    double r2 = radius*radius;
    bool adapt = false;
    double vx, vy, vz;
    double vx2, vy2, vz2;
    //double interpolation = 4.0;

    for(int y=y1; y<y2; ++y) {
      for(int x=x1; x<x2; ++x) {
        vx = tile->xPos + x*highStepX;
        vy = tile->yPos + y*highStepY;
        vz = tile->heightData[y][x];

        vx2 = vx - xPos;
        vy2 = vy - yPos;

        if(vx2*vx2+vy2*vy2 <= r2) {
          vz2 = intersectReplacementSphere(xPos, yPos, zPos, radius,
                                           vx, vy, vz);
          if(vz2 < vz-0.001) {
            adapt = true;
            tile->heightData[y][x] = vz2;
          }
        }
        /*
          if(adapt && 0) {
          if(y > 0 && (tile->heightData[y][x] <
          tile->heightData[y-1][x]-highStepY*interpolation)) {
          tile->heightData[y-1][x] = tile->heightData[y][x]+highStepY*interpolation;
          }

          if(x > 0 && (tile->heightData[y][x] <
          tile->heightData[y][x-1]-highStepX*interpolation)) {
          tile->heightData[y][x-1] = tile->heightData[y][x]+highStepX*interpolation;
          }

          if(y < getHighResCellCntY() && (tile->heightData[y][x] <
          tile->heightData[y+1][x]-highStepY*interpolation)) {
          tile->heightData[y+1][x] = tile->heightData[y][x]+highStepY*interpolation;
          }

          if(x < getHighResCellCntX() && (tile->heightData[y][x] <
          tile->heightData[y][x+1]-highStepX*interpolation)) {
          tile->heightData[y][x+1] = tile->heightData[y][x]+highStepX*interpolation;
          }

          // the corners
          if((y > 0) && (x > 0) &&
          (tile->heightData[y][x] < tile->heightData[y-1][x-1]-(highStepY+highStepX)*interpolation*0.5)) {
          tile->heightData[y-1][x-1] = tile->heightData[y][x]+(highStepY+highStepX)*interpolation*0.5;
          }

          if((y > 0) && (x < getHighResCellCntX()) &&
          (tile->heightData[y][x] < tile->heightData[y-1][x+1]-(highStepY+highStepX)*interpolation*0.5)) {
          tile->heightData[y-1][x+1] = tile->heightData[y][x]+(highStepY+highStepX)*interpolation*0.5;
          }

          if((y < getHighResCellCntY()) && (x > 0) &&
          (tile->heightData[y][x] < tile->heightData[y+1][x-1]-(highStepY+highStepX)*interpolation*0.5)) {
          tile->heightData[y+1][x-1] = tile->heightData[y][x]+(highStepY+highStepX)*interpolation*0.5;
          }

          if((y < getHighResCellCntY()) && (x < getHighResCellCntX()) &&
          (tile->heightData[y][x] < tile->heightData[y+1][x+1]-(highStepY+highStepX)*interpolation*0.5)) {
          tile->heightData[y+1][x+1] = tile->heightData[y][x]+(highStepY+highStepX)*interpolation*0.5;
          }
          }
        */
      }
    }

    if(adapt) {
      glBindBuffer(GL_ARRAY_BUFFER, vboIds[2]);
      VertexData *vertices = (VertexData*)glMapBuffer(GL_ARRAY_BUFFER,
                                                      GL_WRITE_ONLY);
      if(!vertices) {
        fprintf(stderr, "MultiResHeightMapRenderer::adaptSubtile glMapBuffer returned 0\n");
        return;
      }
      VertexData* v;
      int index;
      for(int y=y1; y<y2; ++y) {
        for(int x=x1; x<x2; ++x) {
          index = tile->verticesArrayOffset+y*getHighResVertexCntX()+x;
          v = vertices+index;
          v->position[2] = tile->heightData[y][x] * scaleZ;
          
          getNormal(x, y, getHighResCellCntX(), getHighResCellCntY(),
                    highStepX, highStepY,
                    tile->heightData,
                    vertices[index].normal,
                    vertices[index].tangent, true);
          
        }
      }
      glUnmapBuffer(GL_ARRAY_BUFFER);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
  }

  double MultiResHeightMapRenderer::intersectReplacementSphere(double x,
                                                               double y,
                                                               double z,
                                                               double radius,
                                                               double vx,
                                                               double vy,
                                                               double vz) {
    (void) vz;
    return z - sqrt(sqr(radius)-sqr(x-vx)-sqr(y-vy));
  }

  void MultiResHeightMapRenderer::setHeight(unsigned int gridX,
                                            unsigned int gridY,
                                            double height) {
    assert(gridX < (unsigned int)getLowResVertexCntX());
    assert(gridY < (unsigned int)this->getLowResVertexCntY());
    if(height < minZ)
      minZ = height;
    if(height > maxZ)
      maxZ = height;
    heightData[gridY][gridX] = height;
    dirty = true;
  }

  double MultiResHeightMapRenderer::getHeight(unsigned int gridX,
                                              unsigned int gridY) {
    return heightData[gridY][gridX];
  }

  void MultiResHeightMapRenderer::setOffset(double x, double y, double z) {
    if((x == offset[0]) && (y == offset[1]) && (z == offset[2]))
      return;
    offset[0] = x;
    offset[1] = y;
    offset[2] = z;
  }

  void MultiResHeightMapRenderer::getNormal(int x, int y, int mx, int my,
                                            double x_step, double y_step,
                                            double **height_data,
                                            float *normal,
                                            float *tangent,
                                            bool skipBorder) {
    double vx1, vz1, vy2, vz2;

    if(skipBorder) {
      if(x < mx-2 && x > 1) {
        vz1 = height_data[y][x+1] - height_data[y][x-1];
        vx1 = x_step*2.0;
      }
      else if(x==0) {
        vz1 = height_data[y][x+2] - height_data[y][x+1];
        vx1 = x_step;
      }
      else if(x==1) {
        vz1 = height_data[y][x+1] - height_data[y][x];
        vx1 = x_step;
      }
      else if(x==mx-1) {
        vz1 = height_data[y][x-1] - height_data[y][x-2];
        vx1 = x_step;
      }
      else {
        vz1 = height_data[y][x] - height_data[y][x-1];
        vx1 = x_step;
      }

      if(y > 1 && y < my-2) {
        vz2 = height_data[y+1][x] - height_data[y-1][x];
        vy2 = y_step*2.0;
      }
      else if(y==0) {
        vz2 = height_data[y+2][x] - height_data[y+1][x];
        vy2 = y_step;
      }
      else if(y==1) {
        vz2 = height_data[y+1][x] - height_data[y][x];
        vy2 = y_step;
      }
      else if(y==my-1) {
        vz2 = height_data[y-1][x] - height_data[y-2][x];
        vy2 = y_step;
      }
      else {
        vz2 = height_data[y][x] - height_data[y-1][x];
        vy2 = y_step;
      }
    }
    else {
      if(x != 0 && x != mx-1) {
        vz1 = height_data[y][x+1] - height_data[y][x-1];
        vx1 = x_step*2.0;
      }
      else if(x==0) {
        vz1 = height_data[y][x+1] - height_data[y][x];
        vx1 = x_step;
      }
      else {
        vz1 = height_data[y][x] - height_data[y][x-1];
        vx1 = x_step;
      }

      if(y != 0 && y != my-1) {
        vz2 = height_data[y+1][x] - height_data[y-1][x];
        vy2 = y_step*2.0;
      }
      else if(y==0) {
        vz2 = height_data[y+1][x] - height_data[y][x];
        vy2 = y_step;
      }
      else {
        vz2 = height_data[y][x] - height_data[y-1][x];
        vy2 = y_step;
      }
    }

    vz1 *= scaleZ;
    vx1 *= scaleX;
    vz2 *= scaleZ;
    vy2 *= scaleY;

    normal[0] = -vz1*vy2;
    normal[1] = -vz2*vx1;
    normal[2] = vx1*vy2;
    normalize(normal);
    tangent[0] = vx1;
    tangent[1] = vy2;
    tangent[2] = vz1 + vz2;
    tangent[3] = 0.0;
    normalize(tangent);
  }

  void MultiResHeightMapRenderer::normalize(float *v) {
    float a = v[0],b = v[1],c = v[2];
    //    float length = 1.0f/sqrtf(v[0]*v[0]+ v[1]*v[1]+ v[2]*v[2]);
    float length = 1.0f/sqrtf(a*a+b*b+c*c);
    *(v++) *= length;
    *(v++) *= length;
    *v *= length;
  }

  double MultiResHeightMapRenderer::getHeight(int x, int y, SubTile *tile) {
    double x1, y1;
    x1 = tile->x*stepX + x*highStepX;
    y1 = tile->y*stepY + y*highStepY;
    return interpolateCell(tile->x, tile->y, x1, y1);
  }

  void MultiResHeightMapRenderer::drawSubTile(SubTile *tile) {
    glBindBuffer(GL_ARRAY_BUFFER, vboIds[2]);
    VertexData *vertices = (VertexData*)glMapBuffer(GL_ARRAY_BUFFER,
                                                    GL_WRITE_ONLY);
    VertexData* v;
    int index;

    for(int y = 0; y < getHighResVertexCntY(); ++y) {
      for(int x = 0; x < getHighResVertexCntX(); ++x) {
        index = tile->verticesArrayOffset+y*getHighResVertexCntX()+x;
        v = vertices+index;
        v->position[2] = tile->heightData[y][x] * scaleZ;
        getNormal(x, y, getHighResCellCntX(), getHighResCellCntY(), highStepX, highStepY,
                  tile->heightData,
                  vertices[index].normal,
                  vertices[index].tangent, true);
      }
    }
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }


  void MultiResHeightMapRenderer::render(bool highRes) {

    int indicesToDraw;

    // Tell OpenGL which VBOs to use
    if(highRes) {
      glBindBuffer(GL_ARRAY_BUFFER, vboIds[2]);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[3]);
      indicesToDraw = highIndicesToDraw;
    }
    else {
      glBindBuffer(GL_ARRAY_BUFFER, vboIds[0]);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[1]);
      indicesToDraw = this->indicesToDraw;
    }

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

    if((!highRes && solid) || (highRes && highSolid)) {
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
      glDrawElements(GL_TRIANGLES, indicesToDraw, GL_UNSIGNED_INT, 0);
    }

    //GLfloat mat_diffuse[] = { 1.0, 0.0, 0.0, 1.0 };
    //glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);

    if((!highRes && wireframe)  || (highRes && highWireframe)) {
      glLineWidth(2.0);

      glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
      glDrawElements(GL_TRIANGLES, indicesToDraw, GL_UNSIGNED_INT, 0);
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    }

    glDisableClientState(GL_VERTEX_ARRAY);            // deactivate vertex array
    glDisableClientState(GL_NORMAL_ARRAY);

    //glClientActiveTextureARB(GL_TEXTURE0);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    //glClientActiveTextureARB(GL_TEXTURE1);
    //glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glDisableVertexAttribArray(7);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  void MultiResHeightMapRenderer::setDrawSolid(bool drawSolid) {
    solid = highSolid = drawSolid;
  }

  void MultiResHeightMapRenderer::setDrawWireframe(bool drawWireframe) {
    wireframe = highWireframe = drawWireframe;
  }

} // namespace mars

#endif
