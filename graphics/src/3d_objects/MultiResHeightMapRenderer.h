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

namespace mars {

  struct SubTile {
    int x, y;                    // x, y position of low res cell
    int indicesArrayOffset;      // offset in indices array
    int verticesArrayOffset;     // offset in vertices array
    double **heightData;
    double xPos, yPos;
    int mapIndex;
  }; // struct SubTile

  struct FootPrint {
    double x, y, z, r;
  };

  struct VertexData;

  class MultiResHeightMapRenderer {
  public:
    MultiResHeightMapRenderer(int gridW, int gridH,
                              double visualWidth, double visualHeight,
                              double scaleX, double scaleY,
                              double scaleZ, double texScaleX,
                              double texScaleY);

    ~MultiResHeightMapRenderer();

    void initialize();
    void highInitialize();
    void render();
    void collideSphere(double xPos, double yPos, double zPos, double radius);
    void setHeight(unsigned int gridX, unsigned int gridY, double height);
    double getHeight(unsigned int gridX, unsigned int gridY);
    void setOffset(double x, double y, double z);
    void setDrawSolid(bool drawSolid);
    void setDrawWireframe(bool drawWireframe);

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
    { return highWidth; }
    inline int getHighResCellCntY() const
    { return highHeight; }

  protected:
    void clear();
    void cutHole(int x, int y);
    //        void addCell(int x, int y);
    void fillCell(SubTile *tile);
    void copyLast(int indicesOffsetPos, int verticesOffsetPos);
    void adaptSubTile(SubTile* tile, double xPos, double yPos,
                      double zPos, double radius);
    double intersectReplacementSphere(double x, double y, double z,
                                      double radius,
                                      double vx, double vy, double vz);
    bool initPlane(bool highRes);
    void recalcSteps();

  private:
    double interpolateCell(int gridX, int gridY, double x, double y);
    void prepare();

    GLuint *vboIds;
    bool isInitialized, highIsInitialized;
    double targetWidth, targetHeight;
    int width, height;
    double scaleX, scaleY, scaleZ;
    double texScaleX, texScaleY;

    VertexData *vertices;
    GLuint *indices;
    VertexData *highVertices;
    GLuint *highIndices;

    int numVertices, numIndices;
    int highNumVertices, highNumIndices;
    int indicesToDraw, highIndicesToDraw;

    double stepX, stepY;
    double highStepX, highStepY;
    int highWidth, highHeight;
    int maxNumSubTiles, numSubTiles;
    double newIndicesPos, newVerticesPos;
    double **heightData;
    bool dirty;
    double offset[3];
    double minX, minY, minZ, maxX, maxY, maxZ;
    bool wireframe, solid, highWireframe, highSolid;

    std::map<int, SubTile*> subTiles;
    std::list<SubTile*> listSubTiles;

    void getNormal(int x, int y, int mx, int my, double x_step,
                   double y_step, double **height_data, float *normal,
                   float *tangent, bool skipBorder);

    void normalize(float *v);

    std::list<FootPrint> footPrints;

    void collideSphereI(double xPos, double yPos, double zPos, double radius);
    void fillOriginal(int x, int y);
    double getHeight(int x, int y, SubTile *tile);
    void drawSubTile(SubTile *tile);
    void render(bool highRes);

  }; // class MultiResHeightMapRenderer
  
} // namespace VSPluginMultiResHeightMap


#endif /* VSPluginMultiResHeightMapRenderer_H */

