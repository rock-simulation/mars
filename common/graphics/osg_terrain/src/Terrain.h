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

/**
 * \file Terrain.h
 * \author Malte Langosz
 * \brief 
 **/

#ifndef OSG_TERRAIN_H
#define OSG_TERRAIN_H

#ifdef _PRINT_HEADER_
#warning "Terrain.h"
#endif

#include <osg/Group>
#include <osg/Geometry>
#include <mars/osg_material_manager/OsgMaterialManager.h>
#include <mars/osg_material_manager/MaterialNode.h>
#include <mars/utils/Vector.h>
#include "VertexBufferTerrain.h"

namespace osg_terrain {

  class Terrain : public osg::Group {

  public:
    Terrain(osg_material_manager::OsgMaterialManager *m);
    ~Terrain();
    void setCameraPos(double x, double y, double z);

  private:
    osg_material_manager::OsgMaterialManager *materialManager;
    osg::ref_ptr<osg::Node> createPlane();
    osg::ref_ptr<VertexBufferTerrain> vbt;
    double tPosX, tPosY;
  };

} // end of namespace: osg_terrain

#endif // OSG_TERRAIN_H
