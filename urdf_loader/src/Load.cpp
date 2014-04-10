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

#include "Load.h"
#include "zipit.h"

#include <QtXml>
#include <QDomNodeList>

#include <mars/data_broker/DataBrokerInterface.h>

#include <mars/interfaces/sim/SimulatorInterface.h>
#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/sim/JointManagerInterface.h>
#include <mars/interfaces/sim/SensorManagerInterface.h>
#include <mars/interfaces/sim/MotorManagerInterface.h>
#include <mars/interfaces/sim/ControllerManagerInterface.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>

#include <mars/interfaces/sim/EntityManagerInterface.h>
#include <mars/interfaces/sim/LoadSceneInterface.h>
#include <mars/utils/misc.h>
#include <mars/utils/mathUtils.h>

//#define DEBUG_PARSE_SENSOR 1

namespace mars {
  namespace urdf_loader {

    using namespace std;
    using namespace interfaces;
    using namespace utils;

    Load::Load(std::string fileName, ControlCenter *c, std::string tmpPath_,
               const std::string &robotname) :
      mFileName(fileName), mRobotName(robotname),
      control(c), tmpPath(tmpPath_) {

      mFileSuffix = getFilenameSuffix(mFileName);
    }

    unsigned int Load::load() {

      if (!prepareLoad())
        return 0;
      if (!parseScene())
        return 0;
      return loadScene();
    }

    unsigned int Load::prepareLoad() {
      std::string filename = mFileName;

      nextGroupID = control->nodes->getMaxGroupID()+1;
      nextNodeID = 1;
      nextJointID = 1;
      nextMaterialID = 1;

      if (mRobotName != "") {
        control->entities->addEntity(mRobotName);
      }

      LOG_INFO("urdf_loader: prepare loading");

      // need to unzip into a temporary directory
      if (mFileSuffix == ".zsmurf") {
        if (unzip(tmpPath, mFileName) == 0) {
          return 0;
        }
        mFileSuffix = ".smurf";
      } else {
        // can parse file without unzipping
        tmpPath = getPathOfFile(mFileName);
      }

      removeFilenamePrefix(&filename);
      removeFilenameSuffix(&filename);

      mapIndex = control->loadCenter->getMappedSceneByName(mFileName);
      if (mapIndex == 0) {
        control->loadCenter->setMappedSceneName(mFileName);
        mapIndex = control->loadCenter->getMappedSceneByName(mFileName);
      }
      sceneFilename = tmpPath + filename + mFileSuffix;
      return 1;
    }

    unsigned int Load::unzip(const std::string& destinationDir,
                             const std::string& zipFilename) {
      if (!createDirectory(destinationDir))
        return 0;

      Zipit myZipFile(zipFilename);
      LOG_INFO("Load: unsmurfing zipped SMURF: %s", zipFilename.c_str());

      if (!myZipFile.unpackWholeZipTo(destinationDir))
        return 0;

      return 1;
    }

    void Load::handleInertial(ConfigMap *map,
                              const boost::shared_ptr<urdf::Link> &link) {
      if(link->inertial) {
        (*map)["mass"] = link->inertial->mass;
        // handle inertial
        (*map)["i00"] = link->inertial->ixx;
        (*map)["i01"] = link->inertial->ixy;
        (*map)["i02"] = link->inertial->ixz;
        (*map)["i10"] = link->inertial->ixy;
        (*map)["i11"] = link->inertial->iyy;
        (*map)["i12"] = link->inertial->iyz;
        (*map)["i20"] = link->inertial->ixz;
        (*map)["i21"] = link->inertial->iyz;
        (*map)["i22"] = link->inertial->izz;
        (*map)["inertia"] = true;
      }
      else {
        (*map)["inertia"] = false;
      }
    }

    void Load::calculatePosition(ConfigMap *map,
                                 const boost::shared_ptr<urdf::Link> &link) {
      urdf::Pose jointPose, parentInertialPose, inertialPose;
      urdf::Pose goalPose;

      if(link->parent_joint) {
        jointPose = link->parent_joint->parent_to_joint_origin_transform;
        if(link->getParent()->inertial) {
          parentInertialPose = link->getParent()->inertial->origin;
        }
        unsigned long parentID = nodeIDMap[link->getParent()->name];
        (*map)["relativeid"] = parentID;
      }
      else {
        (*map)["relativeid"] = 0ul;
      }

      if(link->inertial) {
        inertialPose = link->inertial->origin;
      }

      // we need the inverse of parentInertialPose.position
      parentInertialPose.position.x *= -1;
      parentInertialPose.position.y *= -1;
      parentInertialPose.position.z *= -1;

      goalPose.position = jointPose.position + parentInertialPose.position;
      goalPose.position = (goalPose.position +
                           jointPose.rotation * inertialPose.position);
      goalPose.position = (parentInertialPose.rotation.GetInverse()*
                           goalPose.position);
      goalPose.rotation = (parentInertialPose.rotation.GetInverse()*
                           inertialPose.rotation*jointPose.rotation);

      Vector v(goalPose.position.x, goalPose.position.y, goalPose.position.z);
      vectorToConfigItem(&(*map)["position"][0], &v);
      Quaternion q = quaternionFromMembers(goalPose.rotation);
      quaternionToConfigItem(&(*map)["rotation"][0], &q);
    }

    void Load::handleVisual(ConfigMap *map,
                            const boost::shared_ptr<urdf::Link> &link) {
      if(link->visual) {
        boost::shared_ptr<urdf::Geometry> tmpGeometry = link->visual->geometry;
        Vector size(0.0, 0.0, 0.0);
        Vector scale(1.0, 1.0, 1.0);
        urdf::Vector3 v;
        (*map)["filename"] = "PRIMITIVE";
        switch (tmpGeometry->type) {
        case urdf::Geometry::SPHERE:
          size.x() = ((urdf::Sphere*)tmpGeometry.get())->radius;
          (*map)["origname"] ="sphere";
          break;
        case urdf::Geometry::BOX:
          v = ((urdf::Box*)tmpGeometry.get())->dim;
          size = Vector(v.x, v.y, v.z);
          (*map)["origname"] = "box";
          break;
        case urdf::Geometry::CYLINDER:
          size.x() = ((urdf::Cylinder*)tmpGeometry.get())->radius;
          size.y() = ((urdf::Cylinder*)tmpGeometry.get())->length;
          (*map)["origname"] = "cylinder";
          break;
        case urdf::Geometry::MESH:
          v = ((urdf::Mesh*)tmpGeometry.get())->scale;
          scale = Vector(v.x, v.y, v.z);
          (*map)["filename"] = ((urdf::Mesh*)tmpGeometry.get())->filename;
          (*map)["origname"] = "";
          break;
        default:
          break;
        }
        vectorToConfigItem(&(*map)["visualsize"][0], &size);
        vectorToConfigItem(&(*map)["visualscale"][0], &scale);
        (*map)["materialName"] = link->visual->material_name;

        // caculate visual position offset
        urdf::Pose inertialPose, visualPose = link->visual->origin;
        urdf::Pose goalPose;
        if(link->inertial) {
          inertialPose = link->inertial->origin;
        }

        // we need the inverse of inertialPose.position
        inertialPose.position.x *= -1;
        inertialPose.position.y *= -1;
        inertialPose.position.z *= -1;
        goalPose.position = visualPose.position + inertialPose.position;
        goalPose.position = inertialPose.rotation * goalPose.position;
        goalPose.rotation = (inertialPose.rotation.GetInverse() *
                             visualPose.rotation);
        Vector v1(goalPose.position.x, goalPose.position.y,
                  goalPose.position.z);
        vectorToConfigItem(&(*map)["visualposition"][0], &v1);
        Quaternion q = quaternionFromMembers(goalPose.rotation);
        quaternionToConfigItem(&(*map)["visualrotation"][0], &q);
      }
    }

    void Load::handleKinematics(boost::shared_ptr<urdf::Link> link) {
      ConfigMap config;

      config["name"] = link->name;
      config["index"] = nextNodeID++;

      nodeIDMap[link->name] = nextNodeID-1;

      if(link->visual_array.size() < 2 &&
         link->collision_array.size() < 2) {
        config["groupid"] = 0;
      }
      else {
        // we need to group mars nodes
        config["groupid"] = nextGroupID++;
      }

      // we need some special handling because the representation
      // in MARS is different then in URDF

      handleInertial(&config, link);
      calculatePosition(&config, link);
      handleVisual(&config, link);

      //handle visual
      boost::shared_ptr<urdf::Visual> tmpVisual = link->visual;


      //handle collision
      //config["physicmode"] = collision.type
      //config["extend"] = boundingbox.extend
      //config["movable"]
      //handle joint/parent information
      config["physicmode"] = "box";
      Vector v(0.1, 0.1, 0.1);
      vectorToConfigItem(&config["extend"][0], &v);

      debugMap["links"] += config;
      nodeList.push_back(config);
      for (std::vector<boost::shared_ptr<urdf::Link> >::iterator it =
             link->child_links.begin(); it != link->child_links.end(); ++it) {
        handleKinematics(*it); //TODO: check if this is correct with shared_ptr
      }
    }

    void Load::handleMaterial(boost::shared_ptr<urdf::Material> material) {
      ConfigMap config;

      config["id"] = nextMaterialID++;
      config["name"] = material->name;
      config["exists"] = true;
      config["diffuseFront"][0]["a"] = (double)material->color.a;
      config["diffuseFront"][0]["r"] = (double)material->color.r;
      config["diffuseFront"][0]["g"] = (double)material->color.g;
      config["diffuseFront"][0]["b"] = (double)material->color.b;
      config["texturename"] = material->texture_filename;
      debugMap["materials"] += config;
      materialList.push_back(config);
    }


    unsigned int Load::parseScene() {
      //  HandleFileNames h_filenames;
      vector<string> v_filesToLoad;
      QString xmlErrorMsg = "";

      //creating a handle for the xmlfile
      QFile file(sceneFilename.c_str());

      QLocale::setDefault(QLocale::C);

      LOG_INFO("Load: loading scene: %s", sceneFilename.c_str());

      //test to open the xmlfile
      if (!file.open(QIODevice::ReadOnly)) {
        std::cout << "Error while opening scene file content " << sceneFilename
                  << " in Load.cpp->parseScene" << std::endl;
        std::cout << "Make sure your scenefile name corresponds to"
                  << " the name given to the enclosed .scene file" << std::endl;
        return 0;
      }


      boost::shared_ptr<urdf::ModelInterface> model;
      model = urdf::parseURDFFile(sceneFilename);
      if (!model) {
        return 0;
      }

      handleKinematics(model->root_link_);

      std::map<std::string, boost::shared_ptr<urdf::Material> >::iterator it;
      for(it=model->materials_.begin(); it!=model->materials_.end(); ++it) {
        handleMaterial(it->second);
      }

      debugMap.toYamlFile("debugMap.yml");

      //    //the entire tree recursively anyway
      //    std::vector<boost::shared_ptr<urdf::Link>> urdflinklist;
      //    std::vector<boost::shared_ptr<urdf::Joint>> urdfjointlist;
      //

      //    model.getJoints(urdfjointlist);
      //    for (std::vector<boost::shared_ptr<urdf::Link>>::iterator it =
      //            urdfjointlist.begin(); it != urdfjointlist.end(); ++it) {
      //        getGenericConfig(&jointList, it);
      //    }


      return 1;
    }

    unsigned int Load::loadScene() {

      for (unsigned int i = 0; i < materialList.size(); ++i)
        if(!loadMaterial(materialList[i]))
          return 0;
      for (unsigned int i = 0; i < nodeList.size(); ++i)
        if (!loadNode(nodeList[i]))
          return 0;

      // for (unsigned int i = 0; i < jointList.size(); ++i)
      //   if (!loadJoint(jointList[i]))
      //     return 0;

      return 1;
    }

    unsigned int Load::loadNode(utils::ConfigMap config) {
      NodeData node;
      config["mapIndex"].push_back(utils::ConfigItem(mapIndex));
      int valid = node.fromConfigMap(&config, tmpPath, control->loadCenter);
      if (!valid)
        return 0;

      if((std::string)config["materialName"][0] != std::string("")) {
        std::map<std::string, MaterialData>::iterator it;
        it = materialMap.find(config["materialName"][0]);
        if (it != materialMap.end()) {
          node.material = it->second;
        }
      }

      NodeId oldId = node.index;
      NodeId newId = control->nodes->addNode(&node);
      if (!newId) {
        LOG_ERROR("addNode returned 0");
        return 0;
      }
      control->loadCenter->setMappedID(oldId, newId, MAP_TYPE_NODE, mapIndex);
      if (mRobotName != "") {
        control->entities->addNode(mRobotName, node.index, node.name);
      }
      return 1;
    }

    unsigned int Load::loadMaterial(utils::ConfigMap config) {
      MaterialData material;

      int valid = material.fromConfigMap(&config, tmpPath);
      materialMap[config["name"][0]] = material;

      return valid;
    }


  }// end of namespace urdf_loader
}
// end of namespace mars
