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

/**
 * \file LoadCenter.h
 * \author Malte Roemmermann
 *
 */

#ifndef LOAD_CENTER_H
#define LOAD_CENTER_H

#ifdef _PRINT_HEADER_
  #warning "LoaderInterface.h"
#endif

namespace mars {
  namespace interfaces {

    struct NodeData;
    struct terrainStruct;

    
    class LoadMeshInterface {
    public:
      virtual ~LoadMeshInterface() {}
      virtual void getPhysicsFromOBJ(NodeData *node) = 0;
    };


    class LoadHeightmapInterface {
    public:
      virtual ~LoadHeightmapInterface() {}
      virtual void readPixelData(terrainStruct *terrain) = 0;
    };

    class LoadSceneInterface;

    class LoadCenter {
    public:
      LoadCenter(){
        loadMesh = 0;
        loadHeightmap = 0;
        loadScene = 0;
      }
      
      LoadMeshInterface *loadMesh;
      LoadHeightmapInterface *loadHeightmap;
      LoadSceneInterface *loadScene;
    };

  } // end of namespace interfaces
} // end of namespace mars

#endif //LOAD_CENTER_H
