/*
 *  Copyright 2012, 2014, DFKI GmbH Robotics Innovation Center
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
 * \author Malte Langosz
 *
 */

#ifndef LOAD_CENTER_H
#define LOAD_CENTER_H

#ifdef _PRINT_HEADER_
  #warning "LoaderInterface.h"
#endif

#include <map>
#include <string>
#include <vector>

namespace mars {
  namespace interfaces {

    struct NodeData;
    struct terrainStruct;

    struct indexMaps_t {
      std::string s_Scenename;
      std::map<unsigned long, unsigned long> m_indexMap;
      std::map<unsigned long, unsigned long> m_indexMapJoints;
      std::map<unsigned long, unsigned long> m_indexMapMotors;
      std::map<unsigned long, unsigned long> m_indexMapSensors;
      std::map<unsigned long, unsigned long> m_indexMapControllers;
      std::map<unsigned long, unsigned long> m_indexMapGroupID;
    };

    
    class LoadMeshInterface {
    public:
      virtual ~LoadMeshInterface() {}
      virtual void getPhysicsFromMesh(NodeData *node) = 0;
    };


    class LoadHeightmapInterface {
    public:
      virtual ~LoadHeightmapInterface() {}
      virtual void readPixelData(terrainStruct *terrain) = 0;
    };

    class LoadSceneInterface;

    class LoadCenter {
    public:
      LoadCenter();
      ~LoadCenter();
      
      unsigned int getMappedSceneByName(const std::string &scenename) const;
      void setMappedSceneName(const std::string &scenename);

      unsigned long getMappedID(unsigned long id,
                                unsigned int indextype,
                                unsigned int source) const;

      unsigned int setMappedID(unsigned long id_old,
                               unsigned long id_new,
                               unsigned int indextype,
                               unsigned int source);

      LoadMeshInterface *loadMesh;
      LoadHeightmapInterface *loadHeightmap;
      std::map<std::string, LoadSceneInterface*> loadScene;

    private:
      std::vector<indexMaps_t> maps;
    };

  } // end of namespace interfaces
} // end of namespace mars

#endif //LOAD_CENTER_H
