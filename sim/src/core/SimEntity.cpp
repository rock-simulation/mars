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

#include "SimEntity.h"
#include "SimJoint.h"
#include <configmaps/ConfigData.h>
#include <iostream>
#include <mars/utils/mathUtils.h>
#include <mars/utils/misc.h>
#include <mars/interfaces/NodeData.h>
#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/interfaces/sim/SimulatorInterface.h>
#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/sim/JointManagerInterface.h>
#include <mars/interfaces/sim/MotorManagerInterface.h>
#include <mars/interfaces/sim/SensorManagerInterface.h>
#include <mars/interfaces/sim/ControllerManagerInterface.h>
#include <mars/interfaces/sim/EntityManagerInterface.h>
#include <math.h>
#include <float.h>
#include <iterator> // ostream_iterator

namespace mars {
  using namespace interfaces;
  namespace sim {

    SimEntity::SimEntity(const std::string &name) : name(name), control(NULL),
                                                    selected(false) {
    }

    SimEntity::SimEntity(const configmaps::ConfigMap& parameters) : control(NULL),
                                                                    selected(false) {
      config = parameters;
      this->name = (std::string) config["name"];
    }

    SimEntity::SimEntity(ControlCenter *c,
                         const std::string &name) : name(name), control(c),
                                                    selected(false) {
    }

    SimEntity::SimEntity(ControlCenter *c,
                         const configmaps::ConfigMap& parameters) : control(c),
                                                                    selected(false) {
      config = parameters;
      this->name = (std::string) config["name"];
    }

    void SimEntity::appendConfig(const configmaps::ConfigMap& parameters) {
      configmaps::ConfigMap map = parameters;
      config.append(map);
    }

    void SimEntity::removeEntity() {
      for (auto it = nodeIds.begin(); it != nodeIds.end(); ++it) {
        std::vector<unsigned long> joints = control->joints->getIDsByNodeID(it->first);
        for (auto j: joints) control->joints->removeJoint(j);
        control->nodes->removeNode(it->first);
      }
      nodeIds.clear();

      for (auto it = jointIds.begin(); it != jointIds.end(); ++it) {
         control->joints->removeJoint(it->first);
      }
      jointIds.clear();
      if (hasAnchorJoint()) control->joints->removeJoint(anchorJointId);

      for (auto it = motorIds.begin(); it != motorIds.end(); ++it) {
         control->motors->removeMotor(it->first);
      }
      motorIds.clear();

      for (auto it = sensorIds.begin(); it != sensorIds.end(); ++it) {
         control->sensors->removeSensor(it->first);
      }
      sensorIds.clear();

      for (auto it = controllerIds.begin(); it != controllerIds.end(); ++it) {
         control->controllers->removeController(*it);
      }
      controllerIds.clear();
    }

    void SimEntity::addNode(unsigned long nodeId, const std::string& name) {
      nodeIds[nodeId] = name;
    }

    void SimEntity::addMotor(unsigned long motorId, const std::string& name) {
      motorIds[motorId] = name;
    }

    void SimEntity::addController(long unsigned int controllerId) {
      controllerIds.push_back(controllerId);
    }

    void SimEntity::addJoint(long unsigned int jointId, const std::string& name) {
      jointIds[jointId] = name;
    }

    void SimEntity::addSensor(long unsigned int sensorId, const std::string& name) {
      sensorIds[sensorId] = name;
    }

    bool SimEntity::select(unsigned long nodeId) {
      //check if the node belongs to the robot
      //   std::map<unsigned long, std::string>::iterator it = nodeIds.find(nodeId);
      //node belongs to the robot
      if (nodeIds.count(nodeId)) {
        //no need to check if it has been previously selected as std::set does that
        selectedNodes.insert(nodeId);
        return true;
      } else {
        return false;
      }
    }

    std::string SimEntity::getAssembly() {
      if (config.hasKey("assembly")) {
        return (std::string) config["assembly"];
      }
      return "";
    }

    long unsigned int SimEntity::getRootestId(std::string name_specifier /*="" */) {
      unsigned int id_specified = INVALID_ID;
      unsigned int id_lowest = INVALID_ID;
      for (std::map<unsigned long, std::string>::const_iterator iter = nodeIds.begin();
        iter != nodeIds.end(); ++iter) {
        if (iter->first <= id_lowest || (id_lowest == INVALID_ID && iter->first != INVALID_ID)) {
          id_lowest = iter->first;
          if ((iter->first <= id_specified || (id_specified == INVALID_ID && iter->first != INVALID_ID)) && utils::matchPattern(name_specifier, iter->second)) {
              id_specified = iter->first;
          }
        }
      }
      if (id_specified != INVALID_ID) {
        return id_specified;
      } else if (id_specified == INVALID_ID && id_lowest != INVALID_ID) {
        if (name_specifier != "") {
          fprintf(stderr, "Warning: No Node with name_specifier '%s' found while SimEntity::getRootestId() returning %d\n", name_specifier.c_str(), id_lowest);
        }
        return id_lowest;
      }
      fprintf(stderr, "ERROR: No Node found while SimEntity::getRootestId()\n");
      return INVALID_ID;
    }

    std::map<unsigned long, std::string> SimEntity::getAllNodes() {
      return nodeIds;
    }

    std::vector<unsigned long> SimEntity::getNodes(const std::string& name) {
      std::vector<unsigned long> out;
      for (std::map<unsigned long, std::string>::const_iterator iter = nodeIds.begin();
          iter != nodeIds.end(); ++iter) {
        if (utils::matchPattern(name, iter->second)) {
          out.push_back(iter->first);
        }
      }
      return out;
    }

    unsigned long SimEntity::getNode(const std::string& name) {
      for (std::map<unsigned long, std::string>::const_iterator iter = nodeIds.begin();
          iter != nodeIds.end(); ++iter) {
        if (iter->second == name)
          return iter->first;
      }
      return 0;
    }

    std::string SimEntity::getNode(unsigned long id) {
      //TODO problem if node does not exist
      return nodeIds.find(id)->second;
    }

    void SimEntity::getBoundingBox(utils::Vector &center, utils::Quaternion &rotation, utils::Vector &extent) {
      utils::Vector maxVertex(-DBL_MAX, -DBL_MAX, -DBL_MAX);
      utils::Vector minVertex(DBL_MAX, DBL_MAX, DBL_MAX);
      NodeData root = control->nodes->getFullNode(getRootestId());
      NodeData node;
      for (std::map<unsigned long, std::string>::const_iterator iter = nodeIds.begin();
          iter != nodeIds.end(); ++iter) {
        if (!control->nodes->exists(iter->first)) {
          continue;
        }
        node = control->nodes->getFullNode(iter->first);
        utils::Vector vertices[8] = {
          node.ext,
          utils::Vector(-node.ext[0], node.ext[1], node.ext[2]),
          utils::Vector(node.ext[0], -node.ext[1], node.ext[2]),
          utils::Vector(node.ext[0], node.ext[1], -node.ext[2]),
          -node.ext,
          utils::Vector(node.ext[0], -node.ext[1], -node.ext[2]),
          utils::Vector(-node.ext[0], node.ext[1], -node.ext[2]),
          utils::Vector(-node.ext[0], -node.ext[1], node.ext[2])
        };
        for(int v=0;v<8;v++) {
          vertices[v] /= 2;
          vertices[v] = node.rot.toRotationMatrix() * vertices[v];
          vertices[v] += node.pos;
          //till here the bounding box is world frame
          //now we transform to entity frame
          vertices[v] -= root.pos;
          vertices[v] = root.rot.toRotationMatrix().transpose() * vertices[v];
          //now we calculate the extent
          for(int i=0;i<3;i++) {
            maxVertex[i] = fmax(vertices[v][i],maxVertex[i]);
            minVertex[i] = fmin(vertices[v][i],minVertex[i]);
          }
        }
      }
      extent = maxVertex - minVertex;
      center = (maxVertex + minVertex) / 2;
      //transform center to world frame
      center += root.pos;
      rotation = root.rot;
    }

    /**returns the vertices of the boundingbox
    */
    void SimEntity::getBoundingBox(std::vector<utils::Vector> &vertices, utils::Vector& center) {
      vertices.clear();
      utils::Quaternion rotation;
      utils::Vector extent;
      this->getBoundingBox(center,rotation,extent);
      vertices.push_back(extent);
      vertices.push_back(utils::Vector(-extent.x(), extent.y(), extent.z()));
      vertices.push_back(utils::Vector(extent.x(), -extent.y(), extent.z()));
      vertices.push_back(utils::Vector(extent.x(), extent.y(), -extent.z()));
      vertices.push_back(-extent);
      vertices.push_back(utils::Vector(extent.x(), -extent.y(), -extent.z()));
      vertices.push_back(utils::Vector(-extent.x(), extent.y(), -extent.z()));
      vertices.push_back(utils::Vector(-extent.x(), -extent.y(), extent.z()));
      for (int i=0; i<8; i++) {
        vertices[i] *= 0.5;
        vertices[i] = rotation * vertices[i];
        vertices[i] += center;
      }
    }

    long unsigned int SimEntity::getMotor(const std::string& name) {
      for (std::map<unsigned long, std::string>::const_iterator iter = motorIds.begin();
          iter != motorIds.end(); ++iter) {
        if (iter->second == name)
          return iter->first;
      }
      return 0;
    }

    std::string SimEntity::getMotor(long unsigned int id) {
      //TODO problem if node does not exist
      return motorIds.find(id)->second;
    }

    long unsigned int SimEntity::getJoint(const std::string& name) {
      for (std::map<unsigned long, std::string>::const_iterator iter = jointIds.begin();
          iter != jointIds.end(); ++iter) {
        if (iter->second == name)
          return iter->first;
      }
      return 0;
    }

    std::string SimEntity::getJoint(long unsigned int id) {
      //TODO problem if node does not exist
      return jointIds.find(id)->second;
    }

    const ConfigMap SimEntity::getConfig() {
      return config;
    }

    std::vector<unsigned long> SimEntity::getControllerList() {
      return controllerIds;
    }

    void SimEntity::printNodes() {
      std::cout << "Nodes of Robot " << name << "with id: " << ":\n";
      for (std::map<unsigned long, std::string>::const_iterator iter = nodeIds.begin();
          iter != nodeIds.end(); ++iter) {
        std::cout << iter->first << '\t' << iter->second << '\n';
      }
      std::cout << std::endl;
    }

    void SimEntity::printMotors() {
      std::cout << "Motors of Robot " << name << ":\n";
      for (std::map<unsigned long, std::string>::const_iterator iter = motorIds.begin();
          iter != motorIds.end(); ++iter) {
        std::cout << iter->first << '\t' << iter->second << '\n';
      }
      std::cout << std::endl;
    }

    void SimEntity::printControllers() {
      std::cout << "Controllers of Robot " << name << ":\n";
      for (size_t i = 0; i < controllerIds.size(); i++) {
        std::cout << controllerIds.at(i) << std::endl;
      }
    }

    bool SimEntity::hasAnchorJoint() {
      return (anchorJointId != 0);
    }

    void SimEntity::setInitialPose(bool reset/*compatibility*/, configmaps::ConfigMap* pPoseCfg/*=nullptr*/) {
      if(control && (config.find("rootNode") != config.end())) {
        NodeId id = getNode((std::string)config["rootNode"]);
        if (!control->nodes->exists(id)) {
          fprintf(stderr, "ERROR: Did not find node id %lu in setInitialPose()\n", id);
          return;
        }
        NodeData rootNode = control->nodes->getFullNode(id);
        utils::Quaternion tmpQ(1, 0, 0, 0);
        utils::Vector tmpV;
        configmaps::ConfigMap cfg = config;
        if (pPoseCfg != nullptr) {
          if (pPoseCfg->hasKey("rotation")) cfg["rotation"] = (*pPoseCfg)["rotation"];
          if (pPoseCfg->hasKey("position")) cfg["position"] = (*pPoseCfg)["position"];
          cfg["anchor"] = pPoseCfg->get("anchor", (std::string) "none");
          cfg["parent"] = pPoseCfg->get("parent", (std::string) "world");
          cfg["pose"] = pPoseCfg->get("pose", (std::string) "");
        }
        //parent defines the frame for position and rotation
        // check if there is a parent
        std::string parentname = "world";
        unsigned long parentid = 0;
        utils::Vector parentpos(0,0,0);
        utils::Quaternion parentrot(1,0,0,0);
        if (cfg.hasKey("parent") && (std::string)cfg["parent"] != "none" && (std::string)cfg["parent"] != "") { // backwards compatibility
          parentname << cfg["parent"];
        }
        if (!parentname.empty() && parentname != "world") {
          std::string entityname, linkname;
          unsigned int splitsignpos = parentname.find("::");
          if (splitsignpos >= 0) {
            entityname = parentname.substr(0, splitsignpos);
            linkname = parentname.substr(splitsignpos+2);
            SimEntity* parententity = control->entities->getEntity(entityname);
            if (parententity != 0) {
              unsigned long linkid = parententity->getNode(linkname);
              if (linkid > 0) {
                parentid = linkid;
                parentpos = control->nodes->getPosition(parentid);
                parentrot = control->nodes->getRotation(parentid);
              } else
                fprintf(stderr, "No valid link id for '%s' in parent entity '%s' .\n", linkname.c_str(), entityname.c_str());
            } else
              fprintf(stderr, "No valid parent entity '%s' in scene.\n", entityname.c_str());
          } else
            fprintf(stderr, "No valid parent specified by parent: '%s'\n", parentname.c_str());
        }
        if(cfg.find("position") != cfg.end()) {
          rootNode.pos.x() = cfg["position"][0];
          rootNode.pos.y() = cfg["position"][1];
          rootNode.pos.z() = cfg["position"][2];
        }
        if(cfg.find("rotation") != cfg.end()) {
          // check if euler angles or quaternion is provided; rotate around z
          // if only one angle is provided
          switch (cfg["rotation"].size()) {
          case 1: tmpV[0] = 0;
            tmpV[1] = 0;
            tmpV[2] = cfg["rotation"][0];
            tmpQ = utils::eulerToQuaternion(tmpV);
            break;
          case 3: tmpV[0] = cfg["rotation"][0];
            tmpV[1] = cfg["rotation"][1];
            tmpV[2] = cfg["rotation"][2];
            tmpQ = utils::eulerToQuaternion(tmpV);
            break;
          case 4: tmpQ.x() = (sReal)cfg["rotation"][1];
            tmpQ.y() = (sReal)cfg["rotation"][2];
            tmpQ.z() = (sReal)cfg["rotation"][3];
            tmpQ.w() = (sReal)cfg["rotation"][0];
            break;
          }
          rootNode.rot = tmpQ;
        }
        rootNode.pos = parentrot * rootNode.pos + parentpos;
        control->nodes->editNode(&rootNode, EDIT_NODE_POS | EDIT_NODE_MOVE_ALL);
        rootNode.rot = parentrot * rootNode.rot;
        control->nodes->editNode(&rootNode, EDIT_NODE_ROT | EDIT_NODE_MOVE_ALL);

        //anchor defines the node to which the fixed joint is created
        // check if there is an anchor
        std::string anchorname = "";
        if (cfg.hasKey("anchor") && (std::string)cfg["anchor"] != "none") { // backwards compatibility
          anchorname << cfg["anchor"];
        }
        if(!anchorname.empty()) {
          unsigned long anchorid = 0;
          if (anchorname == "parent") {
            anchorid = parentid;
          } else if (anchorname != "world") {// if entity gets attached to other entity
            std::string entityname, linkname;
            unsigned int splitsignpos = anchorname.find("::");
            if (splitsignpos >= 0) {
              entityname = anchorname.substr(0, splitsignpos);
              linkname = anchorname.substr(splitsignpos+2);
              SimEntity* anchorentity = control->entities->getEntity(entityname);
              if (anchorentity != 0) {
                unsigned long linkid = anchorentity->getNode(linkname);
                if (linkid > 0) {
                  anchorid = linkid;
                } else {
                  fprintf(stderr, "No valid link id for '%s' in anchor entity '%s' .\n", linkname.c_str(), entityname.c_str());
                }
              } else {
                fprintf(stderr, "No valid anchor entity '%s' in scene.\n", entityname.c_str());
              }
            } else {
              fprintf(stderr, "No valid anchor specified by parent: '%s'\n", anchorname.c_str());
            }
          }
          JointData anchorjoint;
          //fprintf(stderr, "Creating anchor joint between nodes %lu and %lu\n", id, anchorid);
          anchorjoint.nodeIndex1 = id;
          anchorjoint.nodeIndex2 = anchorid;
          anchorjoint.anchorPos = ANCHOR_NODE1;
          anchorjoint.type = JOINT_TYPE_FIXED;
          anchorjoint.name = "anchor_"+name;
          anchorJointId = control->joints->addJoint(&anchorjoint, true);
          addJoint(anchorJointId, anchorjoint.name);
        } else {
          //if there was a joint before, remove it
          if (hasAnchorJoint()) {
            fprintf(stderr, "Removing anchor joint\n");
            control->joints->removeJoint(anchorJointId);
            jointIds.erase(anchorJointId);
            anchorJointId = 0;
          }
        }
        // set Joints
        configmaps::ConfigVector::iterator it;
        configmaps::ConfigMap::iterator joint_it;
        for (it = config["poses"].begin(); it!= config["poses"].end(); ++it) {
          if ((std::string)(*it)["name"] == (std::string)config["pose"]) {
            for (joint_it = (*it)["joints"].beginMap();
                 joint_it!= (*it)["joints"].endMap(); ++joint_it) {
              //fprintf(stderr, "setMotorValue: joint: %s, id: %lu, value: %f\n", ((std::string)joint_it->first).c_str(), motorIDMap[joint_it->first], (double)joint_it->second);
              control->motors->setMotorValue(getMotor(joint_it->first),
                                             joint_it->second);
            }
          }
        }
      }
    }

    sReal SimEntity::getEntityMass() {
      sReal entity_mass=0.0;
      //sReal inertia=0.0;//TODO calculate Entity inertia with steiner for each node, needs current position and rotation of each node
      for (std::map<unsigned long, std::string>::const_iterator iter = nodeIds.begin();
          iter != nodeIds.end(); ++iter) {
        sReal mass = 0.0;
        control->nodes->getNodeMass(iter->first, &mass, nullptr);
        entity_mass += mass;
      }
      return entity_mass;
    }

    utils::Vector SimEntity::getEntityCOM() {
      std::vector<NodeId> node_ids;
      for (std::map<unsigned long, std::string>::const_iterator iter = nodeIds.begin();
          iter != nodeIds.end(); ++iter) {
        node_ids.push_back(iter->first);
      }
      return control->nodes->getCenterOfMass(node_ids);
    }

  } // end of namespace sim
} // end of namespace mars
