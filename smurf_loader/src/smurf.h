/*
 *  Copyright 2011, 2012, 2014, DFKI GmbH Robotics Innovation Center
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

#include <yaml-cpp/yaml.h>

//#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/utils/ConfigData.h>
#include <mars/interfaces/sensor_bases.h>
#include <mars/interfaces/MaterialData.h>

#include <urdf_parser/urdf_parser.h>
#include <boost/function.hpp>
#include <urdf_model/model.h>
#include <urdf_model/link.h>
#include <urdf_model/joint.h>
#include <urdf_model/pose.h>

namespace mars {
  namespace urdf_loader {

    class Load {
    public:
      Load(interfaces::ControlCenter *control, std::string tmpPath,
           std::string robotname, unsigned int mapIndex);

      /**
       * @return 0 on error.
       */
      unsigned int parseURDF(std::string filename);
      unsigned int load();
      void addConfigMap(utils::ConfigMap &config);
      void setEntityConfig(const utils::ConfigMap &config);
      std::string getRobotname();

      std::vector<utils::ConfigMap> materialList;
      std::vector<utils::ConfigMap> nodeList;
      std::vector<utils::ConfigMap> jointList;
      std::vector<utils::ConfigMap> motorList;
      std::vector<utils::ConfigMap> sensorList;
      std::vector<utils::ConfigMap> controllerList;
      std::vector<utils::ConfigMap> graphicList;
      std::vector<utils::ConfigMap> lightList;

    private:
      int nextGroupID;
      unsigned long nextNodeID;
      unsigned long nextJointID;
      unsigned long nextMaterialID;
      unsigned long nextMotorID;
      unsigned long nextSensorID;
      unsigned long nextControllerID;
      std::map<std::string, unsigned long> nodeIDMap;
      std::map<std::string, unsigned long> jointIDMap;
      std::map<std::string, unsigned long> sensorIDMap;
      std::map<std::string, unsigned long> motorIDMap;
      std::map<std::string, interfaces::MaterialData> materialMap;
      std::map<std::string, std::string> visualNameMap, collisionNameMap;
      interfaces::ControlCenter *control;
      std::string tmpPath;
      //std::map<std::string, std::string> smurffiles;
      unsigned int mapIndex; // index of loaded scenes
      utils::ConfigMap debugMap;
      utils::ConfigMap entityconfig;
      std::string robotname;
      boost::shared_ptr<urdf::ModelInterface> model;

      void handleURI(utils::ConfigMap *map, std::string uri);
      void handleURIs(utils::ConfigMap *map);
      void getSensorIDList(utils::ConfigMap *map);

      void handleInertial(utils::ConfigMap *map,
                          const boost::shared_ptr<urdf::Link> &link);
      void calculatePose(utils::ConfigMap *map,
                         const boost::shared_ptr<urdf::Link> &link);
      void convertPose(const urdf::Pose &pose,
                       const boost::shared_ptr<urdf::Link> &link,
                       utils::Vector *v, utils::Quaternion *q);
      void convertPose(const urdf::Pose &pose, const urdf::Pose &toPose,
                       utils::Vector *v, utils::Quaternion *q);
      bool isEqualPos(const urdf::Pose &p1, const urdf::Pose p2);

      void handleVisual(utils::ConfigMap *map,
                        const boost::shared_ptr<urdf::Visual> &visual);
      void handleCollision(utils::ConfigMap *map,
                           const boost::shared_ptr<urdf::Collision> &c);

      void handleKinematics(boost::shared_ptr<urdf::Link> curlink);

      void handleMaterial(boost::shared_ptr<urdf::Material> material);

      void createFakeMaterial();
      void createFakeVisual(utils::ConfigMap *map);
      void createFakeCollision(utils::ConfigMap *map);

      void setPose();

      unsigned int loadMaterial(utils::ConfigMap config);
      unsigned int loadNode(utils::ConfigMap config);
      unsigned int loadJoint(utils::ConfigMap config);
      unsigned int loadMotor(utils::ConfigMap config);
      interfaces::BaseSensor* loadSensor(utils::ConfigMap config);
      unsigned int loadController(utils::ConfigMap config);
      unsigned int loadGraphic(utils::ConfigMap config);
      unsigned int loadLight(utils::ConfigMap config);

      urdf::Pose getGlobalPose(const boost::shared_ptr<urdf::Link> &link);
    };

  } // end of namespace urdf_loader
} // end of namespace mars

#endif  // LOAD_H
