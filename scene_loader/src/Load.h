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

#ifndef LOAD_H
#define LOAD_H

#ifdef _PRINT_HEADER_
  #warning "Load.h"
#endif

#include <map>

#include <mars/interfaces/sim/ControlCenter.h>
#include <configmaps/ConfigData.h>
#include <mars/interfaces/sensor_bases.h>
#include <mars/interfaces/MaterialData.h>

class QDomElement;

namespace mars {
  namespace scene_loader {

    class Load {
    public:
      Load(std::string fileName, interfaces::ControlCenter *control,
           std::string tmpPath_, const std::string &robotname="");

      /**
       * @return 0 on error.
       */
      unsigned int load();

      unsigned int prepareLoad();
      unsigned int parseScene();
      unsigned int loadScene();

      std::map<unsigned long, interfaces::MaterialData> materials;
      std::vector<configmaps::ConfigMap> materialList;
      std::vector<configmaps::ConfigMap> nodeList;
      std::vector<configmaps::ConfigMap> jointList;
      std::vector<configmaps::ConfigMap> motorList;
      std::vector<configmaps::ConfigMap> sensorList;
      std::vector<configmaps::ConfigMap> controllerList;
      std::vector<configmaps::ConfigMap> graphicList;
      std::vector<configmaps::ConfigMap> lightList;

    private:
      // Every new load scene gets an offset, which is added to all group_ids
      // to prevent that object loaded from the same scenefile (file twice
      // loaded) are connected through the group_ids.
      unsigned long groupIDOffset;
      bool useYAML;

      unsigned int unzip(const std::string& destinationDir,
                         const std::string& zipFilename);

      void getGenericConfig(std::vector<configmaps::ConfigMap> *configList,
                            const QDomElement &elementNode);
      void getGenericConfig(configmaps::ConfigMap *config,
                            const QDomElement &elementNode);

      unsigned int parseYamlScene();


      /**
       * Name of the file which should be opened (including the extension .smurf).
       */
      std::string mFileName;
      /**
       * File extension of mFileName
       */
      std::string mFileSuffix;

      /**
       * Name of the robot for the Robotmanager
       */
      std::string mRobotName;

      interfaces::ControlCenter *control;
      std::string tmpPath;
      std::string sceneFilename;
      unsigned int mapIndex;

      unsigned int loadMaterial(configmaps::ConfigMap config);
      unsigned int loadNode(configmaps::ConfigMap config);
      unsigned int loadJoint(configmaps::ConfigMap config);
      unsigned int loadMotor(configmaps::ConfigMap config);
      interfaces::BaseSensor* loadSensor(configmaps::ConfigMap config);
      unsigned int loadController(configmaps::ConfigMap config);
      unsigned int loadGraphic(configmaps::ConfigMap config);
      unsigned int loadLight(configmaps::ConfigMap config);
      void checkEncodings();
    };

  } // end of namespace scene_loader
} // end of namespace mars

#endif  // LOAD_H
