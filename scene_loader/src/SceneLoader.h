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
 * \file SceneLoader.h
 * \author Malte Roemmermann
 */
#ifndef SCENE_LOADER_H
#define SCENE_LOADER_H

#ifdef _PRINT_HEADER_
  #warning "SceneLoader.h"
#endif

#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/interfaces/sim/LoadCenter.h>
#include <mars/interfaces/sim/LoadSceneInterface.h>
#include "SaveLoadStructs.h"

namespace mars {
  namespace scene_loader {

    class SceneLoader : public interfaces::LoadSceneInterface {
      
    public:
      SceneLoader(lib_manager::LibManager *theManager);
      ~SceneLoader();


      // LibInterface methods
      int getLibVersion() const {return 1;}
      const std::string getLibName() const {return std::string("mars_scene_loader");}
      CREATE_MODULE_INFO();

      virtual bool loadFile(std::string filename, std::string tmpPath,
                            std::string robotname);

      virtual bool loadFile(std::string filename, std::string tmpPath,
                            std::string robotname, utils::Vector pos, utils::Vector rot);
      
      virtual int saveFile(std::string filename, std::string tmpPath);

    private:
      interfaces::ControlCenter *control;
    };

  } // end of namespace scene_loader
} // end of namespace mars

#endif  // SCENE_LOADER_H
