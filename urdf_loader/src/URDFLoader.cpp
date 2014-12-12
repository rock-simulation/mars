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
#include "zipit.h"

#include <mars/lib_manager/LibManager.h>
#include <mars/lib_manager/LibInterface.h>
#include <mars/interfaces/sim/SimulatorInterface.h>
#include <mars/interfaces/sim/EntityManagerInterface.h>
#include <mars/utils/misc.h>
#include <mars/logging/Logging.hpp>

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
      std::string filename_ = filename;

      if (robotname != "") {
        control->entities->addEntity(robotname);
      }

      LOG_INFO("urdf_loader: prepare loading");

      std::string mFileSuffix = utils::getFilenameSuffix(filename);

      // need to unzip into a temporary directory
      if (mFileSuffix == ".zsmurf") {
        if (unzip(tmpPath, filename) == 0) {
          return 0;
        }
        mFileSuffix = ".smurf";
      } else {
        // can parse file without unzipping
        tmpPath = utils::getPathOfFile(filename);
      }

      utils::removeFilenamePrefix(&filename_);
      utils::removeFilenameSuffix(&filename_);

      unsigned int mapIndex;
      mapIndex= control->loadCenter->getMappedSceneByName(filename);
      if (mapIndex == 0) {
        control->loadCenter->setMappedSceneName(filename);
        mapIndex = control->loadCenter->getMappedSceneByName(filename);
      }
      std::string sceneFilename = tmpPath + filename_ + mFileSuffix;

      Load loadObject(control, tmpPath, robotname, mapIndex);


      mFileSuffix = utils::getFilenameSuffix(filename);
      if(mFileSuffix == ".smurf") {
        std::string file;
        utils::ConfigMap map = utils::ConfigMap::fromYamlFile(filename);
        utils::ConfigVector::iterator it = map["files"].begin();
        for(; it!=map["files"].end(); ++it) {
          file = (std::string)(*it);
          mFileSuffix = utils::getFilenameSuffix(file);
          if(mFileSuffix == ".urdf") {
            loadObject.parseURDF(tmpPath + file);
          }
          else if(mFileSuffix == ".yml") {
            utils::ConfigMap m2 = utils::ConfigMap::fromYamlFile(tmpPath+file);
            loadObject.addConfigMap(m2);            
          }
          else {
            fprintf(stderr, "URDFLoader: %s not yet implemented",
                    mFileSuffix.c_str());
          }
        }
      }
      else {
        loadObject.parseURDF(filename);
      }

      return loadObject.load();
    }

    int URDFLoader::saveFile(std::string filename, std::string tmpPath) {
      Save saveObject(filename.c_str(), control, tmpPath);
      return saveObject.prepare();
    }


    unsigned int URDFLoader::unzip(const std::string& destinationDir,
                                     const std::string& zipFilename) {
      if (!utils::createDirectory(destinationDir))
        return 0;

      Zipit myZipFile(zipFilename);
      LOG_INFO("Load: unsmurfing zipped SMURF: %s", zipFilename.c_str());

      if (!myZipFile.unpackWholeZipTo(destinationDir))
        return 0;

      return 1;
    }

  } // end of namespace urdf_loader
} // end of namespace mars

DESTROY_LIB(mars::urdf_loader::URDFLoader);
CREATE_LIB(mars::urdf_loader::URDFLoader);
