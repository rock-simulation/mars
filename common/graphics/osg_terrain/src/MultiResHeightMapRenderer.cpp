/*
 *  Copyright 2016, DFKI GmbH Robotics Innovation Center
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

namespace osg_terrain {

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
                                                       double scaleZ,
                                                       double texScaleX,
                                                       double texScaleY,
                                                       int depth,
                                                       std::string imagefile)
    : vboIds(new GLuint[4]), isInitialized(false), highIsInitialized(false),
      targetWidth(visualW), targetHeight(visualH), width(gridW), height(gridH),
      scaleZ(scaleZ), texScaleX(texScaleX),
      texScaleY(texScaleY), depth(depth) {

    maxNumSubTiles = 0; // will be the depth in new version

    numSubTiles = 0;
    prepare();
    dirty = true;
    swapBuffer = false;
    swapBuffer2 = false;
    wireframe = false;
    highWireframe = false;
    solid = true;
    highSolid = true;
    camX = camY = 0;
    img=cvLoadImage(imagefile.data(), -1);
    if(!img) {
      fprintf(stderr, "error loading heightmap file!\n");
    }
    finish = false;
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
    if(highIsInitialized) this->start();
  }


  void MultiResHeightMapRenderer::prepare() {
    //recalcSteps();
    double **heightData = new double*[getLowResVertexCntY()];
    for(int y = 0; y < getLowResVertexCntY(); ++y) {
      heightData[y] = new double[getLowResVertexCntX()];
      for(int x = 0; x < getLowResVertexCntX(); ++x) {
        heightData[y][x] = -1;
      }
    }
    mainTile = new Tile();
    mainTile->width = targetWidth;
    //fprintf(stderr, "targetWidth: %g\n", mainTile->width);
    mainTile->height = targetHeight;
    mainTile->cols = width-1;
    mainTile->rows = height-1;
    mainTile->stepX = targetWidth / (width-1);
    mainTile->stepY = targetHeight / (height-1);
    mainTile->heightData = heightData;

    int d = 1;
    int f = 1;
    f = f*4;
    maxNumSubTiles += f;
    d += 1;
    while(d < depth+1) {
      f = f*4;
      maxNumSubTiles += f;
      d += 1;
    }
    double scale = 1;
    if(depth == 2) scale = 0.5;
    else if(depth == 3) scale = 0.2;
    else if(depth == 4) scale = 0.1;
    numVertices = getLowResVertexCntX()*getLowResVertexCntY();
    highNumVertices = scale*numVertices*maxNumSubTiles;//maxNumSubTiles*getHighResVertexCntX()*getHighResVertexCntY();
    fprintf(stderr, "maxNumSubTiles: %d\n", maxNumSubTiles);
    fprintf(stderr, "num vertices: %d\n", numVertices+highNumVertices);
    numIndices = getLowResCellCntX()*getLowResCellCntY()*6;
    highNumIndices = scale*numIndices*maxNumSubTiles;//maxNumSubTiles*getHighResCellCntX()*getHighResCellCntY()*6;
    indicesToDraw = getLowResCellCntX()*getLowResCellCntY()*6;
    highIndicesToDraw = 0;
    highIndicesToDrawBuffer = 0;
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
      Tile *toRemove = listSubTiles.back();
      listSubTiles.pop_back();
      if(toRemove->heightData) {
        for(int i = 0; i < getHighResVertexCntY(); ++i) {
          delete toRemove->heightData[i];
        }
        delete toRemove->heightData;
      }
      delete toRemove;
      --numSubTiles;
    }
    if(mainTile) {
      for(int i = 0; i < mainTile->rows; ++i) {
        delete[] mainTile->heightData[i];
      }
      delete[] mainTile->heightData;
      delete mainTile;
      mainTile = 0;
    }
  }

  void MultiResHeightMapRenderer::recalcSteps() {
    double stepX, stepY;
    highStepX = stepX = targetWidth / double(getLowResCellCntX());
    highStepY = stepY = targetHeight / double(getLowResCellCntY());
    highStepX = stepX / 3;
    highStepY = stepY / 3;
    dirty = true;
  }

  void MultiResHeightMapRenderer::render() {
    static double t = 0.0;
    t += 0.001;
    double x = camX;//targetWidth*0.5+2.7;
    double y = camY;//targetHeight*0.5+3.1;
    //x += sin(t)*4;
    //y += cos(t)*4;

    if(!isInitialized) initialize();

    if(dirty) {
      dataMutex.lock();
      VertexData *vertices = vertexCopy;
      int index;
      double xPos, yPos;
      for(int y = 0; y < mainTile->cols+1; ++y) {
        for(int x = 0; x < mainTile->rows+1; ++x) {
          index = mainTile->verticesArrayOffset+y*(mainTile->rows+1) + x;
          xPos = mainTile->xPos+x * mainTile->stepX;
          yPos = mainTile->yPos+y * mainTile->stepY;
          vertices[index].position[0] = xPos;
          vertices[index].position[1] = yPos;
          double z = getHeight(xPos, yPos);
          mainTile->heightData[y][x] = z;
          vertices[index].position[2] = z;
          vertices[index].texCoord[0] = xPos* texScaleX;
          vertices[index].texCoord[1] = yPos * texScaleY;
        }
      }
      for(int y = 0; y < mainTile->cols+1; ++y) {
        for(int x = 0; x < mainTile->rows+1; ++x) {
          index = mainTile->verticesArrayOffset+y*(mainTile->rows+1) + x;
          // todo: handle getNormal correct
          getNormal(x, y, mainTile->rows, mainTile->cols,
                    mainTile->stepX, mainTile->stepY, mainTile->heightData,
                    vertices[index].normal,
                    vertices[index].tangent, true);
        }
      }
      dataMutex.unlock();
      dirty = false;
      swapMutex.lock();
      swapBuffer = true;
      swapMutex.unlock();
    }

    if(!highIsInitialized) highInitialize();
    if(!highIsInitialized) return;

    swapMutex.lock();
    if(swapBuffer) {
      swapMutex.unlock();
      glBindBuffer(GL_ARRAY_BUFFER, vboIds[2]);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[3]);
      VertexData *vertices = (VertexData*)glMapBuffer(GL_ARRAY_BUFFER,
                                                      GL_WRITE_ONLY);

      GLuint *indices = (GLuint*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER,
                                             GL_WRITE_ONLY);

      dataMutex.lock();
      if(vertices) {
        memcpy(vertices, highVertexCopy, listSubTiles.size()*numVertices*sizeof(VertexData));
      }
      if(indices) {
        memcpy(indices, highIndexCopy, highIndicesToDrawBuffer*sizeof(GLuint));
      }
      glUnmapBuffer(GL_ARRAY_BUFFER);
      glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
      glBindBuffer(GL_ARRAY_BUFFER, vboIds[0]);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[1]);
      vertices = (VertexData*)glMapBuffer(GL_ARRAY_BUFFER,
                                          GL_WRITE_ONLY);

      indices = (GLuint*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER,
                                     GL_WRITE_ONLY);

      if(vertices) {
        memcpy(vertices, vertexCopy, numVertices*sizeof(VertexData));
      }
      if(indices) {
        memcpy(indices, indexCopy, numIndices*sizeof(GLuint));
      }
      glUnmapBuffer(GL_ARRAY_BUFFER);
      glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

      highIndicesToDraw = highIndicesToDrawBuffer;
      dataMutex.unlock();
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      swapMutex.lock();
      swapBuffer = false;
    }
    swapMutex.unlock();
    render(false);

    if(highIsInitialized && highIndicesToDraw) {
      render(true);
    }
  }

  void MultiResHeightMapRenderer::handleCamPos(double x, double y, Tile *tile) {
    // get cell in main tile
    //fprintf(stderr, "check: %lu\n", tile);
    //fprintf(stderr, "%g %g %g %g\n", tile->xPos, tile->yPos, x, y);
    std::vector<Tile*>::iterator it, it2;
    double cellStepX = tile->width/3;
    double cellStepY = tile->height/3;
    double x2 = x - 0.5*cellStepX;
    double y2 = y - 0.5*cellStepY;
    int cellx = floor((-tile->xPos + x2) / cellStepX);
    int celly = floor((-tile->yPos + y2) / cellStepY);
    int cellx2 = floor((-tile->xPos + x + 0.5*cellStepX) / cellStepX);
    int celly2 = floor((-tile->yPos + y + 0.5*cellStepY) / cellStepY);
    std::vector<Tile*> tiles;

    // todo: num cell tiles is everywhere in this file hardcoded with 3
    bool skip = false;
    if(x-tile->xPos+0.5*cellStepX < 0 ||
       x-tile->xPos-0.5*cellStepX > tile->width ||
       y-tile->yPos+0.5*cellStepY < 0 ||
       y-tile->yPos-0.5*cellStepY > tile->height) {
      skip = true;
    }
    if(!skip) {
      for(int i=0; i<4; ++i) {
        int cy, cx;
        if(i==0) {
          cx = cellx;
          cy = celly;
          if(cx < 0 || cy < 0) continue;
          if(cx > 2) cx = 2;
          if(cy > 2) cy = 2;
        }
        else if(i==1) {
          cx = cellx2;
          cy = celly;
          if(cy < 0) continue;
          if(cx < 0) cx = 0;
          if(cx > 2) continue;
          if(cy > 2) cy = 2;
        }
        else if(i==2) {
          cx = cellx;
          cy = celly2;
          if(cx < 0) continue;
          if(cy < 0) cy = 0;
          if(cy > 2) continue;
          if(cx > 2) cx = 2;
        }
        else if(i==3) {
          cx = cellx2;
          cy = celly2;
          if(cx > 2 || cy > 2) continue;
          if(cx < 0) cx = 0;
          if(cy < 0) cy=0;
        }
        // now check that we don't already have this cell in this loop
        bool found = false;
        for(it=tiles.begin(); it!=tiles.end(); ++it) {
          if((*it)->x == cx && (*it)->y == cy) {
            found = true;
            break;
          }
        }
        if(found) continue;
        // now check wether the subtile already exists
        for(it=tile->subTiles.begin(); it!=tile->subTiles.end(); ++it) {
          if(*it != 0) {
            if((*it)->x == cx && (*it)->y == cy) {
              tiles.push_back(*it);
              found = true;
              break;
            }
          }
        }
        if(found) continue;
        // else create it
        cutTile(cx, cy, tile);
        //fprintf(stderr, "create: cx: %d cy: %d px: %g py: %g w: %g %lu %g %g\n", cx, cy, tile->xPos, tile->yPos, tile->width/3, tile, x, y);
        //if(depth < 6)
        Tile *newTile = createSubTile(cx, cy, tile);
        if(newTile) tiles.push_back(newTile);
        else {
          fprintf(stderr, "----- we just created an hole!!!!!!!\n");
        }
      }
    }
    // now remove the subtiles that are not any more in use
    for(it=tile->subTiles.begin(); it!=tile->subTiles.end(); ++it) {
      //fprintf(stderr, "check remove\n");
      bool found = false;
      for(it2 = tiles.begin(); it2 != tiles.end(); ++it2) {
        if(*it2 == *it) {
          found = true;
        }
      }
      if(!found) {
        drawPatch((*it)->x, (*it)->y, tile);
        //fprintf(stderr, "clear: cx: %d cy: %d px: %g py: %g w: %g %lu\n", (*it)->x, (*it)->y, tile->xPos, tile->yPos, tile->width/3, tile);
        clearTile(*it);
      }

    }
    tile->subTiles.swap(tiles);
  }

  void MultiResHeightMapRenderer::clearTile(Tile *tile) {

    std::vector<Tile*>::iterator it;
    //fprintf(stderr, "clear Tile: %lu\n", tile);
    for(it=tile->subTiles.begin(); it!=tile->subTiles.end(); ++it) {
      if(*it) clearTile(*it);
    }
    if(tile->listPos < listSubTiles.size()-1) {
      // copy last entry
      Tile* lastTile = listSubTiles.back();

      /*fprintf(stderr, "move: %d %d %d %d\n", tile->indicesArrayOffset/numIndices,
              tile->verticesArrayOffset/numVertices,
              lastTile->indicesArrayOffset/numIndices,
              lastTile->verticesArrayOffset/numVertices);
      */
      //fprintf(stderr, "move2: %d %d %d\n", listSubTiles.size(), lastTile->indicesArrayOffset, listSubTiles.back()->indicesArrayOffset);
      moveTile(tile->indicesArrayOffset, tile->verticesArrayOffset, lastTile);
      lastTile->listPos = tile->listPos;
      listSubTiles[lastTile->listPos] = lastTile;
      listSubTiles.pop_back();
      //fprintf(stderr, "move3: %d %d %d\n", listSubTiles.size(), lastTile->indicesArrayOffset, listSubTiles.back()->indicesArrayOffset);
    }
    else if(tile->listPos == listSubTiles.size()-1) {
      listSubTiles.pop_back();
    }
    else {
      fprintf(stderr, "should never ever happen!!!\n");
    }
    highIndicesToDrawBuffer -= numIndices;
    //fprintf(stderr, "todraw: %d\n", highIndicesToDraw);
    for(int l = 0; l < tile->cols+1; ++l) {
      delete[] tile->heightData[l];
    }
    delete[] tile->heightData;
    delete tile;
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
      height = mainTile->cols;
      width = mainTile->rows;
    }
    else {
      numVertices = this->numVertices;
      numIndices = this->numIndices;

      vboId = 0;
      height = mainTile->cols;
      width = mainTile->rows;
    }

    vertices = (VertexData*)malloc(numVertices*sizeof(VertexData));
    if(highRes) {
      highVertexCopy = (VertexData*)malloc(numVertices*sizeof(VertexData));
    }
    else {
      vertexCopy = (VertexData*)malloc(numVertices*sizeof(VertexData));
    }
    if(!vertices) {
      fprintf(stderr, "MultiResHeightMapRenderer::initPlane error while allocating memory for vertices %d\n", numVertices);
      return false;
    }
    indices = (GLuint*)calloc(numIndices, sizeof(GLuint));
    if(highRes) {
      highIndexCopy = (GLuint*)calloc(numIndices, sizeof(GLuint));
    }
    else {
      indexCopy = (GLuint*)calloc(numIndices, sizeof(GLuint));
    }
    if(!indices) {
      fprintf(stderr, "MultiResHeightMapRenderer::initPlane error while allocating memory for indices %d\n", numIndices);
      free(vertices);
      return false;
    }
    if(highRes) {
      for(int i=0; i<numIndices; ++i) {
        indices[i] = 0;
      }
    }
    else {
      for(int y=0; y<height; ++y) {
        for(int x=0; x<width; ++x) {
          int indexOffset = y*(width)*6+x*6;
          indices[indexOffset+0] = (y+1) * (width+1) + x;
          indices[indexOffset+1] =  y    * (width+1) + x;
          indices[indexOffset+2] = (y+1) * (width+1) + x+1;
          indices[indexOffset+3] = (y+1) * (width+1) + x+1;
          indices[indexOffset+4] =  y    * (width+1) + x;
          indices[indexOffset+5] =  y    * (width+1) + x+1;
          indexCopy[indexOffset+0] = indices[indexOffset+0];
          indexCopy[indexOffset+1] = indices[indexOffset+1];
          indexCopy[indexOffset+2] = indices[indexOffset+2];
          indexCopy[indexOffset+3] = indices[indexOffset+3];
          indexCopy[indexOffset+4] = indices[indexOffset+4];
          indexCopy[indexOffset+5] = indices[indexOffset+5];
        }
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

  double MultiResHeightMapRenderer::getHeight(double x, double y) {
    // need to replaced by image
    //if(!isInitialized) return 0;
    int stepx = targetWidth / img->width;
    int stepy = targetHeight / img->height;

    x = img->width*x/targetWidth;
    y = img->height*y/targetHeight;
    if(x < 0) x = 0;
    if(y < 0) y = 0;
    int gridX = floor(x);
    int gridY = floor(y);
    if(gridX > img->width-1) gridX = img->width-1;
    if(gridY > img->height-1) gridY = img->height-1;
    int gridX2 = gridX+1;
    if(gridX2 > img->width-1) gridX2 = gridX;
    int gridY2 = gridY+1;
    if(gridY2 > img->height-1) gridY2 = gridY;
    double dx = (x - gridX);
    double dy = (y - gridY);
    double cornerHeights[4];
    double imageMaxValue = pow(2., img->depth);
    CvScalar s;
    s = cvGet2D(img, gridY, gridX);
    cornerHeights[0] = s.val[0] / imageMaxValue;
    s = cvGet2D(img, gridY2, gridX);
    cornerHeights[1] = s.val[0] / imageMaxValue;
    s = cvGet2D(img, gridY, gridX2);
    cornerHeights[2] = s.val[0] / imageMaxValue;
    s = cvGet2D(img, gridY2, gridX2);
    cornerHeights[3] = s.val[0] / imageMaxValue;
    double height = (cornerHeights[0] * (1-dx) * (1-dy) +
                     cornerHeights[1] * (1-dx) * dy +
                     cornerHeights[2] * dx * (1-dy) +
                     cornerHeights[3] * dx * dy);
    return height*scaleZ;
    //    return mainTile->getHeight(x, y);
  }

  double Tile::getHeight(double x, double y) {
    //if(!isInitialized) return 0;
    double x2 = (x-xPos)/width*cols;
    double y2 = (y-yPos)/height*rows;
    if(x2 < 0) x2 = 0;
    if(y2 < 0) y2 = 0;
    int gridX = floor(x2);
    int gridY = floor(y2);
    if(gridX > cols) gridX = cols;
    if(gridY > rows) gridY = rows;
    int gridX2 = gridX+1;
    if(gridX2 > cols) gridX2 = gridX;
    int gridY2 = gridY+1;
    if(gridY2 > rows) gridY2 = gridY;
    double dx = (x2 - gridX);
    double dy = (y2 - gridY);
    if(dx < 0.1) dx = 0;
    if(dx > 0.9) dx = 1;
    if(dy < 0.1) dy = 0;
    if(dy > 0.9) dy = 1;
    double cornerHeights[4];
    cornerHeights[0] = heightData[gridY][gridX];
    cornerHeights[1] = heightData[gridY2][gridX];
    cornerHeights[2] = heightData[gridY][gridX2];
    cornerHeights[3] = heightData[gridY2][gridX2];
    double height = (cornerHeights[0] * (1-dx) * (1-dy) +
                     cornerHeights[1] * (1-dx) * dy +
                     cornerHeights[2] * dx * (1-dy) +
                     cornerHeights[3] * dx * dy);
    return height;
  }

  Tile* MultiResHeightMapRenderer::createSubTile(int x, int y, Tile *tile) {
    int iOffset = -numIndices;
    int vOffset = -numVertices;

    if(listSubTiles.size() > 0) {
      if(listSubTiles.size() == maxNumSubTiles) return NULL;
      //fprintf(stderr, "%lu: ", listSubTiles.back());
      iOffset = listSubTiles.back()->indicesArrayOffset;
      vOffset = listSubTiles.back()->verticesArrayOffset;
    }
    assert(iOffset + numIndices < highNumIndices);
    assert(vOffset + numVertices < highNumVertices);
    if(iOffset + numIndices > highNumIndices) {
      fprintf(stderr, "not enough memory reserved for subtiles!!!\n");
      return 0;
    }
    Tile *newTile = new Tile();
    newTile->parent = tile;
    newTile->x = x;
    newTile->y = y;
    newTile->width = tile->width/3;
    newTile->height = tile->height/3;
    newTile->xPos = tile->xPos+tile->width/3*x;
    newTile->yPos = tile->yPos+tile->height/3*y;
    newTile->cols = tile->cols;//getHighResCellCntX();
    newTile->rows = tile->rows;//getHighResCellCntY();
    newTile->stepX = tile->stepX / 3;//(getHighResCellCntX()-1);
    //fprintf(stderr, "steps: %g %g\n", tile->stepX, newTile->stepX);
    newTile->stepY = tile->stepY / 3;//(getHighResCellCntY()-1);
    newTile->indicesArrayOffset = iOffset+numIndices;
    newTile->verticesArrayOffset = vOffset+numVertices;

    //fprintf(stderr, "create: (%d %d) (%d %d)\n", iOffset, numIndices,
    //        vOffset, numVertices);

    newTile->heightData = new double*[newTile->cols+1];
    for(int l = 0; l < newTile->cols+1; ++l) {
      newTile->heightData[l] = new double[newTile->rows+1];
    }
    /*
    for(int n = 0; n < newTile->cols+1; ++n)
      for(int m = 0; m < newTile->rows+1; ++m)
        // TODO: Should we interpolate here?
        newTile->heightData[n][m] = getHeight(newTile->xPos+newTile->stepX*m, newTile->yPos+newTile->stepY*n);
    */
    highIndicesToDrawBuffer += numIndices;
    //fprintf(stderr, "_todraw: %d\n", highIndicesToDraw);
    //fprintf(stderr, "fill subtile\n");
    fillSubTile(newTile);
    newTile->listPos = listSubTiles.size();
    listSubTiles.push_back(newTile);
    return newTile;
  }

  void MultiResHeightMapRenderer::cutTile(int x, int y, Tile *tile) {
    int nx = tile->rows/3;
    int ny = tile->cols/3;
    cutHole(nx*x, nx*y, nx, ny, tile);
  }

  void MultiResHeightMapRenderer::cutHole(int x, int y, int x2, int y2,
                                          Tile *tile) {
    GLuint *indices = highIndexCopy;
    if(tile==mainTile) indices = indexCopy;
    swapBuffer2 = true;
    if(indices) {
      for(int l = x; l<x+x2; ++l) {
        for(int n = y; n<y+y2; ++n) {
          for(int i = 0; i < 6; ++i)
            indices[tile->indicesArrayOffset+n*(tile->rows)*6+l*6+i] = 0;
        }
      }
    }
  }

  void MultiResHeightMapRenderer::drawPatch(int x, int y, Tile *tile) {
    int nx = tile->rows/3;
    int ny = tile->cols/3;
    GLuint *indices = highIndexCopy;
    if(tile==mainTile) indices = indexCopy;
    swapBuffer2 = true;
    if(indices) {
      int offset = tile->indicesArrayOffset;
      int x2 = tile->verticesArrayOffset;
      int vx = tile->rows+1;
      for(int iy = ny*y; iy<ny*y+ny; ++iy) {
        for(int ix = nx*x; ix<nx*x+nx; ++ix) {
          int indexOffset = offset+iy*tile->rows*6+ix*6;
          indices[indexOffset+0] = x2+((iy+1)*vx)+ix;
          indices[indexOffset+1] = x2+( iy   *vx)+ix;
          indices[indexOffset+2] = x2+((iy+1)*vx)+ix+1;

          indices[indexOffset+3] = x2+((iy+1)*vx)+ix+1;
          indices[indexOffset+4] = x2+( iy   *vx)+ix;
          indices[indexOffset+5] = x2+( iy   *vx)+ix+1;
        }
      }
    }
  }

  void MultiResHeightMapRenderer::fillSubTile(Tile *tile) {
    // use highResBuffer
    VertexData *vertices = highVertexCopy;
    GLuint *indices = highIndexCopy;
    swapBuffer2 = true;
    if(vertices) {
      int x2 = tile->verticesArrayOffset;
      int index;

      for(int iy = 0; iy < tile->cols+1; ++iy) {
        for(int ix = 0; ix < tile->rows+1; ++ix) {
          index = x2 + iy*(tile->rows+1) + ix;
          double x = tile->xPos + ix*tile->stepX;
          double y = tile->yPos + iy*tile->stepY;
          double z = 0;
          if(iy==0 || ix == 0 || iy == tile->cols || ix == tile->rows) {
            z = tile->parent->getHeight(x, y);
          }
          else {
            z = getHeight(x, y);
          }
          tile->heightData[iy][ix] = z;
          vertices[index].position[0] = x;
          vertices[index].position[1] = y;
          vertices[index].position[2] = z;
          vertices[index].texCoord[0] = x * texScaleX;
          vertices[index].texCoord[1] = y * texScaleY;
        }
      }
      // calculate normals in a separate loop because the calculation depends on the neighbours
      for(int iy = 0; iy < tile->cols+1; ++iy) {
        for(int ix = 0; ix < tile->rows+1; ++ix) {
          index = x2 + iy*(tile->rows+1) + ix;
          getNormal(ix, iy, tile->rows, tile->cols,
                    tile->stepX, tile->stepY,
                    tile->heightData,
                    vertices[index].normal,
                    vertices[index].tangent, true);
        }
      }
    }

    if(indices) {
      int offset = tile->indicesArrayOffset;
      int x2 = tile->verticesArrayOffset;
      int vx = tile->rows+1;
      for(int iy = 0; iy < tile->cols; ++iy) {
        for(int ix = 0; ix < tile->rows; ++ix) {
          int indexOffset = offset+iy*tile->rows*6+ix*6;
          indices[indexOffset+0] = x2+((iy+1)*vx)+ix;
          indices[indexOffset+1] = x2+( iy   *vx)+ix;
          indices[indexOffset+2] = x2+((iy+1)*vx)+ix+1;

          indices[indexOffset+3] = x2+((iy+1)*vx)+ix+1;
          indices[indexOffset+4] = x2+( iy   *vx)+ix;
          indices[indexOffset+5] = x2+( iy   *vx)+ix+1;
        }
      }
    }
    //highIndicesToDraw += tile->cols*tile->rows*6;
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
    mainTile->heightData[gridY][gridX] = height;
    dirty = true;
  }

  // adapt to new getHeight function
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

    //vz1 *= scaleZ;
    //vx1 *= scaleX;
    //vz2 *= scaleZ;
    //vy2 *= scaleY;

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

  double MultiResHeightMapRenderer::getHeight(int x, int y, Tile *tile) {
    double x1, y1;
    x1 = tile->x*tile->parent->stepX + x*tile->stepX;
    y1 = tile->y*tile->parent->stepY + y*tile->stepY;
    return tile->parent->getHeight(x1, y1);
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
      glLineWidth(4.0);

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

  void MultiResHeightMapRenderer::moveTile(int indicesOffsetPos,
                                           int verticesOffsetPos,
                                           Tile *tile) {
    VertexData *vertices = highVertexCopy;
    GLuint *indices = highIndexCopy;
    swapBuffer2 = true;

    if(vertices) {
      //fprintf(stderr, "copy vertices from %d to %d\n", tile->verticesArrayOffset, verticesOffsetPos);
      memcpy(vertices+verticesOffsetPos, vertices+tile->verticesArrayOffset,
             numVertices*sizeof(VertexData));

    }
    else {
      fprintf(stderr, "error in moveTile function\n");
    }
    if(indices) {
      int from = tile->indicesArrayOffset;
      int to = indicesOffsetPos;
      //fprintf(stderr, "copy indices from %d to %d\n", tile->indicesArrayOffset, indicesOffsetPos);
      for(int i=0; i<numIndices; ++i) {
        indices[to+i] = indices[from+i]-(tile->verticesArrayOffset-verticesOffsetPos);
      }
      /*
      memcpy(indices+indicesOffsetPos, indices+tile->indicesArrayOffset,
             numIndices*sizeof(GLuint));
      */
    }
    else {
      fprintf(stderr, "error in moveTile function\n");
    }


    tile->indicesArrayOffset = indicesOffsetPos;
    tile->verticesArrayOffset = verticesOffsetPos;
  }

  void MultiResHeightMapRenderer::run() {
    std::vector<Tile*> nextTiles;
    std::vector<Tile*> nextTiles2;
    std::vector<Tile*>::iterator outer, inner;
    int d;
    double x, y;
    while(!finish && depth > 0) {
      swapBuffer2 = false;
      dataMutex.lock();
      x = camX;
      y = camY;
      handleCamPos(x, y, mainTile);
      d = 1;
      // handle cam pos on current subtiles
      nextTiles.clear();
      nextTiles.push_back(mainTile);
      while(d < depth) {
        //fprintf(stderr, "depth: %d %d\n", depth, offset);
        for(outer=nextTiles.begin(); outer!=nextTiles.end(); ++outer) {
          //fprintf(stderr, "outer: %lu\n", *outer);
          for(inner=(*outer)->subTiles.begin();
              inner!=(*outer)->subTiles.end();
              ++inner) {
            handleCamPos(x, y, *inner);
            nextTiles2.push_back(*inner);
            //fprintf(stderr, "-depth: %d %d\n", depth, offset);
          }
        }
        nextTiles.swap(nextTiles2);
        nextTiles2.clear();
        d += 1;
      }
      dataMutex.unlock();
      if(swapBuffer2) {
        swapMutex.lock();
        swapBuffer = true;
        swapMutex.unlock();
      }
      msleep(40);
    }
  }

} // namespace osg_terrain
