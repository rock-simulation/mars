/*
 *  Copyright 2011, 2012, DFKI GmbH Robotics Innovation Center
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

#ifndef MARS_INTERFACES_CONTROLLER_DATA_H
#define MARS_INTERFACES_CONTROLLER_DATA_H

#include <vector>
#include <string>

#include "MARSDefs.h"
#include <mars/utils/ConfigData.h>

namespace mars {
  namespace interfaces {

    // forward declaration
    class LoadSceneInterface;

    class ControllerData {
    public:
      ControllerData();

      bool fromConfigMap(utils::ConfigMap *config, std::string filenamePrefix,
                         LoadSceneInterface *loadScene = 0);
      void toConfigMap(utils::ConfigMap *config,
                       bool skipFilenamePrefix = false);
      void getFilesToSave(std::vector<std::string> *fileList);

      unsigned long id;
      sReal rate;
      std::vector<unsigned long> motors;
      std::vector<unsigned long> sensors;
      std::vector<unsigned long> sNodes;
      std::string dylib_path;
    }; // end of class ControllerData

  } // end of namespace interfaces
} // end of namespace mars


#endif /* MARS_INTERFACES_CONTROLLER_DATA_H */
