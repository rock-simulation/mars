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
#include <mars/sim/SimEntity.h>
#include <mars/utils/misc.h>


namespace mars {
  namespace urdf_loader {

    using namespace std;

    URDFLoader::URDFLoader(lib_manager::LibManager *theManager) :
      interfaces::LoadSceneInterface(theManager), control(NULL),
      mars::plugins::entity_generation::EntityFactoryInterface("smurf, urdf"){

      mars::interfaces::SimulatorInterface *marsSim;
      marsSim = libManager->getLibraryAs<mars::interfaces::SimulatorInterface>("mars_sim");
      if(marsSim) {
        control = marsSim->getControlCenter();
        control->loadCenter->loadScene[".zsmurf"] = this; // zipped smurf model
        control->loadCenter->loadScene[".zsmurfs"] = this; // zipped smurf scene
        control->loadCenter->loadScene[".smurf"] = this; // smurf model
        control->loadCenter->loadScene[".smurfs"] = this; // smurf scene
        control->loadCenter->loadScene[".urdf"] = this; // urdf model
        LOG_INFO("urdf_loader: added SMURF loader to loadCenter");
      }

      factoryManager =
            theManager->acquireLibraryAs<mars::plugins::entity_generation::EntityFactoryManager>(
                "entity_factory_manager");
        factoryManager->registerFactory("urdf", this);
        factoryManager->registerFactory("smurf", this);
    }

    URDFLoader::~URDFLoader() {
      if(control) {
        control->loadCenter->loadScene.erase(".zsmurf");
        control->loadCenter->loadScene.erase(".zsmurfs");
        control->loadCenter->loadScene.erase(".smurf");
        control->loadCenter->loadScene.erase(".smurfs");
        control->loadCenter->loadScene.erase(".urdf");
        libManager->releaseLibrary("mars_sim");
      }
    }

    sim::SimEntity* URDFLoader::createEntity(const utils::ConfigMap& parameters) {
      mars::utils::ConfigMap config;
      config.append(parameters);
      sim::SimEntity* entity = new sim::SimEntity(config);
      std::string path = (std::string)config["path"];
      if ((std::string)config["name"] == "") {
              config["name"] = (std::string)config["modelname"];
            }
      std::string robotname = (std::string)config["name"];

      // create a load object with the correct relative path
      unsigned int mapIndex;
      mapIndex= control->loadCenter->getMappedSceneByName(robotname);
      if (mapIndex == 0) {
        control->loadCenter->setMappedSceneName(robotname);
        mapIndex = control->loadCenter->getMappedSceneByName(robotname);
      }
      Load loadObject(control, path, robotname, mapIndex);
      loadObject.setEntityConfig(config);
      std::string urdfpath = "";

      utils::ConfigVector::iterator it;
      std::string file;
      std::string file_extension;
      for(it = config["files"].begin(); it!=config["files"].end(); ++it) {
        file = (std::string)(*it);
        file_extension = utils::getFilenameSuffix(file);
        if(file_extension == ".urdf") {
          urdfpath = path + file;
          fprintf(stderr, "  ...loading urdf data from %s.\n", urdfpath.c_str());
          loadObject.parseURDF(urdfpath);
        }
        else if(file_extension == ".yml") {
          utils::ConfigMap m2 = utils::ConfigMap::fromYamlFile(path+file);
          loadObject.addConfigMap(m2);
        }
        else {
          fprintf(stderr, "URDFLoader: %s not yet implemented",
                  file_extension.c_str());
        }
      }
      loadObject.load();

      return entity;
    }

    /**
       * Loads a URDF, SMURF or SMURFS (SMURF scene) file.
       * @param filename The name of the file to be loaded as a full path.
       * @param tmpPath The path to the temporary directory to which files should be unzipped
       *        if the file format requires it.
       * @param robotname This parameter is obsolete with URDF and SMURF and was only
       *        needed for MARS scenes. It may be removed in the future.
       * @return 1 if the file was successfully loaded, 0 otherwise
       */
    //TODO: remove parameter "robotname"
    bool URDFLoader::loadFile(std::string filename, std::string tmpPath,
                              std::string robotname) {
      LOG_INFO("urdf_loader: prepare loading");

      // split up filename in path + _filename and retrieve file extension
      std::string file_extension = utils::getFilenameSuffix(filename);
      std::string path = utils::getPathOfFile(filename);
      std::string _filename = filename;
      utils::removeFilenamePrefix(&_filename);

      // need to unzip into a temporary directory
      if (file_extension == ".zsmurf" or file_extension == ".zsmurfs") {
        if (unzip(tmpPath, filename) == 0) {
          path = tmpPath;
          return 0;
        }
      }

      if (file_extension != ".urdf") { // if suffix is "smurf" or "smurfs"
        std::vector<utils::ConfigMap> smurfs; // a list of the smurfs found in a smurf scene
        utils::ConfigMap map;
        utils::ConfigVector::iterator it;
        fprintf(stderr, "Reading in %s...\n", (path+_filename).c_str());
        if(file_extension == ".smurfs") {
          // retrieve all smurfs from a smurf scene file
          map = utils::ConfigMap::fromYamlFile(path+_filename, true);
          map.toYamlFile("smurfs_debugmap.yml");
          for (it = map["smurfs"].begin(); it != map["smurfs"].end(); ++it) {
            (*it)["path"] = path;
            (*it)["type"] = "smurf";
            smurfs.push_back((*it).children);
          }
        } else if(file_extension == ".smurf") {
          // if we have only one smurf, only one with rudimentary data is added to the smurf list
            map["URI"] = _filename;
            map["name"] = "";
            smurfs.push_back(map);
        }
        fprintf(stderr, "Reading in smurfs...\n");
        for (std::vector<utils::ConfigMap>::iterator sit = smurfs.begin();
            sit != smurfs.end(); ++sit) {
          factoryManager->createEntity(*sit);
        }
      }
      else { // if file_extension is "urdf"
        uint mapIndex = 666;
        robotname="";
        Load loadObject(control, path, robotname, mapIndex);
        loadObject.parseURDF(filename);
        control->entities->addEntity(loadObject.getRobotname());
        //TODO: do we need to call loadObject.setEntityConfig here?
        loadObject.load();
      }

      return 1; //TODO: check number of successfully loaded entities before returning 1
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
