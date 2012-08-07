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
#include <string.h>

//#define DEBUG_TIME

#ifdef DEBUG_TIME
#include <mars/base/utils.h>
#endif

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
    : vboIds(new GLuint[2]), isInitialized(false), targetWidth(visualW),
      targetHeight(visualH), width(gridW), height(gridH), scaleX(scaleX),
      scaleY(scaleY), scaleZ(scaleZ), texScaleX(texScaleX),
      texScaleY(texScaleY) {

    maxNumSubTiles = 32;
    heightData = NULL;
    numSubTiles = 0;
    prepare();

    for(int i = 0; i < 3; ++i)
      offset[i] = 0.;

    dirty = true;
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
    //glEnable(GL_DEPTH_TEST);
    //glDisable(GL_CULL_FACE);
    glGenBuffers(2, vboIds);

    isInitialized = true;

    // Initializes cube geometry and transfers it to VBOs
    initPlane();
  }

  void MultiResHeightMapRenderer::prepare() {
    recalcSteps();
    heightData = new double*[height];
    for(int i = 0; i < height; ++i) {
      heightData[i] = new double[width];
    }
    for(int i = 0; i < height; ++i)
      for(int j = 0; j < width; ++j)
        heightData[i][j] = -1;
    numVertices = (width*height + maxNumSubTiles*(highWidth+1)*(highHeight+1));
    numIndices = (width-1)*(height-1)*6+maxNumSubTiles*highWidth*highHeight*6;
    indicesToDraw = (width-1)*(height-1)*6;
    newIndicesPos = (height-1)*(width-1)*6;
    newVerticesPos = height*width;
  }

  void MultiResHeightMapRenderer::clear() {

    free (vertices);
    vertices = NULL;
    free (indices);
    indices = NULL;
    glBindBuffer(GL_ARRAY_BUFFER, vboIds[0]);
    glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW);
    glDeleteBuffers(2, vboIds);

    while(!listSubTiles.empty()) {
      SubTile *toRemove = listSubTiles.front();
      listSubTiles.pop_front();
      // this is probably wrong. because the lastTile is not updated. comment out for now
      //copyLast(toRemove->indicesArrayOffset, toRemove->verticesArrayOffset);
      subTiles.erase(toRemove->mapIndex);
      if(toRemove->heightData) {
        for(int i = 0; i < highHeight+1; ++i) {
          delete toRemove->heightData[i];
        }
        delete toRemove->heightData;
      }
      delete toRemove;
      --numSubTiles;
      newIndicesPos -= highWidth * highHeight * 6;
      newVerticesPos -= (highWidth+1) * (highHeight+1);
    }
    if(heightData) {
      for(int i = 0; i < height; ++i) {
        delete[] heightData[i];
      }
      delete[] heightData;
      heightData = NULL;
    }
  }

  void MultiResHeightMapRenderer::recalcSteps() {
    highStepX = stepX = targetWidth / double(width-1);
    highStepY = stepY = targetHeight / double(height-1);

    highWidth = highHeight = 1;
    double radius = 0.05;
    double desiredStep = radius / 10;

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
    //GLboolean culling;

    if(!isInitialized) initialize();

#ifdef DEBUG_TIME
    long drawTime = base::getTime();
#endif

    //glGetBooleanv(GL_CULL_FACE, &culling);
    //if(culling)
    //glDisable(GL_CULL_FACE);

    if(dirty) {
      glBindBuffer(GL_ARRAY_BUFFER, vboIds[0]);
      VertexData *vertices = (VertexData*)glMapBuffer(GL_ARRAY_BUFFER,
                                                      GL_WRITE_ONLY);
      int index;
      for(int y=0; y<height; ++y) {
        for(int x=0; x<width; ++x) {
          index = y*width + x;
          vertices[index].position[0] = x * stepX * scaleX;
          vertices[index].position[1] = y * stepY * scaleY;
          vertices[index].position[2] = heightData[y][x] * scaleZ;
          vertices[index].texCoord[0] = x * stepX * scaleX*texScaleX;
          vertices[index].texCoord[1] = y * stepY * scaleY*texScaleY;
          getNormal(x, y, width, height, stepX, stepY, heightData,
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

    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glTranslatef(offset[0], offset[1], offset[2]);

    glPointSize(5);
    glColor3f(1, 0, 0);

    // Tell OpenGL which VBOs to use
    glBindBuffer(GL_ARRAY_BUFFER, vboIds[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[1]);

    // do same as vertex array except pointer
    // activate vertex coords array
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glVertexPointer(3, GL_FLOAT, sizeof(VertexData), 0); // last param is offset, not ptr
    glNormalPointer(GL_FLOAT, sizeof(VertexData), (GLvoid*)( (char*)NULL + 3*sizeof(GLfloat))); // last param is offset, not ptr

    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData),
                          (GLvoid*)( (char*)NULL + 6*sizeof(GLfloat)));

    glClientActiveTextureARB(GL_TEXTURE0_ARB);
    glTexCoordPointer(2, GL_FLOAT, sizeof(VertexData),
                      (GLvoid*)( (char*)NULL + 10*sizeof(GLfloat)));
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    /*
      glClientActiveTextureARB(GL_TEXTURE1_ARB);
      glTexCoordPointer(2, GL_FLOAT, sizeof(VertexData),
      (GLvoid*)( (char*)NULL + 10*sizeof(GLfloat)));
    */
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    //mat_diffuse[0] = 0.0;
    //mat_diffuse[1] = 1.0;
    //glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    //glColor3f(0, 1, 0);

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glDrawElements(GL_TRIANGLES, indicesToDraw, GL_UNSIGNED_INT, 0);

    GLfloat mat_diffuse[] = { 1.0, 0.0, 0.0, 1.0 };
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);

    glLineWidth(2.0);

    // draw 6 quads using offset of index array
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glDrawElements(GL_TRIANGLES, indicesToDraw, GL_UNSIGNED_INT, 0);
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    
    glDisableClientState(GL_VERTEX_ARRAY);            // deactivate vertex array
    glDisableClientState(GL_NORMAL_ARRAY);

    glClientActiveTextureARB(GL_TEXTURE0_ARB);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    //glClientActiveTextureARB(GL_TEXTURE1_ARB);
    //glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glDisableVertexAttribArray(7);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    //if(culling)
    //glEnable(GL_CULL_FACE);
    glPopMatrix();

#ifdef DEBUG_TIME
    fprintf(stderr, "MultiResHeightMapRenderer: drawTime: %ld\n",
            base::getTimeDiff(drawTime));
#endif
  }

  void MultiResHeightMapRenderer::initPlane() {

    vertices = (VertexData*)malloc(numVertices*sizeof(VertexData));
    if(!vertices) {
      return;
    }
    for(int y=0; y<height; ++y) {
      for(int x=0; x<width; ++x) {
        int index = y*width+x;
        vertices[index].position[0] = x * stepX * scaleX;
        vertices[index].position[1] = y * stepY * scaleY;
        vertices[index].position[2] = heightData[y][x] * scaleZ;
        vertices[index].texCoord[0] = x * stepX * scaleX*texScaleX;
        vertices[index].texCoord[1] = y * stepY * scaleY*texScaleY;
        getNormal(x, y, width, height, stepX, stepY, heightData,
                  vertices[index].normal,
                  vertices[index].tangent, true);
      }
    }

    indices = (GLuint*)calloc(numIndices, sizeof(GLuint));
    if(!indices) {
      free(vertices);
      return;
    }
    for(int y=0; y<height-1; ++y) {
      for(int x=0; x<width-1; ++x) {
        indices[y*(width-1)*6+x*6+0] = (y+1) * width + x;
        indices[y*(width-1)*6+x*6+1] = y * width + x;
        indices[y*(width-1)*6+x*6+2] = (y+1) * width + x+1;

        indices[y*(width-1)*6+x*6+3] = (y+1) * width + x+1;
        indices[y*(width-1)*6+x*6+4] = y * width + x;
        indices[y*(width-1)*6+x*6+5] = y * width + x+1;
      }
    }

    glBindBuffer(GL_ARRAY_BUFFER, vboIds[0]);
    glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(VertexData), vertices, GL_DYNAMIC_DRAW);
    //        glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(VertexData), vertices, GL_STATIC_DRAW);

    // Transfer index data to VBO 1
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(GLuint), indices, GL_DYNAMIC_DRAW);
  }


  void MultiResHeightMapRenderer::cutHole(int x, int y) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[1]);
    GLuint *indices = (GLuint*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER,
                                           GL_WRITE_ONLY);
    if(indices) {
      for(int i = 0; i < 6; ++i)
        indices[y*(width-1)*6+x*6+i] = 0;
    }
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  void MultiResHeightMapRenderer::fillOriginal(int x, int y) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[1]);
    GLuint *indices = (GLuint*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER,
                                           GL_WRITE_ONLY);
    if(indices) {
      indices[y*(width-1)*6+x*6+0] = (y+1) * width + x;
      indices[y*(width-1)*6+x*6+1] = y * width + x;
      indices[y*(width-1)*6+x*6+2] = (y+1) * width + x+1;

      indices[y*(width-1)*6+x*6+3] = (y+1) * width + x+1;
      indices[y*(width-1)*6+x*6+4] = y * width + x;
      indices[y*(width-1)*6+x*6+5] = y * width + x+1;
    }
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // check for subtile neighbours
    std::map<int, SubTile*>::iterator it;
    double val;
    if(y > 0) {
      it = subTiles.find((y-1)*(width-1)+x);
      if(it != subTiles.end()) {
        for(int x1=0; x1<highWidth+1; ++x1) {
          val = getHeight(x1, highHeight, it->second);
          it->second->heightData[highHeight][x1] = val;
        }
        drawSubTile(it->second);
      }
    }

    if(y > 0 && x > 0) {
      it = subTiles.find((y-1)*(width-1)+x-1);
      if(it != subTiles.end()) {
        val = getHeight(highWidth, highHeight, it->second);
        it->second->heightData[highHeight][highWidth] = val;
        drawSubTile(it->second);
      }
    }

    if(x > 0) {
      it = subTiles.find(y*(width-1)+x-1);
      if(it != subTiles.end()) {
        for(int y1=0; y1<highHeight+1; ++y1) {
          val = getHeight(highWidth, y1, it->second);
          it->second->heightData[y1][highWidth] = val;
        }
        drawSubTile(it->second);
      }
    }

    if(y < height-1) {
      it = subTiles.find((y+1)*(width-1)+x);
      if(it != subTiles.end()) {
        for(int x1=0; x1<highWidth+1; ++x1) {
          it->second->heightData[0][x1] = getHeight(x1, 0, it->second);
        }
        drawSubTile(it->second);
      }
    }

    if(y < height-1 && x < width-1) {
      it = subTiles.find((y+1)*(width-1)+x+1);
      if(it != subTiles.end()) {
        it->second->heightData[0][0] = getHeight(0, 0, it->second);
        drawSubTile(it->second);
      }
    }

    if(x < width-1) {
      it = subTiles.find((y)*(width-1)+x+1);
      if(it != subTiles.end()) {
        for(int y1=0; y1<highHeight+1; ++y1)
          it->second->heightData[y1][0] = getHeight(0, y1, it->second);
        drawSubTile(it->second);
      }
    }

    if(y > 0 && x < width-1) {
      it = subTiles.find((y-1)*(width-1)+x+1);
      if(it != subTiles.end()) {
        val = getHeight(0, highHeight, it->second);
        it->second->heightData[highHeight][0] = val;
        drawSubTile(it->second);
      }
    }

    if(y < height-1 && x > 0) {
      it = subTiles.find((y+1)*(width-1)+x-1);
      if(it != subTiles.end()) {
        val = getHeight(highWidth, 0, it->second);
        it->second->heightData[0][highWidth] = val;
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
    indices[y*(width-1)*6+x*6+0] = y * width + x;
    indices[y*(width-1)*6+x*6+1] = (y+1) * width + x;
    indices[y*(width-1)*6+x*6+2] = (y+1) * width + x+1;

    indices[y*(width-1)*6+x*6+3] = y * width + x;
    indices[y*(width-1)*6+x*6+4] = (y+1) * width + x+1;
    indices[y*(width-1)*6+x*6+5] = y * width + x+1;
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
    assert((gridY < height-1) && (gridX < width-1));
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
    glBindBuffer(GL_ARRAY_BUFFER, vboIds[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[1]);
    VertexData *vertices = (VertexData*)glMapBuffer(GL_ARRAY_BUFFER,
                                                    GL_WRITE_ONLY);

    GLuint *indices = (GLuint*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER,
                                           GL_WRITE_ONLY);

    if(vertices) {
      int x2 = tile->verticesArrayOffset;
      int index;

      for(int l=0; l<highHeight+1; l++) {
        for(int j=0; j<highWidth+1; j++) {
          index = x2 + l*(highWidth+1) + j;
          double x = tile->x*stepX + j*highStepX;
          double y = tile->y*stepY + l*highStepY;
          double z = interpolateCell(tile->x, tile->y, x, y);
          tile->heightData[l][j] = z;
          vertices[index].position[0] = x * scaleX;
          vertices[index].position[1] = y * scaleY;
          vertices[index].position[2] = z * scaleZ;
          vertices[index].texCoord[0] = x * scaleX*texScaleX;
          vertices[index].texCoord[1] = y * scaleY*texScaleY;
        }
      }
      for(int l=0; l<highHeight+1; l++) {
        for(int j=0; j<highWidth+1; j++) {
          index = x2 + l*(highWidth+1) + j;
          getNormal(j, l, highWidth, highHeight, highStepX, highStepY,
                    tile->heightData,
                    vertices[index].normal,
                    vertices[index].tangent, true);
        }
      }
    }

    if(indices) {
      int offset = tile->indicesArrayOffset;
      int x2 = tile->verticesArrayOffset;

      for(int l=0; l<highHeight; l++) {
        for(int j=0; j<highWidth; j++) {

          indices[offset+l*highWidth*6+j*6] = x2+(l+1)*(highWidth+1)+j;
          indices[offset+l*highWidth*6+j*6+1] = x2+l*(highWidth+1)+j;
          indices[offset+l*highWidth*6+j*6+2] = x2+(l+1)*(highWidth+1)+j+1;

          indices[offset+l*highWidth*6+j*6+3] = x2+(l+1)*(highWidth+1)+j+1;
          indices[offset+l*highWidth*6+j*6+4] = x2+l*(highWidth+1)+j;
          indices[offset+l*highWidth*6+j*6+5] = x2+l*(highWidth+1)+j+1;
        }
      }
    }
    indicesToDraw += highHeight*highWidth*6;

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  void MultiResHeightMapRenderer::unfillCell() {
    indicesToDraw -= highHeight*highWidth*6;
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

    std::list<SubTile*> toProcess;
    std::list<SubTile*>::iterator iter;

    int x1 = floor((xPos-radius*1.2) / stepX);
    int y1 = floor((yPos-radius*1.2) / stepY);
    int x2 = ceil((xPos+radius*1.2) / stepX);
    int y2 = ceil((yPos+radius*1.2) / stepY);

    if(y1 < 0) y1 = 0;
    if(x1 < 0) x1 = 0;
    if(y2 > width) y2 = width; // ToDo: check bounds correctnes
    if(x2 > width) x2 = width;

    std::map<int, SubTile*>::iterator it;

    // code to remove a SubTile without replacing it
    if(0) {
      SubTile *toRemove = listSubTiles.front();
      listSubTiles.pop_front();
      numSubTiles--;
      indicesToDraw -= highHeight*highWidth*6;
      copyLast(toRemove->indicesArrayOffset, toRemove->verticesArrayOffset);
      subTiles.erase(toRemove->mapIndex);
      fillOriginal(toRemove->x, toRemove->y);

      // free memory
      for(int l=0; l<highHeight+1; ++l)
        delete[] toRemove->heightData[l];
      delete[] toRemove->heightData;
      delete toRemove;
    }

    for(int i = y1; i < y2; ++i) {
      for(int j = x1; j < x2; ++j) {
        it = subTiles.find(i*(width-1)+j);
        if(it != subTiles.end()) {
          toProcess.push_back(it->second);
          //fprintf(stderr, "have Subtile\n");
        } else {
          if(numSubTiles >= maxNumSubTiles) {
            //fprintf(stderr, "remove Subtile\n");
            SubTile *toRemove = listSubTiles.front();
            listSubTiles.pop_front();
            numSubTiles--;
            indicesToDraw -= highHeight*highWidth*6;
            newIndicesPos = toRemove->indicesArrayOffset;
            newVerticesPos = toRemove->verticesArrayOffset;
            subTiles.erase(toRemove->mapIndex);
            fillOriginal(toRemove->x, toRemove->y);

            // free memory
            for(int l=0; l<highHeight+1; ++l)
              delete[] toRemove->heightData[l];
            delete[] toRemove->heightData;
            delete toRemove;
          }
          cutHole(j, i);
          SubTile *newSubTile = new SubTile;
          newSubTile->x = j;
          newSubTile->y = i;
          newSubTile->xPos = j*stepX;
          newSubTile->yPos = i*stepY;
          newSubTile->indicesArrayOffset = newIndicesPos;
          newSubTile->verticesArrayOffset = newVerticesPos;
          newIndicesPos += highWidth*highHeight*6;
          newVerticesPos += (highWidth+1)*(highHeight+1);
          newSubTile->heightData = new double*[highHeight+1];
          for(int l=0; l<highHeight+1; ++l) {
            newSubTile->heightData[l] = new double[highWidth+1];
          }
          for(int n=0; n< (highHeight+1); ++n)
            for(int m=0; m<(highWidth+1); ++m)
              // TODO: Should we interpolate here?
              newSubTile->heightData[n][m] = heightData[i][j];

          fillCell(newSubTile);

          newSubTile->mapIndex = i*(width-1)+j;
          subTiles[newSubTile->mapIndex] = newSubTile;
          listSubTiles.push_back(newSubTile);
          toProcess.push_back(newSubTile);
          numSubTiles++;
        }
      }
    }

    std::list<SubTile*>::iterator listIter;
    for(iter=toProcess.begin(); iter!=toProcess.end(); ++iter) {
      for(listIter=listSubTiles.begin(); listIter!=listSubTiles.end();
          ++listIter) {
        if(*listIter == *iter) {
          listSubTiles.erase(listIter);
          listSubTiles.push_back(*iter);
          break;
        }
      }
      //fprintf(stderr, "adapt Subtile\n");
      adaptSubTile(*iter, xPos, yPos, zPos, radius);
    }
  }

  void MultiResHeightMapRenderer::copyLast(int indicesOffsetPos,
                                           int verticesOffsetPos) {
    glBindBuffer(GL_ARRAY_BUFFER, vboIds[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[1]);
    VertexData *vertices = (VertexData*)glMapBuffer(GL_ARRAY_BUFFER,
                                                    GL_WRITE_ONLY);

    GLuint *indices = (GLuint*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER,
                                           GL_WRITE_ONLY);

    SubTile* lastSubTile = listSubTiles.back();

    if(vertices) {
      int offset = verticesOffsetPos;
      int offset2 = lastSubTile->verticesArrayOffset;

      //            memcpy(vertices+offset, vertices+offset2, (highHeight+1)*(highWidth+1));
      memcpy(vertices+offset, vertices+offset2, (highHeight+1)*(highWidth+1)*sizeof(VertexData));
    }
    if(indices) {
      int offset = indicesOffsetPos;
      int offset2 = lastSubTile->indicesArrayOffset;

      //            memcpy(indices+offset, indices+offset2, highHeight*highWidth*6);
      memcpy(indices+offset, indices+offset2, highHeight*highWidth*6*sizeof(GLuint));
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
    if(x2 > highWidth+1) x2 = highWidth+1;
    if(y2 > highHeight+1) y2 = highHeight+1;
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

          if(y < highHeight && (tile->heightData[y][x] <
          tile->heightData[y+1][x]-highStepY*interpolation)) {
          tile->heightData[y+1][x] = tile->heightData[y][x]+highStepY*interpolation;
          }

          if(x < highWidth && (tile->heightData[y][x] <
          tile->heightData[y][x+1]-highStepX*interpolation)) {
          tile->heightData[y][x+1] = tile->heightData[y][x]+highStepX*interpolation;
          }

          // the corners
          if((y > 0) && (x > 0) &&
          (tile->heightData[y][x] < tile->heightData[y-1][x-1]-(highStepY+highStepX)*interpolation*0.5)) {
          tile->heightData[y-1][x-1] = tile->heightData[y][x]+(highStepY+highStepX)*interpolation*0.5;
          }

          if((y > 0) && (x < highWidth) &&
          (tile->heightData[y][x] < tile->heightData[y-1][x+1]-(highStepY+highStepX)*interpolation*0.5)) {
          tile->heightData[y-1][x+1] = tile->heightData[y][x]+(highStepY+highStepX)*interpolation*0.5;
          }

          if((y < highHeight) && (x > 0) &&
          (tile->heightData[y][x] < tile->heightData[y+1][x-1]-(highStepY+highStepX)*interpolation*0.5)) {
          tile->heightData[y+1][x-1] = tile->heightData[y][x]+(highStepY+highStepX)*interpolation*0.5;
          }

          if((y < highHeight) && (x < highWidth) &&
          (tile->heightData[y][x] < tile->heightData[y+1][x+1]-(highStepY+highStepX)*interpolation*0.5)) {
          tile->heightData[y+1][x+1] = tile->heightData[y][x]+(highStepY+highStepX)*interpolation*0.5;
          }
          }
        */
      }
    }

    if(adapt) {
      glBindBuffer(GL_ARRAY_BUFFER, vboIds[0]);
      VertexData *vertices = (VertexData*)glMapBuffer(GL_ARRAY_BUFFER,
                                                      GL_WRITE_ONLY);
      VertexData* v;
      int index;
      for(int y=y1; y<y2; ++y) {
        for(int x=x1; x<x2; ++x) {
          index = tile->verticesArrayOffset+y*(highWidth+1)+x;
          v = vertices+index;
          v->position[2] = tile->heightData[y][x] * scaleZ;
          getNormal(x, y, highWidth, highHeight, highStepX, highStepY,
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
        vz1 *= scaleZ;
        vx1 = x_step*2.0;
      }
      else if(x==0) {
        vz1 = height_data[y][x+2] - height_data[y][x+1];
        vz1 *= scaleZ;
        vx1 = x_step;
      }
      else if(x==1) {
        vz1 = height_data[y][x+1] - height_data[y][x];
        vz1 *= scaleZ;
        vx1 = x_step;
      }
      else if(x==mx-1) {
        vz1 = height_data[y][x-1] - height_data[y][x-2];
        vz1 *= scaleZ;
        vx1 = x_step;
      }
      else {
        vz1 = height_data[y][x] - height_data[y][x-1];
        vz1 *= scaleZ;
        vx1 = x_step;
      }

      if(y > 1 && y < my-2) {
        vz2 = height_data[y+1][x] - height_data[y-1][x];
        vz2 *= scaleZ;
        vy2 = y_step*2.0;
      }
      else if(y==0) {
        vz2 = height_data[y+2][x] - height_data[y+1][x];
        vz2 *= scaleZ;
        vy2 = y_step;
      }
      else if(y==1) {
        vz2 = height_data[y+1][x] - height_data[y][x];
        vz2 *= scaleZ;
        vy2 = y_step;
      }
      else if(y==my-1) {
        vz2 = height_data[y-1][x] - height_data[y-2][x];
        vz2 *= scaleZ;
        vy2 = y_step;
      }
      else {
        vz2 = height_data[y][x] - height_data[y-1][x];
        vz2 *= scaleZ;
        vy2 = y_step;
      }
    }
    else {
      if(x != 0 && x != mx-1) {
        vz1 = height_data[y][x+1] - height_data[y][x-1];
        vz1 *= scaleZ;
        vx1 = x_step*2.0;
      }
      else if(x==0) {
        vz1 = height_data[y][x+1] - height_data[y][x];
        vz1 *= scaleZ;
        vx1 = x_step;
      }
      else {
        vz1 = height_data[y][x] - height_data[y][x-1];
        vz1 *= scaleZ;
        vx1 = x_step;
      }

      if(y != 0 && y != my-1) {
        vz2 = height_data[y+1][x] - height_data[y-1][x];
        vz2 *= scaleZ;
        vy2 = y_step*2.0;
      }
      else if(y==0) {
        vz2 = height_data[y+1][x] - height_data[y][x];
        vz2 *= scaleZ;
        vy2 = y_step;
      }
      else {
        vz2 = height_data[y][x] - height_data[y-1][x];
        vz2 *= scaleZ;
        vy2 = y_step;
      }
    }

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
    double length = sqrt(v[0]*v[0]+ v[1]*v[1]+ v[2]*v[2]);
    v[0] /= length;
    v[1] /= length;
    v[2] /= length;
  }

  double MultiResHeightMapRenderer::getHeight(int x, int y, SubTile *tile) {
    double x1, y1;
    x1 = tile->x*stepX + x*highStepX;
    y1 = tile->y*stepY + y*highStepY;
    return interpolateCell(tile->x, tile->y, x1, y1);
  }

  void MultiResHeightMapRenderer::drawSubTile(SubTile *tile) {
    glBindBuffer(GL_ARRAY_BUFFER, vboIds[0]);
    VertexData *vertices = (VertexData*)glMapBuffer(GL_ARRAY_BUFFER,
                                                    GL_WRITE_ONLY);
    VertexData* v;
    int index;

    for(int y=0; y<highHeight+1; ++y) {
      for(int x=0; x<highWidth+1; ++x) {
        index = tile->verticesArrayOffset+y*(highWidth+1)+x;
        v = vertices+index;
        v->position[2] = tile->heightData[y][x] * scaleZ;
        getNormal(x, y, highWidth, highHeight, highStepX, highStepY,
                  tile->heightData,
                  vertices[index].normal,
                  vertices[index].tangent, true);
      }
    }
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

} // namespace mars

#endif
