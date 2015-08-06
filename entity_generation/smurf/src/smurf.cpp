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

#include "smurf.h"

#include <QtXml>
#include <QDomNodeList>

#include <mars/data_broker/DataBrokerInterface.h>

#include <mars/interfaces/sim/EntityManagerInterface.h>
#include <mars/interfaces/sim/SimulatorInterface.h>
#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/sim/JointManagerInterface.h>
#include <mars/interfaces/sim/SensorManagerInterface.h>
#include <mars/interfaces/sim/MotorManagerInterface.h>
#include <mars/interfaces/sim/ControllerManagerInterface.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>
#include <mars/sim/SimEntity.h>
#include <mars/sim/SimMotor.h>
#include <mars/entity_generation/entity_factory/EntityFactoryManager.h>
#include <mars/interfaces/Logging.hpp>

#include <mars/utils/misc.h>
#include <mars/utils/mathUtils.h>
#include <smurf_parser/SMURFParser.h>

//#define DEBUG_PARSE_SENSOR 1

/*
 * remarks:
 *
 *   - we need some special handling because the representation
 *     in MARS is different then in URDF with is marked in the source with:
 *     ** special case handling **
 *
 *   - if we load and save a file we might lose names of
 *     collision and visual objects
 *
 */

namespace mars {
  namespace smurf {

    using namespace std;
    using namespace interfaces;
    using namespace utils;
    using namespace configmaps;

    SMURF::SMURF(lib_manager::LibManager *theManager): MarsPluginTemplate(theManager, "SMURF"),
                                                       entity_generation::EntityFactoryInterface("smurf, urdf") {
      entity_generation::EntityFactoryManager* factoryManager =
        theManager->acquireLibraryAs<mars::entity_generation::EntityFactoryManager>(
              "mars_entity_factory");
      factoryManager->registerFactory("smurf", this);
      factoryManager->registerFactory("urdf", this);
      theManager->releaseLibrary("mars_entity_factory");
    }

    SMURF::~SMURF(){
    }

    void SMURF::init() {
      control->sim->switchPluginUpdateMode(0, this);

      reset();
    }

    void SMURF::reset() {
      nextGroupID = control->nodes->getMaxGroupID() + 1;
      nextNodeID = 1;
      nextJointID = 1;
      nextMaterialID = 1;
      nextMotorID = 1;
      nextSensorID = 1;
      nextControllerID = 1;
      // TODO: this needs checking for potential doubling (see above nextGroupID)
      groupID = control->nodes->getMaxGroupID() + 1;

      nodeList.clear();
      jointList.clear();
      motorList.clear();
      sensorList.clear();
      controllerList.clear();
      materialList.clear();
      lightList.clear();
      graphicList.clear();

      linkIDMap.clear();
      nodeIDMap.clear();
      jointIDMap.clear();
      sensorIDMap.clear();
      motorIDMap.clear();
      materialMap.clear();
      collisionNameMap.clear();
      visualNameMap.clear();

      robotname = "";
      model.reset();

      entityconfig.clear();
      debugMap.clear();
    }

    void SMURF::update(sReal time_ms) {
    }

    void SMURF::handleURI(ConfigMap *map, std::string uri) {
      ConfigMap map2 = ConfigMap::fromYamlFile(uri);
      handleURIs(&map2);
      map->append(map2);
    }

    void SMURF::handleURIs(ConfigMap *map) {
      if (map->find("URI") != map->end()) {
        std::string file = (std::string) (*map)["URI"][0];
        if (!file.empty() && file[0] != '/') {
          file = tmpPath + file;
        }
        handleURI(map, file);
      }
      if (map->find("URIs") != map->end()) {
        ConfigVector::iterator vIt = (*map)["URIs"].begin();
        for (; vIt != (*map)["URIs"].end(); ++vIt) {
          std::string file = (std::string) (*vIt);
          if (!file.empty() && file[0] != '/') {
            file = tmpPath + file;
          }
          handleURI(map, file);
        }
      }
    }

    void SMURF::getSensorIDList(ConfigMap *map) {
      ConfigVector::iterator it;
      // TODO: check if objects exist in maps

      if (map->find("link") != map->end()) {
        (*map)["nodeID"] = nodeIDMap[(std::string) (*map)["link"][0]];
      }
      if (map->find("joint") != map->end()) {
        (*map)["jointID"] = jointIDMap[(std::string) (*map)["joint"][0]];
      }
      if (map->find("links") != map->end()) {
        for (it = (*map)["links"].begin(); it != (*map)["links"].end(); ++it) {
          (*map)["id"].push_back(ConfigItem(nodeIDMap[(std::string) *it]));
        }
      }
      if (map->find("joints") != map->end()) {
        for (it = (*map)["joints"].begin(); it != (*map)["joints"].end(); ++it) {
          (*map)["id"].push_back(ConfigItem(jointIDMap[(std::string) *it]));
        }
      }
      if (map->find("motors") != map->end()) {
        for (it = (*map)["motors"].begin(); it != (*map)["motors"].end(); ++it) {
          (*map)["id"].push_back(ConfigItem(motorIDMap[(std::string) *it]));
        }
      }
    }

    sim::SimEntity* SMURF::createEntity(const ConfigMap& config) {
      reset();
      sim::SimEntity* entity;
      entityconfig = config;
      std::string path = (std::string)entityconfig["path"];
      tmpPath = path;
      std::string filename = (std::string)entityconfig["file"];
      fprintf(stderr, "SMURF::createEntity: Creating entity of type %s\n", ((std::string)entityconfig["type"]).c_str());
      if((std::string)entityconfig["type"] == "smurf") {
        model = smurf_parser::parseFile(&entityconfig, path, filename, true);
        debugMap.append(entityconfig);
        // TODO: we should have a system that first loads the URDF and then the other files in
        //   order of priority (or sort the contents in a way as to avoid errors upon loading).

        entity = new sim::SimEntity(entityconfig);
        createModel();

        ConfigMap::iterator it;
        entityconfig.toYamlFile("entityconfig.yml");
        for (it = entityconfig.begin(); it != entityconfig.end(); ++it) {
            fprintf(stderr, "  ...loading smurf data section %s.\n", it->first.c_str());
            ConfigMap tmpconfig;
            tmpconfig[it->first] = it->second;
            addConfigMap(tmpconfig);
        }
      } else { // if type is "urdf"
        std::string urdfpath = path + (std::string) entityconfig["URI"];
        fprintf(stderr, "  ...loading urdf data from %s.\n", urdfpath.c_str());
        fprintf(stderr, "parsing model: %d", parseURDF(urdfpath));
        entity = new sim::SimEntity(entityconfig);
        createModel();
      }

      // node mapping and name checking
      std::string robotname = (std::string)entityconfig["name"];
      if (robotname == "")
        robotname = "unknown_robot";
      entityconfig["name"] = robotname;  //(std::string)entityconfig["modelname"];
      // add finalized config to entity
      entity->appendConfig(entityconfig);

      if(control->loadCenter->getMappedSceneByName(robotname) == 0) {
        control->loadCenter->setMappedSceneName(robotname);
      }
      mapIndex = control->loadCenter->getMappedSceneByName(robotname);
      fprintf(stderr, "mapIndex: %d\n", mapIndex);

      load();

      return entity;
    }

    void SMURF::addConfigMap(ConfigMap &config) {
      ConfigVector::iterator it;
      for (it = config["motors"].begin(); it != config["motors"].end(); ++it) {
        handleURIs(&it->children);
        (*it)["index"] = nextMotorID++;
        motorIDMap[(*it)["name"][0]] = nextMotorID - 1;
        (*it)["axis"] = 1;
        (*it)["jointIndex"] = jointIDMap[(*it)["joint"][0]];
        motorList.push_back((*it).children);
        debugMap["motors"] += (*it).children;
      }
      std::map<std::string, unsigned long> * idmap;
      std::map<std::string, std::string> *namemap;
      for (it = config["sensors"].begin(); it != config["sensors"].end(); ++it) {
        handleURIs(&it->children);
        ConfigMap tmpmap = it->children;
        tmpmap["attached_node"] = (ulong) nodeIDMap[(std::string) tmpmap["link"]];
        //FIXME: tmpmap["mapIndex"] = mapIndex;
        if ((std::string) tmpmap["type"] == "Joint6DOF") {
          std::string linkname = (std::string) tmpmap["link"];
          fprintf(stderr, "addConfig: %s\n", linkname.c_str());
          std::string jointname = model->getLink(linkname)->parent_joint->name;
          fprintf(stderr, "addConfig: %s\n", jointname.c_str());
          tmpmap["nodeID"] = (ulong) nodeIDMap[linkname];
          tmpmap["jointID"] = (ulong) jointIDMap[jointname];
          fprintf(stderr, "creating Joint6DOF..., %lu, %lu\n", (ulong) tmpmap["nodeID"],
              (ulong) tmpmap["jointID"]);
        }
        idmap = 0;
        namemap = 0;
        if (tmpmap.find("id") != tmpmap.end()) {
          ConfigVector tmpids;
          if (((std::string) tmpmap["type"]).find("Joint") != std::string::npos) {
            idmap = &jointIDMap;
          }
          if (((std::string) tmpmap["type"]).find("Node") != std::string::npos) {
            idmap = &nodeIDMap;
            if (((std::string) tmpmap["type"]).find("Contact") != std::string::npos)
              namemap = &collisionNameMap;
          }
          if (((std::string) tmpmap["type"]).find("Motor") != std::string::npos) {
            idmap = &motorIDMap;
          }
          for (ConfigVector::iterator idit = tmpmap["id"].begin();
              idit != tmpmap["id"].end(); ++idit) {
            if (idmap) {
              //(*idit) = (ulong)nodeIDMap[idit->getString()];
              if (namemap) {
                tmpids.push_back(ConfigItem((ulong) (*idmap)[(*namemap)[(std::string) (*idit)]]));
              } else {
                tmpids.push_back(ConfigItem((ulong) (*idmap)[(std::string) (*idit)]));
              }
            } else {
              fprintf(stderr, "Found sensor with id list, but of no known category.\n");
            }
          }
          tmpmap["id"] = tmpids;
        }
        tmpmap["index"] = nextSensorID++;
        sensorIDMap[tmpmap["name"][0]] = nextSensorID - 1;
        getSensorIDList(&tmpmap);
        sensorList.push_back(tmpmap);
        debugMap["sensors"] += tmpmap;
      }
      for (it = config["materials"].begin(); it != config["materials"].end(); ++it) {
        handleURIs(&it->children);
        std::vector<ConfigMap>::iterator mIt = materialList.begin();
        for (; mIt != materialList.end(); ++mIt) {
          if ((std::string) (*mIt)["name"][0] == (std::string) (*it)["name"][0]) {
            mIt->append(it->children);
            break;
          }
        }
      }
      for (it = config["nodes"].begin(); it != config["nodes"].end(); ++it) {
        handleURIs(&it->children);
        std::vector<ConfigMap>::iterator nIt = nodeList.begin();
        for (; nIt != nodeList.end(); ++nIt) {
          if ((std::string) (*nIt)["name"][0] == (std::string) (*it)["name"][0]) {
            ConfigMap::iterator cIt = it->children.begin();
            for (; cIt != it->children.end(); ++cIt) {
              (*nIt)[cIt->first] = cIt->second;
            }
            break;
          }
        }
      }
      for (it = config["joint"].begin(); it != config["joint"].end(); ++it) {
        handleURIs(&it->children);
        std::vector<ConfigMap>::iterator nIt = jointList.begin();
        for (; nIt != jointList.end(); ++nIt) {
          if ((std::string) (*nIt)["name"][0] == (std::string) (*it)["name"][0]) {
            ConfigMap::iterator cIt = it->children.begin();
            for (; cIt != it->children.end(); ++cIt) {
              (*nIt)[cIt->first] = cIt->second;
            }
            break;
          }
        }
      }

      for (it = config["visual"].begin(); it != config["visual"].end(); ++it) {
        handleURIs(&it->children);
        std::string cmpName = (std::string) (*it)["name"][0];
        std::vector<ConfigMap>::iterator nIt = nodeList.begin();
        if (visualNameMap.find(cmpName) != visualNameMap.end()) {
          cmpName = visualNameMap[cmpName];
          for (; nIt != nodeList.end(); ++nIt) {
            if ((std::string) (*nIt)["name"][0] == cmpName) {
              ConfigMap::iterator cIt = it->children.begin();
              for (; cIt != it->children.end(); ++cIt) {
                if (cIt->first != "name") {
                  (*nIt)[cIt->first] = cIt->second;
                }
              }
              break;
            }
          }
        }
      }

      for (it = config["collision"].begin(); it != config["collision"].end(); ++it) {
        handleURIs(&it->children);
        std::string cmpName = (std::string) (*it)["name"][0];
        std::vector<ConfigMap>::iterator nIt = nodeList.begin();
        if (collisionNameMap.find(cmpName) != collisionNameMap.end()) {
          cmpName = collisionNameMap[cmpName];
          for (; nIt != nodeList.end(); ++nIt) {
            if ((std::string) (*nIt)["name"][0] == cmpName) {
              ConfigMap::iterator cIt = it->children.begin();
              for (; cIt != it->children.end(); ++cIt) {
                if (cIt->first != "name") {
                  if (cIt->first == "bitmask") {
                    (*nIt)["coll_bitmask"] = (int) cIt->second;
                  } else {
                    (*nIt)[cIt->first] = cIt->second;
                  }
                }
              }
              break;
            }
          }
        }
      }

      for (it = config["lights"].begin(); it != config["lights"].end(); ++it) {
        handleURIs(&it->children);
        lightList.push_back((*it).children);
        debugMap["lights"] += (*it).children;
      }
      for (it = config["graphics"].begin(); it != config["graphics"].end(); ++it) {
        handleURIs(&it->children);
        graphicList.push_back((*it).children);
        debugMap["graphics"] += (*it).children;
      }
      for (it = config["controllers"].begin(); it != config["controllers"].end(); ++it) {
        handleURIs(&it->children);
        (*it)["index"] = nextControllerID++;
        // convert names to ids
        ConfigVector::iterator it2;
        if (it->children.find("sensors") != it->children.end()) {
          for (it2 = it->children["sensors"].begin(); it2 != it->children["sensors"].end(); ++it2) {
            it->children["sensorid"].push_back(ConfigItem(sensorIDMap[(std::string) *it2]));
          }
        }
        if (it->children.find("motors") != it->children.end()) {
          for (it2 = it->children["motors"].begin(); it2 != it->children["motors"].end(); ++it2) {
            it->children["motorid"] += ConfigItem(motorIDMap[(std::string) *it2]);
          }
        }
        controllerList.push_back((*it).children);
        debugMap["controllers"] += (*it).children;
      }
    }

    void SMURF::convertPose(const urdf::Pose &pose, const urdf::Pose &toPose, Vector *v,
        Quaternion *q) {
      urdf::Pose pose_ = pose;
      urdf::Pose toPose_ = toPose;
      urdf::Vector3 p;
      urdf::Rotation r;

      // we need the inverse of toPose_.position
      toPose_.position.x *= -1;
      toPose_.position.y *= -1;
      toPose_.position.z *= -1;
      p = pose_.position + toPose_.position;
      p = toPose_.rotation.GetInverse() * p;
      r = (toPose_.rotation.GetInverse() * pose_.rotation);
      *v = Vector(p.x, p.y, p.z);
      *q = quaternionFromMembers(r);
    }
    
    void SMURF::poseToVectorAndQuaternion(const urdf::Pose &pose, Vector *v, Quaternion*q) {
      *v = Vector(pose.position.x, pose.position.y, pose.position.z);
      *q = quaternionFromMembers(pose.rotation); //Quaternion(pose.rotation.x, pose.rotation.y, pose.rotation.z, pose.rotation.w);
    }
    
    

    bool SMURF::isEqualPos(const urdf::Pose &p1, const urdf::Pose p2) {
      bool equal = true;
      double epsilon = 0.00000000001;
      if (fabs(p1.position.x - p2.position.x) > epsilon)
        equal = false;
      if (fabs(p1.position.y - p2.position.y) > epsilon)
        equal = false;
      if (fabs(p1.position.z - p2.position.z) > epsilon)
        equal = false;
      if (fabs(p1.rotation.x - p2.rotation.x) > epsilon)
        equal = false;
      if (fabs(p1.rotation.y - p2.rotation.y) > epsilon)
        equal = false;
      if (fabs(p1.rotation.z - p2.rotation.z) > epsilon)
        equal = false;
      if (fabs(p1.rotation.w - p2.rotation.w) > epsilon)
        equal = false;
      return equal;
    }

    bool SMURF::isNullPos(const urdf::Pose &p) {
      bool zero = true;
      double epsilon = 0.0000000001;
      if (p.position.x > epsilon || p.position.x < -epsilon)
        zero = false;
      if (p.position.y > epsilon || p.position.y < -epsilon)
        zero = false;
      if (p.position.z > epsilon || p.position.z < -epsilon)
        zero = false;
      if (p.rotation.x > epsilon || p.position.x < -epsilon)
        zero = false;
      if (p.rotation.y > epsilon || p.position.y < -epsilon)
        zero = false;
      if (p.rotation.z > epsilon || p.position.z < -epsilon)
        zero = false;
      if (p.rotation.w > 1 + epsilon || p.rotation.w < 1 - epsilon)
        zero = false;
      return zero;
    }

    void SMURF::createEmptyVisualMaterial() {
      ConfigMap config;

      config["id"] = nextMaterialID++;
      config["name"] = "_emptyVisualMaterial";
      config["exists"] = true;
      config["diffuseFront"][0]["a"] = 1.0;
      config["diffuseFront"][0]["r"] = 1.0;
      config["diffuseFront"][0]["g"] = 0.0;
      config["diffuseFront"][0]["b"] = 0.0;
      config["texturename"] = "";
      config["cullMask"] = 0; // this makes the object invisible
      debugMap["materials"] += config;
      materialList.push_back(config);
    }
    
    void SMURF::createOriginMaterial() {
      ConfigMap config;

      config["id"] = nextMaterialID++;
      config["name"] = "_originMaterial";
      config["exists"] = true;
      config["diffuseFront"][0]["a"] = 1.0;
      config["diffuseFront"][0]["r"] = 0.0;
      config["diffuseFront"][0]["g"] = 0.0;
      config["diffuseFront"][0]["b"] = 1.0;
      config["texturename"] = "";
      config["cullMask"] = 0;
      debugMap["materials"] += config;
      materialList.push_back(config);
    }
    

    void SMURF::addEmptyVisualToNode(ConfigMap *map) {
      (*map)["origname"] = "";
      (*map)["materialName"] = "_emptyVisualMaterial";
    }

    void SMURF::addEmptyCollisionToNode(ConfigMap *map) {
      Vector size(0.01, 0.01, 0.01);
      (*map)["physicmode"] = "box";
      (*map)["coll_bitmask"] = 0;
      vectorToConfigItem(&(*map)["extend"][0], &size);
    }
    
    void SMURF::createOrigin(const boost::shared_ptr<urdf::Link> &link) {
      ConfigMap config;
      std::string name;
      if (link->name.empty()) {
        name = "link_"; // FIXME
      } else {
        name = link->name;
      }
      if (link->parent_joint) {
        config["relativeid"] = linkIDMap[link->parent_joint->parent_link_name];
      } else {
        config["relativeid"] = (unsigned long) 0;
      }
      
      // init node
      config["name"] = name;
      config["index"] = nextNodeID++;
      linkIDMap[name] = nextNodeID - 1;
      nodeIDMap[name] = nextNodeID - 1;
      currentNodeID = nextNodeID - 1;
      config["groupid"] = groupID;
      config["movable"] = true;
      
      // pose
      Vector v;
      Quaternion q;
      urdf::Pose pose;
      if (link->parent_joint)
        pose = link->parent_joint->parent_to_joint_origin_transform;
      poseToVectorAndQuaternion(pose, &v, &q);
      vectorToConfigItem(&config["position"][0], &v);
      quaternionToConfigItem(&config["rotation"][0], &q);
      
      // complete node
      addEmptyVisualToNode(&config);
      addEmptyCollisionToNode(&config);
      
      config["origname"] = "sphere";
      config["material"] = "_originMaterial";
      
      debugMap["nodes"] += config;      
      nodeList.push_back(config);
    }
    
    void SMURF::createInertial(const boost::shared_ptr<urdf::Link> &link) {
      ConfigMap config;
      std::string name;
      if (link->name.empty()) {
        name = "inertial_"; // FIXME
      } else {
        name = "inertial_" + link->name;
      }
      
      // init node
      config["name"] = name;
      config["index"] = nextNodeID++;
      nodeIDMap[name] = nextNodeID - 1;
      config["groupid"] = groupID;
      config["movable"] = true;
      config["relativeid"] = currentNodeID;
      
      // add inertial information
      config["density"] = 0.0;
      config["mass"] = link->inertial->mass;
      config["i00"] = link->inertial->ixx;
      config["i01"] = link->inertial->ixy;
      config["i02"] = link->inertial->ixz;
      config["i10"] = link->inertial->ixy;
      config["i11"] = link->inertial->iyy;
      config["i12"] = link->inertial->iyz;
      config["i20"] = link->inertial->ixz;
      config["i21"] = link->inertial->iyz;
      config["i22"] = link->inertial->izz;
      
      // pose
      Vector v;
      Quaternion q;
      urdf::Pose pose;
      pose = link->inertial->origin;
      poseToVectorAndQuaternion(pose, &v, &q);
      vectorToConfigItem(&config["position"][0], &v);
      quaternionToConfigItem(&config["rotation"][0], &q);      
      
      // complete node
      addEmptyVisualToNode(&config);
      addEmptyCollisionToNode(&config);
      
      debugMap["nodes"] += config;
      nodeList.push_back(config);        
    }    
    
    void SMURF::createCollision(const boost::shared_ptr<urdf::Collision> &collision) {
      ConfigMap config;
      std::string name;
      if (collision->name.empty()) {
        name = "collision_"; // FIXME
      } else {
        name = collision->name;
      }
      
      // init node
      config["name"] = name;
      config["index"] = nextNodeID++;
      nodeIDMap[name] = nextNodeID - 1;
      config["groupid"] = groupID;
      config["movable"] = true;
      config["relativeid"] = currentNodeID;
      config["mass"] = 0.001;
      config["density"] = 0.0;

      // parse geometry
      boost::shared_ptr<urdf::Geometry> tmpGeometry = collision->geometry;
      Vector size(0.0, 0.0, 0.0);
      Vector scale(1.0, 1.0, 1.0);
      urdf::Vector3 tmpV;
      switch (tmpGeometry->type) {
      case urdf::Geometry::SPHERE:
        size.x() = ((urdf::Sphere*) tmpGeometry.get())->radius;
        config["physicmode"] = "sphere";
        break;
      case urdf::Geometry::BOX:
        tmpV = ((urdf::Box*) tmpGeometry.get())->dim;
        size = Vector(tmpV.x, tmpV.y, tmpV.z);
        config["physicmode"] = "box";
        break;
      case urdf::Geometry::CYLINDER:
        size.x() = ((urdf::Cylinder*) tmpGeometry.get())->radius;
        size.y() = ((urdf::Cylinder*) tmpGeometry.get())->length;
        config["physicmode"] = "cylinder";
        break;
      case urdf::Geometry::MESH:
        tmpV = ((urdf::Mesh*) tmpGeometry.get())->scale;
        scale = Vector(tmpV.x, tmpV.y, tmpV.z);
        config["filename"] = ((urdf::Mesh*) tmpGeometry.get())->filename;
        config["origname"] = "";
        config["physicmode"] = "mesh";
        break;
      default:
        config["physicmode"] = "undefined";
        break;
      }
      vectorToConfigItem(&config["extend"][0], &size);
      vectorToConfigItem(&config["scale"][0], &scale);
      // FIXME: We need to correctly deal with scale and size in MARS
      //       if we have a mesh here, as a first hack we use the scale as size
      if (tmpGeometry->type == urdf::Geometry::MESH) {
        vectorToConfigItem(&config["extend"][0], &scale);
      }
      
      // pose
      Vector v;
      Quaternion q;
      poseToVectorAndQuaternion(collision->origin, &v, &q);
      vectorToConfigItem(&config["position"][0], &v);
      quaternionToConfigItem(&config["rotation"][0], &q);
      
      collisionNameMap[name] = name;
      addEmptyVisualToNode(&config);
      
      debugMap["nodes"] += config;
      nodeList.push_back(config);
    }    
    
    void SMURF::createVisual(const boost::shared_ptr<urdf::Visual> &visual) {
      ConfigMap config;
      std::string name;
      if (visual->name.empty()) {
        name = "visual_"; // FIXME
      } else {
        name = visual->name;
      }
      
      // init node
      config["name"] = name;
      config["index"] = nextNodeID++;
      nodeIDMap[name] = nextNodeID - 1;
      config["groupid"] = groupID;
      config["movable"] = true;
      config["relativeid"] = currentNodeID;
      config["mass"] = 0.001;
      config["density"] = 0.0;
      Vector v(0.001, 0.001, 0.001);
      vectorToConfigItem(&config["extend"][0], &v);
      
      // parse position
      Quaternion q;
      poseToVectorAndQuaternion(visual->origin, &v, &q);
      vectorToConfigItem(&config["position"][0], &v);
      quaternionToConfigItem(&config["rotation"][0], &q);
    
      // parse geometry    
      boost::shared_ptr<urdf::Geometry> tmpGeometry = visual->geometry;
      Vector size(0.0, 0.0, 0.0);
      Vector scale(1.0, 1.0, 1.0);
      urdf::Vector3 tmpV;
      config["filename"] = "PRIMITIVE";
      switch (tmpGeometry->type) {
      case urdf::Geometry::SPHERE:
        size.x() = ((urdf::Sphere*) tmpGeometry.get())->radius;
        config["origname"] = "sphere";
        break;
      case urdf::Geometry::BOX:
        tmpV = ((urdf::Box*) tmpGeometry.get())->dim;
        size = Vector(tmpV.x, tmpV.y, tmpV.z);
        config["origname"] = "box";
        break;
      case urdf::Geometry::CYLINDER:
        size.x() = ((urdf::Cylinder*) tmpGeometry.get())->radius;
        size.y() = ((urdf::Cylinder*) tmpGeometry.get())->length;
        config["origname"] = "cylinder";
        break;
      case urdf::Geometry::MESH:
        tmpV = ((urdf::Mesh*) tmpGeometry.get())->scale;
        scale = Vector(tmpV.x, tmpV.y, tmpV.z);
        config["filename"] = ((urdf::Mesh*) tmpGeometry.get())->filename;
        config["origname"] = "";
        break;
      default:
        break;
      }
      vectorToConfigItem(&config["visualsize"][0], &size);
      vectorToConfigItem(&config["visualscale"][0], &scale);
      config["materialName"] = visual->material_name;
      
      addEmptyCollisionToNode(&config);
      visualNameMap[name] = name;
      
      debugMap["nodes"] += config;
      nodeList.push_back(config);
    }

    void SMURF::translateLink(boost::shared_ptr<urdf::Link> link) {
      Vector v;
      Quaternion q;
      
      groupID++;
      
      createOrigin(link);
      
      if (link->parent_joint)
        translateJoint(link);

      // inertial
      if (link->inertial) {
        createInertial(link);
      }
      
      // collision
      if (link->collision) {
        for (std::vector<boost::shared_ptr<urdf::Collision> >::iterator it = link->collision_array.begin();
          it != link->collision_array.end(); ++it) {
            createCollision(*it);
        }
      }
      
      // visual
      if (link->visual) {
        for (std::vector<boost::shared_ptr<urdf::Visual> >::iterator it = link->visual_array.begin();
          it != link->visual_array.end(); ++it) {
            createVisual(*it);
        }
      }
      
      for (std::vector<boost::shared_ptr<urdf::Link> >::iterator it = link->child_links.begin();
          it != link->child_links.end(); ++it) {
          fprintf(stderr, "parsing link %s->%s..\n", (link->name).c_str(), ((*it)->name).c_str());
        translateLink(*it);
      }

    }
    
    void SMURF::translateJoint(boost::shared_ptr<urdf::Link> childlink) {
        ConfigMap config;
        boost::shared_ptr<urdf::Joint> joint = childlink->parent_joint;
        config["name"] = joint->name;
        config["index"] = nextJointID++;
        jointIDMap[joint->name] = nextJointID - 1;
        config["nodeindex1"] = nodeIDMap[joint->parent_link_name];
        config["nodeindex2"] = nodeIDMap[joint->child_link_name];
        config["anchorpos"] = 2; // always use the child_link as the anchor since joint and child_link are in the same frame
        // FIXME: reading in the limits was discarded until further notice as joint
        //   limits can lead ODE to become unstable
        // if (link->parent_joint->limits) {
        //   joint["lowStopAxis1"] = link->parent_joint->limits->lower;
        //   joint["highStopAxis1"] = link->parent_joint->limits->upper;
        // }
        // FIXME: we do not at this point read the joint "maxeffort" and "maxvelocity"
        // limits as they are effectively motor values and should be used only
        // if there are no explicit motor values defined
        if (joint->type == urdf::Joint::REVOLUTE
            || joint->type == urdf::Joint::CONTINUOUS) {
          config["type"] = "hinge";
        } else if (joint->type == urdf::Joint::PRISMATIC) {
          config["type"] = "slider";
        } else if (joint->type == urdf::Joint::FIXED) {
          config["type"] = "fixed";
        } else {
          // we don't support the type yet and use a fixed joint
          config["type"] = "fixed";
        }

        // transform the joint's axis into global coordinates
        urdf::Pose pose = getGlobalPose(childlink);
        urdf::Pose axispose;
//        axispose.position = childlink->parent_joint->parent_to_joint_origin_transform.rotation * joint->axis;
        axispose.position = pose.rotation * joint->axis;
        Vector v;
        v = Vector(axispose.position.x, axispose.position.y, axispose.position.z);
        vectorToConfigItem(&config["axis1"][0], &v);

        // add to debug and joint list
        debugMap["joints"] += config;
        jointList.push_back(config);
    }
    
    urdf::Pose SMURF::getGlobalPose(const boost::shared_ptr<urdf::Link> &link) {
      urdf::Pose globalPose;
      boost::shared_ptr<urdf::Link> pLink = link->getParent();
      if (link->parent_joint) {
        globalPose = link->parent_joint->parent_to_joint_origin_transform;
      }
      if (pLink) {
        urdf::Pose parentPose = getGlobalPose(pLink);
        globalPose.position = parentPose.rotation * globalPose.position;
        globalPose.position = globalPose.position + parentPose.position;
        globalPose.rotation = parentPose.rotation * globalPose.rotation;
      }
      return globalPose;
    }    

    void SMURF::createMaterial(boost::shared_ptr<urdf::Material> material) {
      ConfigMap config;

      config["id"] = nextMaterialID++;
      config["name"] = material->name;
      config["exists"] = true;
      config["diffuseFront"][0]["a"] = (double) material->color.a;
      config["diffuseFront"][0]["r"] = (double) material->color.r;
      config["diffuseFront"][0]["g"] = (double) material->color.g;
      config["diffuseFront"][0]["b"] = (double) material->color.b;
      config["texturename"] = material->texture_filename;
      
      // add to debug and material list
      debugMap["materials"] += config;
      materialList.push_back(config);
    }

    unsigned int SMURF::parseURDF(std::string filename) {
      QString xmlErrorMsg = "";

      //creating a handle for the xmlfile
      QFile file(filename.c_str());

      QLocale::setDefault(QLocale::C);

      LOG_INFO("SMURF: smurfing scene: %s", filename.c_str());

      //test to open the xmlfile
      if (!file.open(QIODevice::ReadOnly)) {
        std::cout << "Error while opening scene file content " << filename
            << " in SMURF.cpp->parseScene" << std::endl;
        std::cout << "Make sure your scenefile name corresponds to"
            << " the name given to the enclosed .scene file" << std::endl;
        return 0;
      }

      model = urdf::parseURDFFile(filename);
      if (!model) {
        return 0;
      }
    }

    void SMURF::createModel() {

      if (robotname == "") {
        robotname = model.get()->name_;
      }

      createEmptyVisualMaterial();
      createOriginMaterial();
      std::map<std::string, boost::shared_ptr<urdf::Material> >::iterator it;
      for (it = model->materials_.begin(); it != model->materials_.end(); ++it) {
        createMaterial(it->second);
      }

      translateLink(model->root_link_);
    }

    unsigned int SMURF::load() {
      fprintf(stderr, "smurfing robot: %s...\n", robotname.c_str());
      debugMap.toYamlFile("debugMap.yml");

      for (unsigned int i = 0; i < materialList.size(); ++i)
        if (!loadMaterial(materialList[i]))
          return 0;
      for (unsigned int i = 0; i < nodeList.size(); ++i)
        if (!loadNode(nodeList[i])) {
          fprintf(stderr, "Couldn't load node %d, %s..\n'", (unsigned long)nodeList[i]["index"], ((std::string)nodeList[i]["name"]).c_str());
          return 0;
        }
          

      for (unsigned int i = 0; i < jointList.size(); ++i)
        if (!loadJoint(jointList[i]))
          return 0;

      for (unsigned int i = 0; i < motorList.size(); ++i)
        if (!loadMotor(motorList[i]))
          return 0;

      for (std::map<unsigned long, std::string>::iterator it = mimicmotors.begin();
        it != mimicmotors.end(); ++it)
          loadMimic(it->first, it->second);

      for (unsigned int i = 0; i < sensorList.size(); ++i)
        if (!loadSensor(sensorList[i]))
          return 0;

      for (unsigned int i = 0; i < controllerList.size(); ++i)
        if (!loadController(controllerList[i]))
          return 0;

      for (unsigned int i = 0; i < lightList.size(); ++i)
        if (!loadLight(lightList[i]))
          return 0;

      for (unsigned int i = 0; i < graphicList.size(); ++i)
        if (!loadGraphic(graphicList[i]))
          return 0;

      setPose();

      control->nodes->printNodeMasses(true);

      return 1;
    }

    void SMURF::setPose() {
      core_objects_exchange node;
      uint nodeid = control->loadCenter->getMappedID(nodeIDMap[model->root_link_->name],
          MAP_TYPE_NODE, mapIndex);
      fprintf(stderr, "placing: %s, %d\n", (model->root_link_->name).c_str(), nodeid);
      control->nodes->getNodeExchange(nodeid, &node);
      Quaternion tmpQ(1, 0, 0, 0);
      Vector tmpR;
      Vector tmpV;
      tmpV[0] = entityconfig["position"][0];
      tmpV[1] = entityconfig["position"][1];
      tmpV[2] = entityconfig["position"][2];
      // check if euler angles or quaternion is provided; rotate around z if only one angle is provided
      switch (entityconfig["rotation"].size()) {
        case 1: tmpR[0] = 0;
                tmpR[1] = 0;
                tmpR[2] = entityconfig["rotation"][0];
                tmpQ = eulerToQuaternion(tmpR);
                break;
        case 3: tmpR[0] = entityconfig["rotation"][0];
                tmpR[1] = entityconfig["rotation"][1];
                tmpR[2] = entityconfig["rotation"][2];
                tmpQ = eulerToQuaternion(tmpR);
                break;
        case 4: tmpQ.x() = (sReal)entityconfig["rotation"][1];
                tmpQ.y() = (sReal)entityconfig["rotation"][2];
                tmpQ.z() = (sReal)entityconfig["rotation"][3];
                tmpQ.w() = (sReal)entityconfig["rotation"][0];
                break;
      }


      NodeData my_node;
      my_node.index = nodeid;
      my_node.pos = tmpV;
      my_node.rot = tmpQ;
      control->nodes->editNode(&my_node, EDIT_NODE_POS | EDIT_NODE_MOVE_ALL);
      control->nodes->editNode(&my_node, EDIT_NODE_ROT | EDIT_NODE_MOVE_ALL);

      if((std::string)entityconfig["anchor"] == "world") {
        fprintf(stderr, "Anchor robot to world...\n");
        // create non-movable anchor node
        NodeData node;
        node.init("anchor_" + robotname, tmpV, tmpQ);
        node.initPrimitive(interfaces::NODE_TYPE_BOX, Vector(0.01, 0.01, 0.01), 0.01);
        // FIXME: we need to check the below line
	node.groupID = 
        node.index = 666666666; // unlikely that anyone will ever use this within a model
        node.c_params.coll_bitmask = 0;
        NodeId oldId = node.index;
        NodeId newId = control->nodes->addNode(&node);
        control->loadCenter->setMappedID(oldId, newId, MAP_TYPE_NODE, mapIndex);
        if (robotname != "") {
          control->entities->addNode(robotname, node.index, node.name);
        }

        // create fixed joint between anchor node and root node
        JointData joint;
        joint.init("joint_anchor_" + robotname, interfaces::JOINT_TYPE_FIXED,
                              nodeid, newId);
        joint.index = 666666666;
        JointId newJointId = control->joints->addJoint(&joint);
        if (!newId) {
          LOG_ERROR("addJoint returned 0");
        }
        control->loadCenter->setMappedID(joint.index, newJointId, MAP_TYPE_JOINT, mapIndex);

        if (robotname != "") {
          control->entities->addJoint(robotname, joint.index, joint.name);
        }
      }

      // set Joints
      configmaps::ConfigVector::iterator it;
      configmaps::ConfigMap::iterator joint_it;
      for (it = entityconfig["poses"].begin(); it!= entityconfig["poses"].end(); ++it) {
        if ((std::string)(*it)["name"] == (std::string)entityconfig["pose"]) {
          for (joint_it = (*it)["joints"][0].children.begin(); joint_it!= (*it)["joints"][0].children.end(); ++joint_it) {
            fprintf(stderr, "setMotorValue: joint: %s, id: %i, value: %f\n", ((std::string)joint_it->first).c_str(), motorIDMap[joint_it->first], (double)joint_it->second);
            control->motors->setMotorValue(motorIDMap[joint_it->first], joint_it->second);
          }
        }
      }
    }

    unsigned int SMURF::loadNode(ConfigMap config) {
      NodeData node;
      config["mapIndex"].push_back(ConfigItem(mapIndex));
      int valid = node.fromConfigMap(&config, tmpPath, control->loadCenter);
      if (!valid)
        return 0;

      if ((std::string) config["materialName"][0] != std::string("")) {
        std::map<std::string, MaterialData>::iterator it;
        it = materialMap.find(config["materialName"][0]);
        if (it != materialMap.end()) {
          node.material = it->second;
        }
      } else {
        node.material.diffuseFront = Color(0.4, 0.4, 0.4, 1.0);
      }

      // check if meshes are stored as `.stl` file
      string suffix = getFilenameSuffix(node.filename);
      if (suffix == ".stl" || suffix == ".STL") {
        // add an additional rotation of -90.0 degree due to wrong definition
        // of which direction is up within .stl (for .stl -Y is up and in MARS
        // Z is up)
        node.visual_offset_rot *= eulerToQuaternion(Vector(-90.0, 0.0, 0.0));
      }

      NodeId oldId = node.index;
      config.toYamlFile("SMURFNode.yml");
      NodeId newId = control->nodes->addNode(&node);
      if (!newId) {
        LOG_ERROR("addNode returned 0");
        return 0;
      }
      control->loadCenter->setMappedID(oldId, newId, MAP_TYPE_NODE, mapIndex);
      if (robotname != "") {
        control->entities->addNode(robotname, node.index, node.name);
      }
      return 1;
    }

    unsigned int SMURF::loadMaterial(ConfigMap config) {
      MaterialData material;

      int valid = material.fromConfigMap(&config, tmpPath);
      materialMap[config["name"][0]] = material;

      return valid;
    }

    unsigned int SMURF::loadJoint(ConfigMap config) {
      JointData joint;
      joint.invertAxis = true;
      config["mapIndex"].push_back(ConfigItem(mapIndex));
      int valid = joint.fromConfigMap(&config, tmpPath, control->loadCenter);
      if (!valid) {
        fprintf(stderr, "SMURF: error while smurfing joint\n");
        return 0;
      }

      JointId oldId = joint.index;
      JointId newId = control->joints->addJoint(&joint);
      if (!newId) {
        LOG_ERROR("addJoint returned 0");
        return 0;
      }
      control->loadCenter->setMappedID(oldId, newId, MAP_TYPE_JOINT, mapIndex);

      if (robotname != "") {
        control->entities->addJoint(robotname, joint.index, joint.name);
      }
      return true;
    }

    unsigned int SMURF::loadMotor(ConfigMap config) {
      MotorData motor;
      config["mapIndex"].push_back(ConfigItem(mapIndex));

      int valid = motor.fromConfigMap(&config, tmpPath, control->loadCenter);
      if (!valid) {
        fprintf(stderr, "SMURF: error while smurfing motor\n");
        return 0;
      }

      MotorId oldId = motor.index;
      MotorId newId = control->motors->addMotor(&motor);
      if (!newId) {
        LOG_ERROR("addMotor returned 0");
        return 0;
      }
      control->loadCenter->setMappedID(oldId, newId, MAP_TYPE_MOTOR, mapIndex);

      if (robotname != "") {
        control->entities->addMotor(robotname, motor.index, motor.name);
      }

      sim::SimMotor* newMotor = control->motors->getSimMotor(newId);

      // set motor mimics
      if (config.find("mimic_motor") != config.end()) {
        mimicmotors[newId] = (std::string)config["mimic_motor"];
        newMotor->setMimic(
          (sReal)config["mimic_multiplier"], (sReal)config["mimic_offset"]);
      }

      // set approximation functions
      if (config.find("maxeffort_approximation") != config.end()) {
        std::vector<sReal>* maxeffort_coefficients = new std::vector<sReal>;
        ConfigVector::iterator vIt = config["maxeffort_coefficients"].begin();
        for (; vIt != config["maxeffort_coefficients"].end(); ++vIt) {
          maxeffort_coefficients->push_back((double)(*vIt));
          newMotor->setMaxEffortApproximation(
            utils::approximationFunctionMap[(std::string)config["maxeffort_approximation"]],
            maxeffort_coefficients);
        }
      }
      if (config.find("maxspeed_approximation") != config.end()) {
        fprintf(stderr, "found maxspeed_approximation in %s\n", ((std::string)config["name"]).c_str());
        std::vector<sReal>* maxspeed_coefficients = new std::vector<sReal>;
        ConfigVector::iterator vIt = config["maxspeed_coefficients"].begin();
        for (; vIt != config["maxspeed_coefficients"].end(); ++vIt) {
          maxspeed_coefficients->push_back((double)(*vIt));
          newMotor->setMaxSpeedApproximation(
            utils::approximationFunctionMap[(std::string)config["maxspeed_approximation"]],
            maxspeed_coefficients);
        }
      }

      return true;
    }

    void SMURF::loadMimic(unsigned long mimicId, std::string parent_name) {
      sim::SimMotor* parentmotor =
        control->motors->getSimMotorByName(parent_name);
      if (parentmotor != NULL)
        fprintf(stderr, ", %s\n", parentmotor->getName().c_str());
      else
        fprintf(stderr, "no parentmotor found\n");
      parentmotor->addMimic(control->motors->getSimMotor(mimicId));
    }

    BaseSensor* SMURF::loadSensor(ConfigMap config) {
      config["mapIndex"].push_back(ConfigItem(mapIndex));
      BaseSensor *sensor = control->sensors->createAndAddSensor(&config);
      if (sensor != 0) {
        control->loadCenter->setMappedID((ulong) config["index"], sensor->getID(), MAP_TYPE_SENSOR,
            mapIndex);
      }

      return sensor;
    }

    unsigned int SMURF::loadGraphic(ConfigMap config) {
      GraphicData graphic;
      config["mapIndex"].push_back(ConfigItem(mapIndex));
      int valid = graphic.fromConfigMap(&config, tmpPath, control->loadCenter);
      if (!valid) {
        fprintf(stderr, "SMURF: error while smurfing graphic\n");
        return 0;
      }

      if (control->graphics)
        control->graphics->setGraphicOptions(graphic);

      return 1;
    }

    unsigned int SMURF::loadLight(ConfigMap config) {
      LightData light;
      config["mapIndex"].push_back(ConfigItem(mapIndex));
      int valid = light.fromConfigMap(&config, tmpPath, control->loadCenter);
      if (!valid) {
        fprintf(stderr, "SMURF: error while smurfing light\n");
        return 0;
      }
      control->sim->addLight(light);
      return true;
    }

    unsigned int SMURF::loadController(ConfigMap config) {
      ControllerData controller;
      config["mapIndex"].push_back(ConfigItem(mapIndex));

      int valid = controller.fromConfigMap(&config, tmpPath, control->loadCenter);
      if (!valid) {
        fprintf(stderr, "SMURF: error while smurfing Controller\n");
        return 0;
      }

      MotorId oldId = controller.id;
      MotorId newId = control->controllers->addController(controller);
      if (!newId) {
        LOG_ERROR("SMURF: addController returned 0");
        return 0;
      }
      control->loadCenter->setMappedID(oldId, newId, MAP_TYPE_CONTROLLER, mapIndex);
      if (robotname != "") {
        control->entities->addController(robotname, newId);
      }
      return 1;
    }

    std::string SMURF::getRobotname() {
      return robotname;
    }

  }  // end of namespace smurf
}  // end of namespace mars

DESTROY_LIB(mars::smurf::SMURF);
CREATE_LIB(mars::smurf::SMURF);
