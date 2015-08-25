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
 *
 * \file SceneLoader.cpp
 * \author Malte Roemmermann
 */

#include "SceneLoader.h"
#include "Load.h"
#include "Save.h"

#include <lib_manager/LibManager.hpp>
#include <lib_manager/LibInterface.hpp>
#include <mars/interfaces/sim/SimulatorInterface.h>

namespace mars {
  namespace scene_loader {

    using namespace std;

    SceneLoader::SceneLoader(lib_manager::LibManager *theManager) :
      interfaces::LoadSceneInterface(theManager), control(NULL) {

      mars::interfaces::SimulatorInterface *marsSim;
      marsSim = libManager->getLibraryAs<mars::interfaces::SimulatorInterface>("mars_sim");
      if(marsSim) {
        control = marsSim->getControlCenter();
        control->loadCenter->loadScene[".scn"] = this;
        control->loadCenter->loadScene[".scene"] = this;
        control->loadCenter->loadScene[".yml"] = this;
        //control->loadCenter->loadScene[".zip"] = this;
      }
    }

    SceneLoader::~SceneLoader() {
      if(control) {
        control->loadCenter->loadScene.erase(".scn");
        control->loadCenter->loadScene.erase(".scene");
        control->loadCenter->loadScene.erase(".yml");
        //control->loadCenter->loadScene.erase(".zip");
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

  } // end of namespace scene_loader
} // end of namespace mars

DESTROY_LIB(mars::scene_loader::SceneLoader);
CREATE_LIB(mars::scene_loader::SceneLoader);
