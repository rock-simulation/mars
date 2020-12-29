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
 * \author Malte Roemmermann
 *
 */

#ifndef LOAD_SCENE_INTERFACE_H
#define LOAD_SCENE_INTERFACE_H

#ifdef _PRINT_HEADER_
  #warning "LoaderInterface.h"
#endif

#include <string>

#include <lib_manager/LibInterface.hpp>
#include <mars/utils/Vector.h>

namespace mars {

  namespace interfaces {

    class LoadSceneInterface : public lib_manager::LibInterface {
    public:
      LoadSceneInterface(lib_manager::LibManager *theManager) :
        lib_manager::LibInterface(theManager) {}
      virtual ~LoadSceneInterface() {}

      virtual bool loadFile(std::string filename, std::string tmpPath,
                            std::string robotname) = 0;
      virtual bool loadFile(std::string filename, std::string tmpPath,
                            std::string robotname, utils::Vector pos, utils::Vector rot) = 0;
      virtual int saveFile(std::string filename, std::string tmpPath) = 0;
    };

  } // end of namespace interfaces
} // end of namespace mars

#endif //LOAD_SCENE_INTERFACE_H
