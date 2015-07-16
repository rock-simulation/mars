/*
 *  Copyright 2011, 2012 DFKI GmbH Robotics Innovation Center
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
 *  TerrainDrawObject2.h
 *  General TerrainDrawObject2 to inherit from.
 *
 *  Created by Roemmermann on 21.10.09.
 */

#ifndef MARS_GRAPHICS_TERRAIN_DRAW_OBJECT2_H
#define MARS_GRAPHICS_TERRAIN_DRAW_OBJECT2_H


//#include "DrawObject.h"
#include "LoadDrawObject.h"

#include <mars/interfaces/MARSDefs.h>
#include <mars/utils/Vector.h>
#include <mars/interfaces/terrainStruct.h>

#include <string>
#include <vector>
#include <list>
#include <map>

namespace mars {
  namespace graphics {

#ifdef USE_VERTEX_BUFFER
    class VertexBufferTerrain;
#endif

    struct TerrainDrawObject2Info {
      // texture scaling - a value of 0 will
      // set the texture coordinates to fit the terrain
      double texScale;
      std::string fileName;
      std::string objectName;
    }; // end of struct TerrainDrawObject2Info

    struct SubTile {
      double xPos, yPos;
      double **heightData;
      double xRes, yRes;
      double xSize, ySize;
      int xCount, yCount;
      double vStart;
      double vCount;
      int where;
      int frame;
      int mapIndex;
      int x, y;
      osg::ref_ptr<osg::DrawElementsUInt> pSet;
      osg::ref_ptr<osg::Vec3Array> vertices;
      osg::ref_ptr<osg::Vec4Array> tangents;
      osg::ref_ptr<osg::Vec2Array> texcoords;
      osg::ref_ptr<osg::Vec3Array> normals;
      osg::ref_ptr<osg::Vec3Array> normal_debug;
      osg::ref_ptr<osg::Geometry> normalGeom;
      osg::ref_ptr<osg::Geometry> geom;
      osg::ref_ptr<osg::Geode> geode;
    }; // end of struct SubTile

    class TerrainDrawObject : public DrawObject {

    public:
      TerrainDrawObject(GraphicsManager *g,
                        const mars::interfaces::terrainStruct *ts);
      virtual ~TerrainDrawObject(void);
      virtual void generateTangents();
      virtual void collideSphere(mars::utils::Vector pos,
                                 mars::interfaces::sReal radius);
      static int countSubTiles;

#ifdef USE_VERTEX_BUFFER
      virtual void setSelected(bool val);
#endif

    private:

#ifdef USE_VERTEX_BUFFER
      osg::ref_ptr<VertexBufferTerrain> vbt;
#endif

      mars::interfaces::terrainStruct info;
      osg::ref_ptr<osg::Vec4Array> tangents;
      osg::ref_ptr<osg::Geometry> geom;

      osg::ref_ptr<osg::Vec3Array> vertices;
      osg::ref_ptr<osg::Vec2Array> texcoords;
      osg::ref_ptr<osg::Vec3Array> normals;
      osg::ref_ptr<osg::Vec3Array> normal_debug;
      osg::ref_ptr<osg::Geometry> normal_geom;

      double **height_data;

      int tangentUnit;
      int num_y, num_x;
      double x_step, y_step;
      double x_step2, y_step2;
      double tex_scale_x, tex_scale_y;
      std::vector<std::vector<LoadDrawObjectPSetBox*>*> gridPSets;
      virtual std::list< osg::ref_ptr< osg::Geode > > createGeometry();

      std::map<int, SubTile*> subTiles;
      std::vector<SubTile*> vSubTiles;

      void createNewSubTile(SubTile *newSubTile, mars::utils::Vector pos,
                            double radius);
      mars::utils::Vector getNormal(int x, int y, int mx, int my,
                       double x_step, double y_step,
                       double **height_data, osg::Vec3d* t,
                       bool skipBorder = false);

      int rectangleIntersect(const osg::Vec3& first_leftup,
                             const osg::Vec3& first_downright,
                             const osg::Vec3& second_leftup,
                             const osg::Vec3& second_downright);
      osg::Vec3d intersectReplacementSphere(osg::Vec3 center, double radius,
                                            osg::Vec3 vertex);
      void cutHole(int x1, int x2, int y1, int y2);
      bool adaptSubtile(SubTile* tile, mars::utils::Vector pos, double radius);
      void removeSubTile(SubTile *tile);
      void updateShadowObject(mars::utils::Vector pos);
      void drawSubTile(SubTile *tile);
      double getHeight(int x, int y, SubTile *tile);

    }; // end of class TerrainDrawObject

  } // end of namespace graphics
} // end of namespace mars

#endif /* MARS_GRAPHICS_TERRAIN_DRAW_OBJECT_H */
