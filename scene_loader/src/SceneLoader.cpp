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
 *
 * \file SceneLoader.cpp
 * \author Malte Roemmermann
 */

#include "SceneLoader.h"
#include "Load.h"
#include "Save.h"

#include <mars/lib_manager/LibManager.h>
#include <mars/lib_manager/LibInterface.h>
#include <mars/interfaces/sim/SimulatorInterface.h>

namespace mars {
  namespace scene_loader {

    using namespace std;

    SceneLoader::SceneLoader(lib_manager::LibManager *theManager) :
      interfaces::LoadSceneInterface(theManager) {
      indexMaps_t tmp;
      maps.push_back(tmp);

      mars::interfaces::SimulatorInterface *marsSim;
      marsSim = libManager->getLibraryAs<mars::interfaces::SimulatorInterface>("mars_sim");
      if(marsSim) {
        control = marsSim->getControlCenter();
        control->loadCenter->loadScene = this;
      }
    }

    SceneLoader::~SceneLoader() {
      if(control) {
        control->loadCenter->loadScene = NULL;
        libManager->releaseLibrary("mars_sim");
      }
    }

    bool SceneLoader::loadFile(std::string filename, std::string tmpPath,
                               std::string robotname) {
      Load loadObject(filename, control, tmpPath,
                      (const std::string&) robotname);
      return loadObject.load();
    }

    int SceneLoader::saveFile(std::string filename, std::string tmpPath) {
      Save saveObject(filename.c_str(), control, tmpPath);
      return saveObject.prepare();
    }

    unsigned long SceneLoader::getMappedID(unsigned long id,
                                           unsigned int indextype,
                                           unsigned int source) const {
      map<unsigned long, unsigned long>::const_iterator it;

      if(id==0) return 0;
      if(maps.size() <= source) return 0;
      switch (indextype) {
      case interfaces::MAP_TYPE_NODE:
        it = maps[source].m_indexMap.find(id);
        break;
      case interfaces::MAP_TYPE_JOINT:
        it = maps[source].m_indexMapJoints.find(id);
        break;
      case interfaces::MAP_TYPE_MOTOR:
        it = maps[source].m_indexMapMotors.find(id);
        break;
      case interfaces::MAP_TYPE_SENSOR:
        it = maps[source].m_indexMapSensors.find(id);
        break;
      case interfaces::MAP_TYPE_CONTROLLER:
        it = maps[source].m_indexMapControllers.find(id);
        break;
      default:
        return 0;
        break;
      }
      if(it->first == id)
        return it->second;
      return 0;
    }

    unsigned int SceneLoader::setMappedID(unsigned long id_old,
                                          unsigned long id_new,
                                          unsigned int indextype,
                                          unsigned int source) {
      switch (indextype) {
      case interfaces::MAP_TYPE_NODE:
        maps[source].m_indexMap[id_old]=id_new;
        return 1;
        break;
      case interfaces::MAP_TYPE_JOINT:
        maps[source].m_indexMapJoints[id_old]=id_new;
        return 1;
        break;
      case interfaces::MAP_TYPE_MOTOR:
        maps[source].m_indexMapMotors[id_old]=id_new;
        return 1;
        break;
      case interfaces::MAP_TYPE_SENSOR:
        maps[source].m_indexMapSensors[id_old]=id_new;
        return 1;
        break;
      case interfaces::MAP_TYPE_CONTROLLER:
        maps[source].m_indexMapControllers[id_old]=id_new;
        return 1;
        break;
      default:
        break;
      }
      return 0;
    }

    void SceneLoader::setMappedSceneName(const std::string &scenename) {
      indexMaps_t tmp;
      tmp.s_Scenename=scenename;
      maps.push_back(tmp);
    }

    unsigned int SceneLoader::getMappedSceneByName(const std::string &scenename) const {
      for (unsigned int i=0; i<maps.size(); i++) {
        if (maps[i].s_Scenename==scenename) {
          return i;
        }
      }
      return 0;
    }


  } // end of namespace scene_loader
} // end of namespace mars

DESTROY_LIB(mars::scene_loader::SceneLoader);
CREATE_LIB(mars::scene_loader::SceneLoader);
