/*
 *  Copyright 2014, DFKI GmbH Robotics Innovation Center
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
 * \file LoadCenter.cpp
 * \author Malte Langozs
 *
 */


#include "LoadCenter.h"
#include "MARSDefs.h"

namespace mars {
  namespace interfaces {

    using namespace std;

    LoadCenter::LoadCenter() : loadMesh(NULL), loadHeightmap(NULL) {
    }

    LoadCenter::~LoadCenter() {

    }

    unsigned int LoadCenter::getMappedSceneByName(const std::string &scenename) const {
      for (unsigned int i=0; i<maps.size(); i++) {
        if (maps[i].s_Scenename==scenename) {
          return i+1;
        }
      }
      return 0;
    }
    
    void LoadCenter::setMappedSceneName(const std::string &scenename) {
      indexMaps_t tmp;
      tmp.s_Scenename=scenename;
      maps.push_back(tmp);
    }

    unsigned long LoadCenter::getMappedID(unsigned long id,
                                          unsigned int indextype,
                                          unsigned int source) const {
      map<unsigned long, unsigned long>::const_iterator it;

      if(id==0) return 0;
      if(maps.size() < source) return 0;
      switch (indextype) {
      case interfaces::MAP_TYPE_NODE:
        it = maps[source-1].m_indexMap.find(id);
        break;
      case interfaces::MAP_TYPE_JOINT:
        it = maps[source-1].m_indexMapJoints.find(id);
        break;
      case interfaces::MAP_TYPE_MOTOR:
        it = maps[source-1].m_indexMapMotors.find(id);
        break;
      case interfaces::MAP_TYPE_SENSOR:
        it = maps[source-1].m_indexMapSensors.find(id);
        break;
      case interfaces::MAP_TYPE_CONTROLLER:
        it = maps[source-1].m_indexMapControllers.find(id);
        break;
      default:
        return 0;
        break;
      }
      if(it->first == id)
        return it->second;
      return 0;
    }

    unsigned int LoadCenter::setMappedID(unsigned long id_old,
                                         unsigned long id_new,
                                         unsigned int indextype,
                                         unsigned int source) {
      switch (indextype) {
      case interfaces::MAP_TYPE_NODE:
        maps[source-1].m_indexMap[id_old]=id_new;
        return 1;
        break;
      case interfaces::MAP_TYPE_JOINT:
        maps[source-1].m_indexMapJoints[id_old]=id_new;
        return 1;
        break;
      case interfaces::MAP_TYPE_MOTOR:
        maps[source-1].m_indexMapMotors[id_old]=id_new;
        return 1;
        break;
      case interfaces::MAP_TYPE_SENSOR:
        maps[source-1].m_indexMapSensors[id_old]=id_new;
        return 1;
        break;
      case interfaces::MAP_TYPE_CONTROLLER:
        maps[source-1].m_indexMapControllers[id_old]=id_new;
        return 1;
        break;
      default:
        break;
      }
      return 0;
    }

  } // end of namespace interfaces
} // end of namespace mars
