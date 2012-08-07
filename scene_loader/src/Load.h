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
#include <mars/utils/ConfigData.h>
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

    private:
      // Every new load scene gets a new hack id, which will be multiplied with
      // 1000 and added to all group_ids to prevent that object loaded from
      // the same scenefile (file twice loaded) are connected through the
      // group_ids. This hack is fine as long as one scenefile contains group-
      // ids less then 1000. Oh, and a scene can only be x times saved without
      // over running the group_id range (unsigned long).
      static unsigned long hack_ids;

      unsigned int unzip(const std::string& destinationDir,
                         const std::string& zipFilename);

      void getGenericConfig(utils::ConfigMap *config, QDomElement * elementNode);
      unsigned int parseScene(const std::string& sceneDirectory,
                              const std::string& sceneFilename,
                              unsigned int mapIndex);

      std::map<unsigned long, interfaces::MaterialData> materials;

      /**
       * Name of the file which should be opened (including the extension .scn or .scene).
       */
      std::string mFileName;

      /**
       * Name of the robot for the Robotmanager
       */
      std::string mRobotName;

      interfaces::ControlCenter *control;
      std::string tmpPath;

      /**
       * unsigned int parseNode(QDomElement * elementNode, NodeData * p_node)
       *
       * is used for parsing the passed QDom structure for node entries.
       * outsourced for reasons of readability.
       *
       * /pre: there has to be a pointer to a QDomElement and one to a NodeData
       *        the QDomElement includes a root element named nodelist
       * /post: none
       * /important: not to be used or called outside Load.cpp
       */
      unsigned int parseNode(QDomElement *elementNode, unsigned int mapIndex);
      unsigned int parseJoint(QDomElement *elementNode, unsigned int mapIndex);
      unsigned int parseMotor(QDomElement *elementNode, unsigned int mapIndex);
      unsigned int parseLight(QDomElement *elementNode, unsigned int mapIndex);
      unsigned int parseGraphic(QDomElement *elementNode, unsigned int mapIndex);
      unsigned int parseController(QDomElement *elementNode,
                                   unsigned int mapIndex);

      interfaces::BaseSensor* parseSensor(QDomElement *elementNode,
                                          unsigned int mapIndex);

      unsigned int parseMaterial(QDomElement *elementNode,
                                 unsigned int mapIndex);

    };

  } // end of namespace scene_loader
} // end of namespace mars

#endif  // LOAD_H
