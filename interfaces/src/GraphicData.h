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

#ifndef MARS_INTERFACES_GRAPHIC_DATA_H
#define MARS_INTERFACES_GRAPHIC_DATA_H

#include "MARSDefs.h"
#include <mars/utils/Color.h>
#include <mars/utils/ConfigData.h>

namespace mars {
  namespace interfaces {

    // forward declaration
    class LoadSceneInterface;

    class GraphicData {
    public:
      GraphicData();

      bool fromConfigMap(utils::ConfigMap *config, std::string filenamePrefix,
                         LoadSceneInterface *loadScene = 0);
      void toConfigMap(utils::ConfigMap *config,
                       bool skipFilenamePrefix = false);
      void getFilesToSave(std::vector<std::string> *fileList);

      utils::Color clearColor;
      bool fogEnabled;
      sReal fogDensity;
      sReal fogStart;
      sReal fogEnd;
      utils::Color fogColor;
    };

  } // end of namespace interfaces
} // end of namespace mars

#endif /* MARS_INTERFACES_GRAPHIC_DATA_H */
