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

#ifndef MultiResHeightMapRenderer_H
#define MultiResHeightMapRenderer_H

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
#include <vector>
#ifdef WIN32
 #include <cv.h>
 #include <highgui.h>
#else
 #include <opencv/cv.h>
 #include <opencv/highgui.h>
#endif

#include <mars/utils/Thread.h>
#include <mars/utils/Mutex.h>

namespace osg_terrain {

  class Tile {
  public:
    Tile() : x(0), y(0), indicesArrayOffset(0), verticesArrayOffset(0),
             heightData(0), xPos(0), yPos(0), cols(0), rows(0), width(0),
	     height(0), parent(0) {
      x = 0;
    }
    int x, y;                    // x, y position of low res cell
    int indicesArrayOffset;      // offset in indices array
    int verticesArrayOffset;     // offset in vertices array
    double **heightData;
    double xPos, yPos, stepX, stepY;
    int cols, rows;
    double width, height;
    int listPos;
    Tile *parent;
    std::vector<Tile*> subTiles;
    double getHeight(double x, double y);
  }; // Tile

  struct VertexData;

  class MultiResHeightMapRenderer : public mars::utils::Thread {
  public:
    MultiResHeightMapRenderer(int gridW, int gridH,
                              double visualWidth, double visualHeight,
                              double scaleZ, double texScaleX,
                              double texScaleY, int depth,
                              std::string imagefile);

    ~MultiResHeightMapRenderer();

    void initialize();
    void highInitialize();
    void render();
    void collideSphere(double xPos, double yPos, double zPos, double radius);
    void setHeight(unsigned int gridX, unsigned int gridY, double height);
    //double getHeight(unsigned int gridX, unsigned int gridY);
    void setDrawSolid(bool drawSolid);
    void setDrawWireframe(bool drawWireframe);
    void setCameraPosition(double x, double y) {
      camX = x;
      camY = y;
    }

    inline int getLowResVertexCntX() const
    { return width; }
    inline int getLowResVertexCntY() const
    { return height; }
    inline int getLowResCellCntX() const
    { return getLowResVertexCntX()-1; }
    inline int getLowResCellCntY() const
    { return getLowResVertexCntY()-1; }
    inline int getHighResVertexCntX() const
    { return getHighResCellCntX()+1; }
    inline int getHighResVertexCntY() const
    { return getHighResCellCntY()+1; }
    inline int getHighResCellCntX() const
    { return 9; }
    inline int getHighResCellCntY() const
    { return 9; }

  protected:
    void clear();
    Tile* createSubTile(int x, int y, Tile *tile);
    void fillSubTile(Tile *tile);
    void cutTile(int x, int y, Tile *tile);
    void cutHole(int x, int y, int x2, int y2, Tile *tile);
    //        void addCell(int x, int y);
    // void fillCell(Tile *tile);
    // void copyLast(int indicesOffsetPos, int verticesOffsetPos);
    // void adaptSubTile(Tile* tile, double xPos, double yPos,
    //                   double zPos, double radius);
    // double intersectReplacementSphere(double x, double y, double z,
    //                                   double radius,
    //                                   double vx, double vy, double vz);
    bool initPlane(bool highRes);
    void recalcSteps();
    double getHeight(double x, double y);
    void handleCamPos(double x, double y, Tile *tile);
    void clearTile(Tile *tile);
    void drawPatch(int x, int y, Tile *tile);
    void moveTile(int indicesOffsetPos, int verticesOffsetPos, Tile *tile);

    // inherited from mars::utils::Thread
    void run(); ///< The simulator main loop.

  private:
    //double interpolateCell(int gridX, int gridY, double x, double y);
    void prepare();

    GLuint *vboIds;
    bool isInitialized, highIsInitialized;
    double targetWidth, targetHeight;
    int width, height;
    double scaleZ;
    double texScaleX, texScaleY;
    Tile *mainTile;
    double camX, camY;
    VertexData *vertices;
    GLuint *indices;
    VertexData *highVertices;
    GLuint *highIndices;

    // for thread optimization
    VertexData *vertexCopy, *highVertexCopy;
    GLuint *indexCopy, *highIndexCopy;
    bool swapBuffer, swapBuffer2, finish;
    mars::utils::Mutex dataMutex, swapMutex;

    int numVertices, numIndices;
    int highNumVertices, highNumIndices;
    int indicesToDraw, highIndicesToDraw, highIndicesToDrawBuffer;

    double highStepX, highStepY;
    int highWidth, highHeight;
    int maxNumSubTiles, numSubTiles, depth;
    double newIndicesPos, newVerticesPos;
    bool dirty;
    double minX, minY, minZ, maxX, maxY, maxZ;
    bool wireframe, solid, highWireframe, highSolid;
    IplImage* img;

    std::map<int, Tile*> subTiles;
    std::vector<Tile*> listSubTiles;

    void getNormal(int x, int y, int mx, int my, double x_step,
                   double y_step, double **height_data, float *normal,
                   float *tangent, bool skipBorder);

    void normalize(float *v);

    //void collideSphereI(double xPos, double yPos, double zPos, double radius);
    //void fillOriginal(int x, int y);
    double getHeight(int x, int y, Tile *tile);
    //void drawSubTile(Tile *tile);
    void render(bool highRes);

  }; // class MultiResHeightMapRenderer
  
} // namespace osg_terrain


#endif /* MultiResHeightMapRenderer_H */

