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

#ifndef SMURF_H
#define SMURF_H

#ifdef _PRINT_HEADER_
#warning "smurf.h"
#endif

#include <map>

#include <yaml-cpp/yaml.h>

#include <configmaps/ConfigData.h>
#include <mars/interfaces/sensor_bases.h>
#include <mars/interfaces/MaterialData.h>

#include <mars/interfaces/sim/MarsPluginTemplate.h>
#include <mars/entity_generation/entity_factory/EntityFactoryInterface.h>

#include <urdf_parser/urdf_parser.h>
#include <boost/function.hpp>
#include <urdf_model/model.h>
#include <urdf_model/link.h>
#include <urdf_model/joint.h>
#include <urdf_model/pose.h>

namespace mars {

  namespace smurf {

    class SMURF: public interfaces::MarsPluginTemplate,
        public mars::entity_generation::EntityFactoryInterface {

    public:
      SMURF(lib_manager::LibManager *theManager);
      ~SMURF();

      // LibInterface methods
      int getLibVersion() const {return 1;}
      const std::string getLibName() const {return std::string("SMURF");}
      CREATE_MODULE_INFO();

      /**
       * @return 0 on error.
       */
      void createModel();
      unsigned int parseURDF(std::string filename);
      unsigned int load();
      void addConfigMap(configmaps::ConfigMap &config);
      std::string getRobotname();

      // EntityFactoryInterface
      virtual sim::SimEntity* createEntity(const configmaps::ConfigMap& config);

      // MarsPlugin methods
      void init();
      void reset();
      void update(mars::interfaces::sReal time_ms);

      std::vector<configmaps::ConfigMap> materialList;
      std::vector<configmaps::ConfigMap> nodeList;
      std::vector<configmaps::ConfigMap> jointList;
      std::vector<configmaps::ConfigMap> motorList;
      std::vector<configmaps::ConfigMap> sensorList;
      std::vector<configmaps::ConfigMap> controllerList;
      std::vector<configmaps::ConfigMap> graphicList;
      std::vector<configmaps::ConfigMap> lightList;

    private:
      int groupID;
      unsigned int mapIndex; // index to map nodes of a single entity
      unsigned long nextNodeID;
      unsigned long nextGroupID;
      unsigned long currentNodeID;
      unsigned long nextJointID;
      unsigned long nextMaterialID;
      unsigned long nextMotorID;
      unsigned long nextSensorID;
      unsigned long nextControllerID;
      std::map<std::string, unsigned long> linkIDMap;
      std::map<std::string, unsigned long> nodeIDMap;
      std::map<std::string, unsigned long> jointIDMap;
      std::map<std::string, unsigned long> sensorIDMap;
      std::map<std::string, unsigned long> motorIDMap;
      std::map<std::string, interfaces::MaterialData> materialMap;
      std::map<std::string, std::string> visualNameMap, collisionNameMap;
      std::string tmpPath;
      //std::map<std::string, std::string> smurffiles;
      configmaps::ConfigMap debugMap;
      configmaps::ConfigMap entityconfig;
      std::string robotname;
      boost::shared_ptr<urdf::ModelInterface> model;
      sim::SimEntity* entity;

      void handleURI(configmaps::ConfigMap *map, std::string uri);
      void handleURIs(configmaps::ConfigMap *map);
      void getSensorIDList(configmaps::ConfigMap *map);

      // creating URDF objects
      void translateLink(boost::shared_ptr<urdf::Link> link); // handleKinematics
      void translateJoint(boost::shared_ptr<urdf::Link> childlink); // handleKinematics
      void createMaterial(const boost::shared_ptr<urdf::Material> material); // handleMaterial
      void createOrigin(const boost::shared_ptr<urdf::Link> &link);
      void createInertial(const boost::shared_ptr<urdf::Link> &link);
      void createVisual(const boost::shared_ptr<urdf::Visual> &visual);
      void createCollision(const boost::shared_ptr<urdf::Collision> &collision);
      void addEmptyVisualToNode(configmaps::ConfigMap *map); // createFakeVisual
      void addEmptyCollisionToNode(configmaps::ConfigMap *map); // createFakeCollision
      void createEmptyVisualMaterial();
      void createOriginMaterial();
      
      // geometry calculations
      urdf::Pose getGlobalPose(const boost::shared_ptr<urdf::Link> &link);
      void calculatePose(configmaps::ConfigMap *map, const boost::shared_ptr<urdf::Link> &link);
      void convertPose(const urdf::Pose &pose, const urdf::Pose &toPose, utils::Vector *v,
          utils::Quaternion *q);
      void poseToVectorAndQuaternion(const urdf::Pose &pose, utils::Vector *v, utils::Quaternion *q);
      bool isEqualPos(const urdf::Pose &p1, const urdf::Pose p2);
      bool isNullPos(const urdf::Pose &p);

      // load functions
      unsigned int loadMaterial(configmaps::ConfigMap config);
      unsigned int loadNode(configmaps::ConfigMap config);
      unsigned int loadJoint(configmaps::ConfigMap config);
      unsigned int loadMotor(configmaps::ConfigMap config);
      interfaces::BaseSensor* loadSensor(configmaps::ConfigMap config);
      unsigned int loadController(configmaps::ConfigMap config);
      unsigned int loadGraphic(configmaps::ConfigMap config);
      unsigned int loadLight(configmaps::ConfigMap config);


    };

  } // end of namespace smurf
} // end of namespace mars

#endif  // SMURF_H

