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
 * \file URDFLoader.cpp
 * \author Malte Langosz, Kai von Szadkowski
 */

#include "URDFLoader.h"
#include "Load.h"
#include "Save.h"

#include <mars/lib_manager/LibManager.h>
#include <mars/lib_manager/LibInterface.h>
#include <mars/interfaces/sim/SimulatorInterface.h>

namespace mars {
  namespace urdf_loader {

    using namespace std;

    URDFLoader::URDFLoader(lib_manager::LibManager *theManager) :
      interfaces::LoadSceneInterface(theManager), control(NULL) {

      mars::interfaces::SimulatorInterface *marsSim;
      marsSim = libManager->getLibraryAs<mars::interfaces::SimulatorInterface>("mars_sim");
      if(marsSim) {
        control = marsSim->getControlCenter();
        control->loadCenter->loadScene[".zsmurf"] = this;
        control->loadCenter->loadScene[".smurf"] = this;
        control->loadCenter->loadScene[".urdf"] = this;
        LOG_INFO("urdf_loader: added urdf loader to loadCenter");
        //control->loadCenter->loadScene[".yaml"] = this;
        //control->loadCenter->loadScene[".yml"] = this;
      }
    }

    URDFLoader::~URDFLoader() {
      if(control) {
        control->loadCenter->loadScene.erase(".zsmurf");
        control->loadCenter->loadScene.erase(".smurf");
        control->loadCenter->loadScene.erase(".urdf");
        //control->loadCenter->loadScene.erase(".yaml");
        //control->loadCenter->loadScene.erase(".yml");
        libManager->releaseLibrary("mars_sim");
      }
    }

    bool URDFLoader::loadFile(std::string filename, std::string tmpPath,
                              std::string robotname) {
      Load loadObject(filename, control, tmpPath,
                      (const std::string&) robotname);
      return loadObject.load();
    }

    int URDFLoader::saveFile(std::string filename, std::string tmpPath) {
      Save saveObject(filename.c_str(), control, tmpPath);
      return saveObject.prepare();
    }

  } // end of namespace urdf_loader
} // end of namespace mars

DESTROY_LIB(mars::urdf_loader::URDFLoader);
CREATE_LIB(mars::urdf_loader::URDFLoader);
