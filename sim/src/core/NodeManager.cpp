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

/**
 * \file NodeManager.cpp
 * \author Malte Roemmermann
 * \brief "NodeManager" is the class that manage all nodes and their
 * operations and communication between the different modules of the simulation.
 */

#include "SimNode.h"
#include "SimJoint.h"
#include "NodeManager.h"
#include "JointManager.h"
#include "PhysicsMapper.h"

#include <mars/interfaces/sim/LoadCenter.h>
#include <mars/interfaces/sim/SimulatorInterface.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>
#include <mars/interfaces/terrainStruct.h>
#include <mars/interfaces/Logging.hpp>

#include <lib_manager/LibManager.hpp>

#include <mars/interfaces/utils.h>
#include <mars/utils/mathUtils.h>
#include <mars/utils/misc.h>

#include <stdexcept>

#include <mars/utils/MutexLocker.h>

namespace mars {
  namespace sim {

    using namespace std;
    using namespace utils;
    using namespace interfaces;

    /**
     *\brief Initialization of a new NodeManager
     *
     * pre:
     *     - a pointer to a ControlCenter is needed
     * post:
     *     - next_node_id should be initialized to one
     */
    NodeManager::NodeManager(ControlCenter *c,
                             lib_manager::LibManager *theManager) :
                                                 next_node_id(1),
                                                 update_all_nodes(false),
                                                 visual_rep(1),
                                                 maxGroupID(0),
                                                 control(c),
                                                 libManager(theManager)
    {
      if(control->graphics) {
        GraphicsUpdateInterface *gui = static_cast<GraphicsUpdateInterface*>(this);
        control->graphics->addGraphicsUpdateInterface(gui);
      }
    }


    NodeId NodeManager::createPrimitiveNode(const std::string &name,
                                            NodeType type,
                                            bool moveable,
                                            const Vector &pos,
                                            const Vector &extension,
                                            double mass,
                                            const Quaternion &orientation,
                                            bool disablePhysics) {
      NodeData s;
      s.initPrimitive(type, extension, mass);
      s.name = name;
      s.pos = pos;
      s.rot = orientation;
      s.movable = moveable;
      s.noPhysical = disablePhysics;
      return addNode(&s);
    }

    /**
     *\brief Add a node to the node pool of the simulation
     *
     * It is very important to assure the serialization between the threads to
     * have the desired results.
     *
     * pre:
     *     - nodeS->groupID >= 0
     */
    NodeId NodeManager::addNode(NodeData *nodeS, bool reload,
                                bool loadGraphics) {
      iMutex.lock();
      nodeS->index = next_node_id;
      next_node_id++;
      iMutex.unlock();

      if (!reload) {
        iMutex.lock();
        NodeData reloadNode = *nodeS;
        if((nodeS->physicMode == NODE_TYPE_TERRAIN) && nodeS->terrain ) {
          if(!control->loadCenter || !control->loadCenter->loadHeightmap) {
            LOG_ERROR("NodeManager:: loadCenter is missing, can not create Node");
            iMutex.unlock();
            return INVALID_ID;
          }
          reloadNode.terrain = new(terrainStruct);
          *(reloadNode.terrain) = *(nodeS->terrain);
          control->loadCenter->loadHeightmap->readPixelData(reloadNode.terrain);
          if(!reloadNode.terrain->pixelData) {
            LOG_ERROR("NodeManager::addNode: could not load image for terrain");
            iMutex.unlock();
            return INVALID_ID;
          }
        }
        simNodesReload.push_back(reloadNode);

        if (nodeS->c_params.friction_direction1) {
          Vector *tmp = new Vector();
          *tmp = *(nodeS->c_params.friction_direction1);
          if(simNodesReload.back().index == nodeS->index) {
            simNodesReload.back().c_params.friction_direction1 = tmp;
          }
        }
        iMutex.unlock();
      }

      // to check some preconditions
      if (nodeS->groupID < 0) {
        nodeS->groupID = 0;
      }
      else if (nodeS->groupID > maxGroupID) {
        maxGroupID = nodeS->groupID;
      }

      // convert obj to ode mesh
      if((nodeS->physicMode == NODE_TYPE_MESH) && (nodeS->terrain == 0) ) {
        if(!control->loadCenter) {
          LOG_ERROR("NodeManager:: loadCenter is missing, can not create Node");
          return INVALID_ID;
        }
        if(!control->loadCenter->loadMesh) {
          GraphicsManagerInterface *g = libManager->getLibraryAs<GraphicsManagerInterface>("mars_graphics");
          if(!g) {
            libManager->loadLibrary("mars_graphics", NULL, false, true);
            g = libManager->getLibraryAs<GraphicsManagerInterface>("mars_graphics");
          }
          if(g) {
            control->loadCenter->loadMesh = g->getLoadMeshInterface();
          }
          else {
            LOG_ERROR("NodeManager:: loadMesh is missing, can not create Node");
          }
        }
        control->loadCenter->loadMesh->getPhysicsFromMesh(nodeS);
      }
      if((nodeS->physicMode == NODE_TYPE_TERRAIN) && nodeS->terrain ) {
        if(!nodeS->terrain->pixelData) {
          if(!control->loadCenter) {
            LOG_ERROR("NodeManager:: loadCenter is missing, can not create Node");
            return INVALID_ID;
          }
          if(!control->loadCenter->loadHeightmap) {
            GraphicsManagerInterface *g = libManager->getLibraryAs<GraphicsManagerInterface>("mars_graphics");
            if(!g) {
              libManager->loadLibrary("mars_graphics", NULL, false, true);
              g = libManager->getLibraryAs<GraphicsManagerInterface>("mars_graphics");
            }
            if(g) {
              control->loadCenter->loadHeightmap = g->getLoadHeightmapInterface();
            }
            else {
              LOG_ERROR("NodeManager:: loadHeightmap is missing, can not create Node");
              return INVALID_ID;
            }
          }
          control->loadCenter->loadHeightmap->readPixelData(nodeS->terrain);
          if(!nodeS->terrain->pixelData) {
            LOG_ERROR("NodeManager::addNode: could not load image for terrain");
            return INVALID_ID;
          }
        }
      }

      // this should be done somewhere else
      // if we have a relative position, we have to calculate the absolute
      // position here

      if(nodeS->relative_id != 0) {
        setNodeStructPositionFromRelative(nodeS);
        //nodeS->relative_id = 0;
      }

      // create a node object
      SimNode *newNode = new SimNode(control, *nodeS);

      // create the physical node data
      if(! (nodeS->noPhysical)){
        // create an interface object to the physics
        NodeInterface *newNodeInterface = PhysicsMapper::newNodePhysics(control->sim->getPhysics());
        if (!newNodeInterface->createNode(nodeS)) {
          // if no node was created in physics
          // delete the objects
          delete newNode;
          delete newNodeInterface;
          // and return false
          LOG_ERROR("NodeManager::addNode: No node was created in physics.");
          return INVALID_ID;
        }
        // put all data to the correct place
        //      newNode->setSNode(*nodeS);
        newNode->setInterface(newNodeInterface);
        iMutex.lock();
        simNodes[nodeS->index] = newNode;
        if (nodeS->movable)
          simNodesDyn[nodeS->index] = newNode;
        iMutex.unlock();
        control->sim->sceneHasChanged(false);
        NodeId id;
        if(control->graphics) {
          if(loadGraphics) {
            id = control->graphics->addDrawObject(*nodeS, visual_rep & 1);
            if(id) {
              newNode->setGraphicsID(id);
              if(!reload) {
                simNodesReload.back().graphicsID1 = id;
              }
            }
          }
          else {
            newNode->setGraphicsID(nodeS->graphicsID1);
          }
          //        NEW_NODE_STRUCT(physicalRep);
          NodeData physicalRep;
          physicalRep = *nodeS;
          physicalRep.material = nodeS->material;
          physicalRep.material.exists = 1;
          physicalRep.material.transparency = 0.3;
          physicalRep.material.name += "_trans";
          physicalRep.visual_offset_pos = Vector(0.0, 0.0, 0.0);
          physicalRep.visual_offset_rot = Quaternion::Identity();
          physicalRep.visual_size = Vector(0.0, 0.0, 0.0);
          physicalRep.map["sharedDrawID"] = 0lu;
          physicalRep.map["visualType"] = NodeData::toString(nodeS->physicMode);
          if(nodeS->physicMode != NODE_TYPE_TERRAIN) {
            if(nodeS->physicMode != NODE_TYPE_MESH) {
              physicalRep.filename = "PRIMITIVE";
              //physicalRep.filename = nodeS->filename;
              if(nodeS->physicMode > 0 && nodeS->physicMode < NUMBER_OF_NODE_TYPES){
                physicalRep.origName = NodeData::toString(nodeS->physicMode);
              }
            }
            if(loadGraphics) {
              id = control->graphics->addDrawObject(physicalRep,
                                                    visual_rep & 2);
              if(id) {
                newNode->setGraphicsID2(id);
                if(!reload) {
                  simNodesReload.back().graphicsID2 = id;
                }
              }
            }
            else {
              newNode->setGraphicsID2(nodeS->graphicsID2);
            }
          }
          newNode->setVisualRep(visual_rep);
        }
      } else {  //if nonPhysical
        iMutex.lock();
        simNodes[nodeS->index] = newNode;
        if (nodeS->movable)
          simNodesDyn[nodeS->index] = newNode;
        iMutex.unlock();
        control->sim->sceneHasChanged(false);
        if(control->graphics) {
          if(loadGraphics) {
            NodeId id = control->graphics->addDrawObject(*nodeS);
            if(id) {
              newNode->setGraphicsID(id);
              if(!reload) {
                simNodesReload.back().graphicsID1 = id;
              }
            }
          }
          else {
            newNode->setGraphicsID(nodeS->graphicsID1);
          }
        }
      }
      return nodeS->index;
    }

    /**
     *\brief This function maps a terrainStruct to a node struct and adds
     * that node to the simulation.
     *
     */
    NodeId NodeManager::addTerrain(terrainStruct* terrain) {
      NodeData newNode;
      terrainStruct *newTerrain = new terrainStruct(*terrain);
      sRotation trot = {0, 0, 0};

      newNode.name = terrain->name;
      newNode.filename = terrain->srcname;
      newNode.terrain = newTerrain;
      newNode.movable = false;
      newNode.groupID = 0;
      newNode.relative_id = 0;
      newNode.physicMode = NODE_TYPE_TERRAIN;
      newNode.pos = Vector(0.0, 0.0, 0.0);
      newNode.rot = eulerToQuaternion(trot);
      newNode.density = 1;
      newNode.mass = 0;
      newNode.ext = Vector(terrain->targetWidth, terrain->targetHeight,
                           terrain->scale);
      newNode.material = terrain->material;
      control->sim->sceneHasChanged(false);
      return addNode(&newNode, true);
    }

    /**
     *\brief This function adds an vector of nodes to the factory.
     * The functionality is implemented in the GUI, but should
     * move to the node factory soon.
     *
     */
    vector<NodeId> NodeManager::addNode(vector<NodeData> v_NodeData) {
      vector<NodeId> tmp;
      vector<NodeData>::iterator iter;

      control->sim->sceneHasChanged(false);
      for(iter=v_NodeData.begin(); iter!=v_NodeData.end(); iter++)
        tmp.push_back(addNode(&(*iter)));
      return tmp;
    }

    /**
     *\brief This function adds an primitive to the simulation.
     * The functionality is implemented in the GUI, but should
     * move to the node factory soon.
     *
     */
    NodeId NodeManager::addPrimitive(NodeData *snode) {
      control->sim->sceneHasChanged(false);
      return addNode(snode);
    }

    /**
     *\brief returns the number of nodes added to the simulation
     *
     */
    int NodeManager::getNodeCount() const {
      MutexLocker locker(&iMutex);
      return simNodes.size();
    }

    NodeId NodeManager::getNextNodeID() const {
      return next_node_id;
    }

    /**
     * \brief Change a node. The simulation must be updated in here.
     * doc has to be written
     */
    void NodeManager::editNode(NodeData *nodeS, int changes) {
      NodeMap::iterator iter;
      std::vector<SimJoint*>::iterator jter;
      std::vector<int> gids;
      NodeMap::iterator it;
      Vector offset;
      Vector rotation_point;

      //cout << "NodeManager::editNode !!!" << endl;
      // first lock all core functions
      iMutex.lock();

      iter = simNodes.find(nodeS->index);
      if(iter == simNodes.end()) {
        iMutex.unlock();
        LOG_ERROR("NodeManager::editNode: node id not found!");
        return;
      }

      SimNode *editedNode = iter->second;
      NodeData sNode = editedNode->getSNode();
      if(changes & EDIT_NODE_POS) {
        if(changes & EDIT_NODE_MOVE_ALL) {
          // first move the node an all nodes of the group
          offset = editedNode->setPosition(nodeS->pos, true);
          // then move recursive all nodes that are connected through
          // joints to the node
          std::vector<SimJoint*> joints = control->joints->getSimJoints();
          if(editedNode->getGroupID())
            gids.push_back(editedNode->getGroupID());
          NodeMap nodes = simNodes;
          std::vector<SimJoint*> jointsj = joints;
          nodes.erase(nodes.find(editedNode->getID()));
          moveNodeRecursive(nodeS->index, offset, &joints, &gids, &nodes);
        } else {
          if(nodeS->relative_id) {
            iMutex.unlock();
            setNodeStructPositionFromRelative(nodeS);
            iMutex.lock();
          }
          Vector diff = nodeS->pos - editedNode->getPosition();
          editedNode->setPosition(nodeS->pos, false);

          // new implementation in jointManager?
          NodeMap nodes = simNodes;
          NodeMap nodesj = simNodes;
          std::vector<SimJoint*> jointsj = control->joints->getSimJoints();
          nodes.erase(nodes.find(editedNode->getID()));
          moveRelativeNodes(*editedNode, &nodes, diff);

          if(sNode.groupID != 0) {
            for(it=simNodes.begin(); it!=simNodes.end(); ++it) {
              if(it->second->getGroupID() == sNode.groupID) {
                control->joints->reattacheJoints(it->second->getID());
              }
            }
          }
          else {
            control->joints->reattacheJoints(nodeS->index);
          }
          iMutex.unlock();
          resetRelativeJoints(*editedNode, &nodesj, &jointsj);
          iMutex.lock();
        }
        update_all_nodes = true;
      }
      if(changes & EDIT_NODE_ROT) {
        Quaternion q(Quaternion::Identity());
        if(changes & EDIT_NODE_MOVE_ALL) {
          // first move the node an all nodes of the group
          rotation_point = editedNode->getPosition();
          // the first node have to be rotated normal, not at a point
          // and should return the relative rotation it executes
          q = editedNode->setRotation(nodeS->rot, true);
          // then rotate recursive all nodes that are connected through
          // joints to the node
          std::vector<SimJoint*> joints = control->joints->getSimJoints();
          if(editedNode->getGroupID())
            gids.push_back(editedNode->getGroupID());
          NodeMap nodes = simNodes;
          std::vector<SimJoint*> jointsj = control->joints->getSimJoints();
          nodes.erase(nodes.find(editedNode->getID()));
          rotateNodeRecursive(nodeS->index, rotation_point, q, &joints,
                              &gids, &nodes);
        } else {
          if(nodeS->relative_id) {
            iMutex.unlock();
            setNodeStructPositionFromRelative(nodeS);
            iMutex.lock();
            NodeData da = editedNode->getSNode();
            da.rot = nodeS->rot;
            editedNode->setRelativePosition(da);
          }
          rotation_point = editedNode->getPosition();
          //if(nodeS->relative_id && !load_actual)
          //setNodeStructPositionFromRelative(nodeS);
          q = editedNode->setRotation(nodeS->rot, 0);

          //(*iter)->rotateAtPoint(&rotation_point, &nodeS->rot, false);

          NodeMap nodes = simNodes;
          NodeMap nodesj = simNodes;
          std::vector<SimJoint*> jointsj = control->joints->getSimJoints();
          nodes.erase(nodes.find(editedNode->getID()));
          rotateRelativeNodes(*editedNode, &nodes, rotation_point, q);

          if(sNode.groupID != 0) {
            for(it=simNodes.begin(); it!=simNodes.end(); ++it) {
              if(it->second->getGroupID() == sNode.groupID) {
                control->joints->reattacheJoints(it->second->getID());
              }
            }
          }
          else {
            control->joints->reattacheJoints(nodeS->index);
          }

          iMutex.unlock(); // is this desired???
          resetRelativeJoints(*editedNode, &nodesj, &jointsj);
          iMutex.lock();
        }
        update_all_nodes = true;
      }
      if ((changes & EDIT_NODE_SIZE) || (changes & EDIT_NODE_TYPE) || (changes & EDIT_NODE_CONTACT) ||
          (changes & EDIT_NODE_MASS) || (changes & EDIT_NODE_NAME) ||
          (changes & EDIT_NODE_GROUP) || (changes & EDIT_NODE_PHYSICS)) {
        //cout << "EDIT_NODE_SIZE !!!" << endl;
        changeNode(editedNode, nodeS);
        /*
          if (changes & EDIT_NODE_SIZE) {
          NodeMap nodes = simNodes;
          NodeMap nodesj = simNodes;
          std::vector<SimJoint*> jointsj = control->joints->getSimJoints();
          nodes.erase(nodes.find(editedNode->getID()));
          resetRelativeNodes(*editedNode, &nodes);
          iMutex.unlock(); // is this desired???
          resetRelativeJoints(*editedNode, &nodesj, &jointsj);
          iMutex.lock();
          }
        */
      }
      if ((changes & EDIT_NODE_MATERIAL)) {
        editedNode->setMaterial(nodeS->material);
        if(control->graphics)
          control->graphics->setDrawObjectMaterial(editedNode->getGraphicsID(),
                                                   nodeS->material);
      }

      // vs: updateNodesFromPhysics();
      iMutex.unlock();
      updateDynamicNodes(0, false);
    }

    void NodeManager::changeGroup(NodeId id, int group) {
      CPP_UNUSED(id);
      CPP_UNUSED(group);
    }

    /**
     * \brief Fills a list of core_object_exchange objects with node
     * iformations.
     */
    void NodeManager::getListNodes(vector<core_objects_exchange>* nodeList) const {
      core_objects_exchange obj;
      NodeMap::const_iterator iter;
      MutexLocker locker(&iMutex);
      nodeList->clear();
      for (iter = simNodes.begin(); iter != simNodes.end(); iter++) {
        iter->second->getCoreExchange(&obj);
        nodeList->push_back(obj);
      }
    }

    /** \brief
     * Fills one core_object_exchange object with node information
     * of the node with the given id.
     */
    void NodeManager::getNodeExchange(NodeId id, core_objects_exchange* obj) const {
      MutexLocker locker(&iMutex);
      NodeMap::const_iterator iter = simNodes.find(id);
      if (iter != simNodes.end())
        iter->second->getCoreExchange(obj);
      else
        obj = NULL;
    }

    /**
     * \brief get the full struct of a Node for editing purposes
     * \throw std::runtime_error if the node cannot be found
     */
    const NodeData NodeManager::getFullNode(NodeId id) const {
      MutexLocker locker(&iMutex);
      NodeMap::const_iterator iter = simNodes.find(id);
      if (iter != simNodes.end())
        return iter->second->getSNode();
      else {
        char msg[128];
        sprintf(msg, "could not find node with id: %lu", id);
        throw std::runtime_error(msg);
      }
    }


    /**
     * \brief removes the node with the corresponding id.
     *
     * Ok, first we have to check if the node is an element of a composite object.
     * If so, we have to cases:
     *
     * first case:
     *       - other nodes exist, which are element of the composite object.
     *       -> we can delete the node and keep the group
     *
     * second case:
     *       - this node is the only (last) one of the composite object
     *       -> we have to delete both, the node and the group
     *
     * At this moment we only the physical implementation handle the groups,
     * so the tow cases can be handled here in the same way:
     * -> tell the physics to destroy the object and remove the node from
     *    the core.
     *
     * What about joints?
     */
    void NodeManager::removeNode(NodeId id, bool clearGraphics) {
      removeNode(id, true, clearGraphics);
    }

    void NodeManager::removeNode(NodeId id, bool lock, bool clearGraphics) {
      NodeMap::iterator iter; //NodeMap is a map containing an id and a SimNode
      SimNode *tmpNode = 0;

      if(lock) iMutex.lock();

      iter = simNodes.find(id);
      if (iter != simNodes.end()) {
        tmpNode = iter->second; //iter->second is a pointer to the SimNode associated with the map
        simNodes.erase(iter);
      }

      iter = nodesToUpdate.find(id);
      if (iter != nodesToUpdate.end()) {
        nodesToUpdate.erase(iter);
      }

      if (tmpNode && tmpNode->isMovable()) {
        iter = simNodesDyn.find(id);
        if (iter != simNodesDyn.end()) {
          simNodesDyn.erase(iter);
        }
      }

      iMutex.unlock();
      if(!lock) iMutex.lock();
      if (tmpNode) {
        clearRelativePosition(id, lock);
        if(control->graphics && clearGraphics) {
          control->graphics->removeDrawObject(tmpNode->getGraphicsID());
          control->graphics->removeDrawObject(tmpNode->getGraphicsID2());
        }
        delete tmpNode;
      }
      control->sim->sceneHasChanged(false);
    }

    /**
     *\brief Set physical dynamic values for the node with the given id.
     */
    void NodeManager::setNodeState(NodeId id, const nodeState &state) {
      MutexLocker locker(&iMutex);
      NodeMap::iterator iter = simNodes.find(id);
      if (iter != simNodes.end())
        iter->second->setPhysicalState(state);
    }

    /**
     *\brief Get physical dynamic values for the node with the given id.
     */
    void NodeManager::getNodeState(NodeId id, nodeState *state) const {
      MutexLocker locker(&iMutex);
      NodeMap::const_iterator iter = simNodes.find(id);
      if (iter != simNodes.end())
        iter->second->getPhysicalState(state);
    }

    /**
     *\brief Return the center of mass for the nodes corresponding to
     * the id's from the given vector.
     */
    const Vector NodeManager::getCenterOfMass(const std::vector<NodeId> &ids) const {
      std::vector<NodeId>::const_iterator iter;
      NodeMap::const_iterator nter;
      std::vector<NodeInterface*> pNodes;

      MutexLocker locker(&iMutex);

      for (iter = ids.begin(); iter != ids.end(); iter++) {
        nter = simNodes.find(*iter);
        if (nter != simNodes.end())
          pNodes.push_back(nter->second->getInterface());
      }

      return control->sim->getPhysics()->getCenterOfMass(pNodes);
    }

    /**
     *\brief Sets the position of the node with the given id.
     */
    void NodeManager::setPosition(NodeId id, const Vector &pos) {
      MutexLocker locker(&iMutex);
      NodeMap::iterator iter = simNodes.find(id);
      if (iter != simNodes.end()) {
        iter->second->setPosition(pos, 1);
        nodesToUpdate[id] = iter->second;
      }
    }


    const Vector NodeManager::getPosition(NodeId id) const {
      Vector pos(0.0,0.0,0.0);
      MutexLocker locker(&iMutex);
      NodeMap::const_iterator iter = simNodes.find(id);
      if (iter != simNodes.end())
        pos = iter->second->getPosition();
      return pos;
    }


    const Quaternion NodeManager::getRotation(NodeId id) const {
      Quaternion q(Quaternion::Identity());
      MutexLocker locker(&iMutex);
      NodeMap::const_iterator iter = simNodes.find(id);
      if (iter != simNodes.end())
        q = iter->second->getRotation();
      return q;
    }


    const Vector NodeManager::getLinearVelocity(NodeId id) const {
      Vector vel(0.0,0.0,0.0);
      MutexLocker locker(&iMutex);
      NodeMap::const_iterator iter = simNodes.find(id);
      if (iter != simNodes.end())
        vel = iter->second->getLinearVelocity();
      return vel;
    }

    const Vector NodeManager::getAngularVelocity(NodeId id) const {
      Vector avel(0.0,0.0,0.0);
      MutexLocker locker(&iMutex);
      NodeMap::const_iterator iter = simNodes.find(id);
      if (iter != simNodes.end())
        avel = iter->second->getAngularVelocity();
      return avel;
    }


    const Vector NodeManager::getLinearAcceleration(NodeId id) const {
      Vector acc(0.0,0.0,0.0);
      MutexLocker locker(&iMutex);
      NodeMap::const_iterator iter = simNodes.find(id);
      if (iter != simNodes.end())
        acc = iter->second->getLinearAcceleration();
      return acc;
    }

    const Vector NodeManager::getAngularAcceleration(NodeId id) const {
      Vector aacc(0.0,0.0,0.0);
      MutexLocker locker(&iMutex);
      NodeMap::const_iterator iter = simNodes.find(id);
      if (iter != simNodes.end())
        aacc = iter->second->getAngularAcceleration();
      return aacc;
    }




    /**
     *\brief Sets the rotation of the node with the given id.
     */
    void NodeManager::setRotation(NodeId id, const Quaternion &rot) {
      MutexLocker locker(&iMutex);
      NodeMap::iterator iter = simNodes.find(id);
      if (iter != simNodes.end())
        iter->second->setRotation(rot, 1);
    }

    /**
     *\brief Adds a off-center Force to the node with the given id.
     */
    void NodeManager::applyForce(NodeId id, const Vector &force, const Vector &pos) {
      MutexLocker locker(&iMutex);
      NodeMap::iterator iter = simNodes.find(id);
      if (iter != simNodes.end())
        iter->second->applyForce(force, pos);
    }
    /**
     *\brief Adds a Force to the node with the given id.
     */
    void NodeManager::applyForce(NodeId id, const Vector &force) {
      MutexLocker locker(&iMutex);
      NodeMap::iterator iter = simNodes.find(id);
      if (iter != simNodes.end())
        iter->second->applyForce(force);
    }

    /**
     *\brief Adds a Torque to the node with the given id.
     */
    void NodeManager::applyTorque(NodeId id, const Vector &torque) {
      MutexLocker locker(&iMutex);
      NodeMap::iterator iter = simNodes.find(id);
      if (iter != simNodes.end())
        iter->second->applyTorque(torque);
    }


    /**
     *\brief Sets the contact parameter motion1 for the node with the given id.
     */
    void NodeManager::setContactParamMotion1(NodeId id, sReal motion) {
      MutexLocker locker(&iMutex);
      NodeMap::iterator iter = simNodes.find(id);
      if (iter != simNodes.end())
        iter->second->setContactMotion1(motion);
    }


    /**
     *\brief Adds a physical sensor to the node with the given id.
     */
    void NodeManager::addNodeSensor(BaseNodeSensor *sensor){
      MutexLocker locker(&iMutex);
      NodeMap::iterator iter = simNodes.find(sensor->getAttachedNode());
      if (iter != simNodes.end()) {
        iter->second->addSensor(sensor);
        NodeMap::iterator kter = simNodesDyn.find(sensor->getAttachedNode());
        if (kter == simNodesDyn.end())
          simNodesDyn[iter->first] = iter->second;
      }
      else
        {
          std::cerr << "Could not find node id " << sensor->getAttachedNode() << " in simNodes and did not call addSensors on the node." << std::endl;
        }
    }

    void NodeManager::reloadNodeSensor(BaseNodeSensor* sensor) {
      MutexLocker locker(&iMutex);
      NodeMap::iterator iter = simNodes.find(sensor->getAttachedNode());
      if (iter != simNodes.end())
        iter->second->reloadSensor(sensor);
    }

    /**
     *\brief Returns a pointer to the SimNode Object.
     */
    SimNode *NodeManager::getSimNode(NodeId id) {
      return const_cast<SimNode*>(static_cast<const NodeManager*>(this)->getSimNode(id));
    }

    const SimNode* NodeManager::getSimNode(NodeId id) const {
      MutexLocker locker(&iMutex);
      NodeMap::const_iterator iter = simNodes.find(id);
      if (iter != simNodes.end())
        return iter->second;
      else
        return NULL;
    }


    void NodeManager::setNodeStructPositionFromRelative(NodeData *node) const {
      MutexLocker locker(&iMutex);
      NodeMap::const_iterator iter = simNodes.find(node->relative_id);
      if (iter != simNodes.end()) {
        NodeData tmpNode = iter->second->getSNode();
        getAbsFromRel(tmpNode, node);
      }
    }

    void NodeManager::resetRelativeNodes(const SimNode &node,
                                         NodeMap *nodes,
                                         const Quaternion *rotate) {
      NodeMap::iterator iter;
      NodeData tmpNode, tmpNode2;
      SimNode* nextNode;

      // TODO: doesn't this function need locking? no
      tmpNode = node.getSNode();
      for (iter = nodes->begin(); iter != nodes->end(); iter++) {
        if (iter->second->getSNode().relative_id == node.getID()) {
          nextNode = iter->second;
          tmpNode2 = iter->second->getSNode();
          if(rotate)
            tmpNode2.rot = *rotate * tmpNode2.rot;
          getAbsFromRel(tmpNode, &tmpNode2);
          iter->second->setPosition(tmpNode2.pos, false);
          iter->second->setRotation(tmpNode2.rot, 0);
          nodes->erase(iter);
          resetRelativeNodes(node, nodes, rotate);
          resetRelativeNodes(*nextNode, nodes, rotate);
          break;
        }
      }
    }

    void NodeManager::resetRelativeJoints(const SimNode &node,
                                          NodeMap *nodes,
                                          std::vector<SimJoint*> *joints,
                                          const Quaternion *rotate) {
      NodeMap::iterator iter;
      std::vector<SimJoint*>::iterator jter;
      std::vector<SimJoint*>::iterator jter2;
      SimNode* nextNode;

      iMutex.lock();
      for (iter = nodes->begin(); iter != nodes->end(); iter++) {
        if (iter->second->getSNode().relative_id == node.getID()) {
          nextNode = iter->second;
          for (jter = joints->begin(); jter != joints->end();) {
            if ((*jter)->getSJoint().nodeIndex1 == iter->first ||
                (*jter)->getSJoint().nodeIndex2 == iter->first) {
              if(rotate) (*jter)->rotateAxis(*rotate);
              (*jter)->reattachJoint();
              jter2 = jter;
              if(jter != joints->begin()) jter--;
              else jter = joints->begin();
              joints->erase(jter2);
            }
            else jter++;
          }

          nodes->erase(iter);
          iMutex.unlock();
          resetRelativeJoints(node, nodes, joints, rotate);
          resetRelativeJoints(*nextNode, nodes, joints, rotate);
          iMutex.lock();
          break;
        }
      }
      iMutex.unlock();
    }


    void NodeManager::recursiveHelper(NodeId id, const Params *params,
                                      std::vector<SimJoint*> *joints,
                                      std::vector<int> *gids,
                                      NodeMap *nodes,
                                      void (*applyFunc)(SimNode *, const Params *)) {

      std::vector<SimJoint*>::iterator iter;
      std::vector<int>::iterator jter;
      NodeMap::iterator nter;
      NodeId id2;
      bool found = false;

      for(jter = gids->begin(); jter != gids->end(); jter++) {
        for(nter = nodes->begin(); nter != nodes->end(); nter++) {
          if(nter->second->getGroupID() == (*jter)) {
            id2 = nter->first;
            nodes->erase(nter);
            recursiveHelper(id, params, joints, gids, nodes, applyFunc);
            recursiveHelper(id2, params, joints, gids, nodes, applyFunc);
            return;
          }
        }
      }

      for (iter = joints->begin(); iter != joints->end(); iter++) {
        if ((*iter)->getAttachedNode() &&
            (*iter)->getAttachedNode()->getID() == id) {
          for (jter = gids->begin(); jter != gids->end(); jter++) {
            if ((*iter)->getAttachedNode(2) &&
                (*jter) == (*iter)->getAttachedNode(2)->getGroupID()) {
              found = true;
              break;
            }
          }
          if ((*iter)->getAttachedNode(2) &&
              nodes->find((*iter)->getAttachedNode(2)->getID()) != nodes->end()) {
            id2 = (*iter)->getAttachedNode(2)->getID();
            if (!found) {
              if ((*iter)->getAttachedNode(2)->getGroupID())
                gids->push_back((*iter)->getAttachedNode(2)->getGroupID());
              applyFunc((*iter)->getAttachedNode(2), params);
            }
            nodes->erase(nodes->find((*iter)->getAttachedNode(2)->getID()));
            joints->erase(iter);
            recursiveHelper(id, params, joints, gids, nodes, applyFunc);
            recursiveHelper(id2, params, joints, gids, nodes, applyFunc);
            return;
          }
          else found = false;
        } else if ((*iter)->getAttachedNode(2) &&
                   (*iter)->getAttachedNode(2)->getID() == id) {
          for (jter = gids->begin(); jter != gids->end(); jter++) {
            if ((*iter)->getAttachedNode() &&
                (*jter) == (*iter)->getAttachedNode()->getGroupID()) {
              found = true;
              break;
            }
          }
          if(nodes->find((*iter)->getAttachedNode()->getID()) != nodes->end()) {
            id2 = (*iter)->getAttachedNode()->getID();
            if (!found) {
              if ((*iter)->getAttachedNode()->getGroupID())
                gids->push_back((*iter)->getAttachedNode()->getGroupID());
              applyFunc((*iter)->getAttachedNode(), params);
            }
            nodes->erase(nodes->find((*iter)->getAttachedNode()->getID()));
            joints->erase(iter);
            recursiveHelper(id, params, joints, gids, nodes, applyFunc);
            recursiveHelper(id2, params, joints, gids, nodes, applyFunc);
            return;
          }
          else found = false;
        }
      }
    }

    void NodeManager::applyMove(SimNode *node, const Params *params)
    {
      const Vector offset = dynamic_cast<const MoveParams*>(params)->offset;
      node->setPositionOffset(offset);
    }

    void NodeManager::applyRotation(SimNode *node, const Params *params)
    {
      const RotationParams *p = dynamic_cast<const RotationParams*>(params);
      node->rotateAtPoint(p->rotation_point, p->rotation, true);
    }

    void NodeManager::moveNodeRecursive(NodeId id, const Vector &offset,
                                        std::vector<SimJoint*> *joints,
                                        std::vector<int> *gids,
                                        NodeMap *nodes) {
      MoveParams params;
      params.offset = offset;
      recursiveHelper(id, &params, joints, gids, nodes, &applyMove);
    }

    void NodeManager::rotateNode(NodeId id, Vector pivot, Quaternion q,
                                 unsigned long excludeJointId, bool includeConnected) {
      std::vector<int> gids;
      NodeMap::iterator iter = simNodes.find(id);
      if(iter == simNodes.end()) {
        iMutex.unlock();
        LOG_ERROR("NodeManager::rotateNode: node id not found!");
        return;
      }

      SimNode *editedNode = iter->second;
      editedNode->rotateAtPoint(pivot, q, true);

      if (includeConnected) {
        std::vector<SimJoint*> joints = control->joints->getSimJoints();
        std::vector<SimJoint*>::iterator jter;
        for(jter=joints.begin(); jter!=joints.end(); ++jter) {
          if((*jter)->getIndex() == excludeJointId) {
            joints.erase(jter);
            break;
          }
        }

        if(editedNode->getGroupID())
          gids.push_back(editedNode->getGroupID());

        NodeMap nodes = simNodes;
        nodes.erase(nodes.find(editedNode->getID()));

        rotateNodeRecursive(id, pivot, q, &joints,
                            &gids, &nodes);
      }
      update_all_nodes = true;
      updateDynamicNodes(0, false);
    }

    void NodeManager::positionNode(NodeId id, Vector pos,
                                   unsigned long excludeJointId) {
      std::vector<int> gids;
      NodeMap::iterator iter = simNodes.find(id);
      if(iter == simNodes.end()) {
        iMutex.unlock();
        LOG_ERROR("NodeManager::rotateNode: node id not found!");
        return;
      }

      SimNode *editedNode = iter->second;
      Vector offset = pos - editedNode->getPosition();
      editedNode->setPosition(pos, true);

      std::vector<SimJoint*> joints = control->joints->getSimJoints();
      std::vector<SimJoint*>::iterator jter;
      for(jter=joints.begin(); jter!=joints.end(); ++jter) {
        if((*jter)->getIndex() == excludeJointId) {
          joints.erase(jter);
          break;
        }
      }

      if(editedNode->getGroupID())
        gids.push_back(editedNode->getGroupID());

      NodeMap nodes = simNodes;
      nodes.erase(nodes.find(editedNode->getID()));

      moveNodeRecursive(id, offset, &joints, &gids, &nodes);

      update_all_nodes = true;
      updateDynamicNodes(0, false);
    }

    void NodeManager::rotateNodeRecursive(NodeId id,
                                          const Vector &rotation_point,
                                          const Quaternion &rotation,
                                          std::vector<SimJoint*> *joints,
                                          std::vector<int> *gids,
                                          NodeMap *nodes) {
      RotationParams params;
      params.rotation_point = rotation_point;
      params.rotation = rotation;
      recursiveHelper(id, &params, joints, gids, nodes, &applyRotation);
    }

    void NodeManager::clearRelativePosition(NodeId id, bool lock) {
      NodeMap::iterator iter;
      if(lock) iMutex.lock();
      for(iter = simNodes.begin(); iter != simNodes.end(); iter++) {
        if(iter->second->getSNode().relative_id == id) {
          iter->second->clearRelativePosition();
        }
      }
      if(lock) iMutex.unlock();
    }


    /**
     *\brief Reloads all nodes in the simulation.
     */
    void NodeManager::reloadNodes(bool reloadGrahpics) {
      std::list<NodeData>::iterator iter;
      NodeData tmp;
      Vector* friction;

      iMutex.lock();
      for(iter = simNodesReload.begin(); iter != simNodesReload.end(); iter++) {
        tmp = *iter;
        if(tmp.c_params.friction_direction1) {
          friction = new Vector(0.0, 0.0, 0.0);
          *friction = *(tmp.c_params.friction_direction1);
          tmp.c_params.friction_direction1 = friction;
        }
        if(tmp.terrain) {
          tmp.terrain = new(terrainStruct);
          *(tmp.terrain) = *(iter->terrain);
          tmp.terrain->pixelData = (double*)calloc((tmp.terrain->width*
                                                     tmp.terrain->height),
                                                    sizeof(double));
          memcpy(tmp.terrain->pixelData, iter->terrain->pixelData,
                 (tmp.terrain->width*tmp.terrain->height)*sizeof(double));
        }
        iMutex.unlock();
        addNode(&tmp, true, reloadGrahpics);
        iMutex.lock();
      }
      iMutex.unlock();
      updateDynamicNodes(0);
    }

    std::list<NodeData>::iterator NodeManager::getReloadNode(NodeId id) {
      std::list<NodeData>::iterator iter = simNodesReload.begin();
      for(;iter!=simNodesReload.end(); ++iter) {
        if(iter->index == id) break;
      }
      return iter;
    }

    /**
     *\brief set the size for the node with the given id.
     */
    const Vector NodeManager::setReloadExtent(NodeId id, const Vector &ext) {
      Vector x(0.0,0.0,0.0);
      MutexLocker locker(&iMutex);
      std::list<NodeData>::iterator iter = getReloadNode(id);
      if (iter != simNodesReload.end()) {
        if(iter->filename != "PRIMITIVE") {
          x.x() = ext.x() / iter->ext.x();
          x.y() = ext.y() / iter->ext.y();
          x.z() = ext.z() / iter->ext.z();
        }
        iter->ext = ext;
      }
      return x;
    }


    void NodeManager::setReloadFriction(NodeId id, sReal friction1,
                                        sReal friction2) {
      MutexLocker locker(&iMutex);

      std::list<NodeData>::iterator iter = getReloadNode(id);
      if (iter != simNodesReload.end()) {
        iter->c_params.friction1 = friction1;
        iter->c_params.friction2 = friction2;
      }
    }


    /**
     *\brief set the position for the node with the given id.
     */
    void NodeManager::setReloadPosition(NodeId id, const Vector &pos) {
      MutexLocker locker(&iMutex);
      std::list<NodeData>::iterator iter = getReloadNode(id);
      if (iter != simNodesReload.end()) {
        iter->pos = pos;
      }
    }


    /**
     *\brief Updates the Node values of dynamical nodes from the physics.
     */
    void NodeManager::updateDynamicNodes(sReal calc_ms, bool physics_thread) {
      MutexLocker locker(&iMutex);
      NodeMap::iterator iter;
      for(iter = simNodesDyn.begin(); iter != simNodesDyn.end(); iter++) {
        iter->second->update(calc_ms, physics_thread);
      }
    }

    void NodeManager::preGraphicsUpdate() {
      NodeMap::iterator iter;
      if(!control->graphics)
        return;

      iMutex.lock();
      if(update_all_nodes) {
        update_all_nodes = false;
        for(iter = simNodes.begin(); iter != simNodes.end(); iter++) {
          control->graphics->setDrawObjectPos(iter->second->getGraphicsID(),
                                              iter->second->getVisualPosition());
          control->graphics->setDrawObjectRot(iter->second->getGraphicsID(),
                                              iter->second->getVisualRotation());
          control->graphics->setDrawObjectPos(iter->second->getGraphicsID2(),
                                              iter->second->getPosition());
          control->graphics->setDrawObjectRot(iter->second->getGraphicsID2(),
                                              iter->second->getRotation());
        }
      }
      else {
        for(iter = simNodesDyn.begin(); iter != simNodesDyn.end(); iter++) {
          control->graphics->setDrawObjectPos(iter->second->getGraphicsID(),
                                              iter->second->getVisualPosition());
          control->graphics->setDrawObjectRot(iter->second->getGraphicsID(),
                                              iter->second->getVisualRotation());
          control->graphics->setDrawObjectPos(iter->second->getGraphicsID2(),
                                              iter->second->getPosition());
          control->graphics->setDrawObjectRot(iter->second->getGraphicsID2(),
                                              iter->second->getRotation());
        }
        for(iter = nodesToUpdate.begin(); iter != nodesToUpdate.end(); iter++) {
          control->graphics->setDrawObjectPos(iter->second->getGraphicsID(),
                                              iter->second->getVisualPosition());
          control->graphics->setDrawObjectRot(iter->second->getGraphicsID(),
                                              iter->second->getVisualRotation());
          control->graphics->setDrawObjectPos(iter->second->getGraphicsID2(),
                                              iter->second->getPosition());
          control->graphics->setDrawObjectRot(iter->second->getGraphicsID2(),
                                              iter->second->getRotation());
        }
        nodesToUpdate.clear();
      }
      iMutex.unlock();
    }

    /**
     *\brief Removes all nodes from the simulation to clear the world.
     */
    void NodeManager::clearAllNodes(bool clear_all, bool clearGraphics) {
      MutexLocker locker(&iMutex);
      NodeMap::iterator iter;
      while (!simNodes.empty())
        removeNode(simNodes.begin()->first, false, clearGraphics);
      simNodes.clear();
      simNodesDyn.clear();
      if(clear_all) simNodesReload.clear();
      next_node_id = 1;
      iMutex.unlock();
    }

    /**
     *\brief Set the reload orientation of a node.
     */
    void NodeManager::setReloadAngle(NodeId id, const sRotation &angle) {
      setReloadQuaternion(id, eulerToQuaternion(angle));
    }


    /**
     *\brief Set the reload orientation of a node by using a quaternion.
     */
    void NodeManager::setReloadQuaternion(NodeId id, const Quaternion &q) {
      MutexLocker locker(&iMutex);
      std::list<NodeData>::iterator iter = getReloadNode(id);
      if (iter != simNodesReload.end()) {
        iter->rot = q;
      }
    }

    /**
     *\brief Set the contact parameter of a node.
     */
    void NodeManager::setContactParams(NodeId id, const contact_params &cp) {
      MutexLocker locker(&iMutex);
      NodeMap::iterator iter = simNodes.find(id);
      if (iter != simNodes.end())
        iter->second->setContactParams(cp);
    }


    void NodeManager::setVelocity(NodeId id, const Vector& vel) {
      MutexLocker locker(&iMutex);
      NodeMap::iterator iter = simNodesDyn.find(id);
      if (iter != simNodesDyn.end())
        iter->second->setLinearVelocity(vel);
    }


    void NodeManager::setAngularVelocity(NodeId id, const Vector& vel) {
      MutexLocker locker(&iMutex);
      NodeMap::iterator iter = simNodesDyn.find(id);
      if (iter != simNodesDyn.end())
        iter->second->setAngularVelocity(vel);
    }


    /**
     *\brief Scales the nodes to reload.
     */
    void NodeManager::scaleReloadNodes(sReal factor_x, sReal factor_y,
                                       sReal factor_z) {
      std::list<NodeData>::iterator iter;

      iMutex.lock();
      for(iter = simNodesReload.begin(); iter != simNodesReload.end(); iter++) {
        iter->pos.x() *= factor_x;
        iter->pos.y() *= factor_y;
        iter->pos.z() *= factor_z;
        iter->ext.x() *= factor_x;
        iter->ext.y() *= factor_y;
        iter->ext.z() *= factor_z;
        iter->visual_offset_pos.x() *= factor_x;
        iter->visual_offset_pos.y() *= factor_y;
        iter->visual_offset_pos.z() *= factor_z;
        iter->visual_size.x() *= factor_x;
        iter->visual_size.y() *= factor_y;
        iter->visual_size.z() *= factor_z;

      }
      iMutex.unlock();
    }

    void NodeManager::getNodeMass(NodeId id, sReal *mass,
                                  sReal* inertia) const {
      MutexLocker locker(&iMutex);
      NodeMap::const_iterator iter = simNodes.find(id);
      if (iter != simNodes.end())
        iter->second->getMass(mass, inertia);
    }


    void NodeManager::setAngularDamping(NodeId id, sReal damping) {
      MutexLocker locker(&iMutex);
      NodeMap::iterator iter = simNodes.find(id);
      if (iter != simNodes.end())
        iter->second->setAngularDamping(damping);
    }


    void NodeManager::addRotation(NodeId id, const Quaternion &q) {
      MutexLocker locker(&iMutex);
      NodeMap::iterator iter = simNodes.find(id);
      if (iter != simNodes.end())
        iter->second->addRotation(q);
    }


    const contact_params NodeManager::getContactParams(NodeId id) const {
      MutexLocker locker(&iMutex);
      NodeMap::const_iterator iter = simNodes.find(id);
      if (iter != simNodes.end())
        return iter->second->getContactParams();
      contact_params a;
      return a;
    }




    void NodeManager::exportGraphicNodesByID(const std::string &folder) const {
      if(control->graphics) {
        char text[255];
        std::string filename;

        NodeMap::const_iterator iter;
        iMutex.lock();
        for(iter=simNodes.begin(); iter!=simNodes.end(); ++iter) {
          sprintf(text, "/%lu.stl", iter->first);
          filename = folder+std::string(text);
          control->graphics->exportDrawObject(iter->second->getGraphicsID(), filename);
          sprintf(text, "/%lu.obj", iter->first);
          filename = folder+std::string(text);
          control->graphics->exportDrawObject(iter->second->getGraphicsID(), filename);
        }
        iMutex.unlock();
      }
    }

    void NodeManager::getContactPoints(std::vector<NodeId> *ids,
                                       std::vector<Vector> *contact_points) const {
      NodeMap::const_iterator iter;
      std::vector<Vector>::const_iterator lter;
      std::vector<Vector> points;

      iMutex.lock();
      for(iter=simNodes.begin(); iter!=simNodes.end(); ++iter) {
        iter->second->getContactPoints(&points);
        for(lter=points.begin(); lter!=points.end(); ++lter) {
          ids->push_back(iter->first);
          contact_points->push_back((*lter));
        }
      }
      iMutex.unlock();
    }

    void NodeManager::getContactIDs(const interfaces::NodeId &id,
                                    std::list<interfaces::NodeId> *ids) const {
      MutexLocker locker(&iMutex);
      NodeMap::const_iterator iter = simNodes.find(id);
      if (iter != simNodes.end()) {
        iter->second->getContactIDs(ids);
      }
    }

    void NodeManager::updateRay(NodeId id) {
      MutexLocker locker(&iMutex);
      NodeMap::iterator iter = simNodes.find(id);
      if (iter != simNodes.end())
        iter->second->updateRay();
    }



    NodeId NodeManager::getDrawID(NodeId id) const {
      MutexLocker locker(&iMutex);
      NodeMap::const_iterator iter = simNodes.find(id);
      if (iter != simNodes.end())
        return iter->second->getGraphicsID();
      else
        return INVALID_ID;
    }


    const Vector NodeManager::getContactForce(NodeId id) const {
      MutexLocker locker(&iMutex);
      NodeMap::const_iterator iter = simNodes.find(id);
      if (iter != simNodes.end())
        return iter->second->getContactForce();
      else
        return Vector(0.0, 0.0, 0.0);
    }


    double NodeManager::getCollisionDepth(NodeId id) const {
      MutexLocker locker(&iMutex);
      NodeMap::const_iterator iter = simNodes.find(id);
      if (iter != simNodes.end())
        return iter->second->getCollisionDepth();
      else
        return 0.0;
    }


    void NodeManager::setVisualRep(NodeId id, int val) {
      if(!(control->graphics))
        return;
      visual_rep = val;
      NodeMap::iterator iter;
      int current;

      iMutex.lock();
      for(iter = simNodes.begin(); iter != simNodes.end(); iter++) {
        if(id == 0 || iter->first == id) {
          current = iter->second->getVisualRep();
          if(val & 1 && !(current & 1))
            control->graphics->setDrawObjectShow(iter->second->getGraphicsID(), true);
          else if(!(val & 1) && current & 1)
            control->graphics->setDrawObjectShow(iter->second->getGraphicsID(), false);
          if(val & 2 && !(current & 2))
            control->graphics->setDrawObjectShow(iter->second->getGraphicsID2(), true);
          else if(!(val & 2) && current & 2)
            control->graphics->setDrawObjectShow(iter->second->getGraphicsID2(), false);

          iter->second->setVisualRep(val);
          if(id != 0) break;
        }
      }
      iMutex.unlock();
    }

    NodeId NodeManager::getID(const std::string& node_name) const {

      iMutex.lock();
      NodeMap::const_iterator iter;
      for(iter = simNodes.begin(); iter != simNodes.end(); iter++) {
        if (iter->second->getName() == node_name)  {
          iMutex.unlock();
          return iter->first;
        }
      }
      iMutex.unlock();
      return INVALID_ID;
    }

    void NodeManager::pushToUpdate(SimNode* node) {
      MutexLocker locker(&iMutex);
      NodeMap::iterator iter = nodesToUpdate.find(node->getID());
      if (iter == nodesToUpdate.end())
        nodesToUpdate[node->getID()] = node;
    }



    std::vector<NodeId> NodeManager::getConnectedNodes(NodeId id) {
      std::vector<NodeId> connected;
      MutexLocker locker(&iMutex);
      NodeMap::iterator iter = simNodes.find(id);
      if (iter == simNodes.end())
        return connected;

      SimNode* current = iter->second;
      std::vector<SimJoint*> simJoints = control->joints->getSimJoints();

      if (current->getGroupID() != 0)
        for (iter = simNodes.begin(); iter != simNodes.end(); iter++)
          if (iter->second->getGroupID() == current->getGroupID())
            connected.push_back(iter->first);

      for (size_t i = 0; i < simJoints.size(); i++) {
        if (simJoints[i]->getAttachedNode() &&
            simJoints[i]->getAttachedNode()->getID() == id &&
            simJoints[i]->getAttachedNode(2)) {
          connected.push_back(simJoints[i]->getAttachedNode(2)->getID());
          /*    current = simNodes.find(connected.back())->second;
                if (current->getGroupID() != 0)
                for (iter = simNodes.begin(); iter != simNodes.end(); iter++)
                if (iter->second->getGroupID() == current->getGroupID())
                connected.push_back(iter->first);*/
        }

        if (simJoints[i]->getAttachedNode(2) &&
            simJoints[i]->getAttachedNode(2)->getID() == id &&
            simJoints[i]->getAttachedNode()) {
          connected.push_back(simJoints[i]->getAttachedNode()->getID());
          /*      current = simNodes.find(connected.back())->second;
                  if (current->getGroupID() != 0)
                  for (iter = simNodes.begin(); iter != simNodes.end(); iter++)
                  if (iter->second->getGroupID() == current->getGroupID())
                  connected.push_back(iter->first);*/
        }
      }

      return connected;

    }


    bool NodeManager::getDataBrokerNames(NodeId id, std::string *groupName,
                                         std::string *dataName) const {
      NodeMap::const_iterator iter = simNodes.find(id);
      //LOG_DEBUG("We have currently %i elements\n",(int)simNodes.size());
      if (iter == simNodes.end())
        return false;
      iter->second->getDataBrokerNames(groupName, dataName);
      return true;
    }

    void NodeManager::setVisualQOffset(NodeId id, const Quaternion &q) {
      NodeMap::const_iterator iter = simNodes.find(id);
      if (iter != simNodes.end())
        iter->second->setVisQOffset(q);
    }

    void NodeManager::updatePR(NodeId id, const Vector &pos,
                               const Quaternion &rot,
                               const Vector &visOffsetPos,
                               const Quaternion &visOffsetRot,
                               bool doLock) {
      NodeMap::const_iterator iter = simNodes.find(id);

      if (iter != simNodes.end()) {
        iter->second->updatePR(pos, rot, visOffsetPos, visOffsetRot);
        if(doLock) MutexLocker locker(&iMutex);
        nodesToUpdate[id] = iter->second;
      }
    }

    bool NodeManager::getIsMovable(NodeId id) const {
      NodeMap::const_iterator iter = simNodes.find(id);
      if(iter != simNodes.end())
        return iter->second->isMovable();
      return false;
    }

    void NodeManager::setIsMovable(NodeId id, bool isMovable) {
      NodeMap::iterator iter = simNodes.find(id);
      if(iter != simNodes.end())
        iter->second->setMovable(isMovable);
    }

    void NodeManager::moveRelativeNodes(const SimNode &node, NodeMap *nodes,
                                        Vector v) {
      NodeMap::iterator iter;
      SimNode* nextNode;

      // TODO: doesn't this function need locking? no
      for (iter = nodes->begin(); iter != nodes->end(); iter++) {
        if (iter->second->getParentID() == node.getID()) {
          nextNode = iter->second;
          Vector newPos = nextNode->getPosition() + v;
          nextNode->setPosition(newPos, false);
          nodes->erase(iter);
          moveRelativeNodes(node, nodes, v);
          moveRelativeNodes(*nextNode, nodes, v);
          break;
        }
      }
    }

    void NodeManager::rotateRelativeNodes(const SimNode &node, NodeMap *nodes,
                                          Vector pivot, Quaternion rot) {
      NodeMap::iterator iter;
      SimNode* nextNode;

      // TODO: doesn't this function need locking? no
      for (iter = nodes->begin(); iter != nodes->end(); iter++) {
        if (iter->second->getParentID() == node.getID()) {
          nextNode = iter->second;
          nextNode->rotateAtPoint(pivot, rot, false);
          nodes->erase(iter);
          rotateRelativeNodes(node, nodes, pivot, rot);
          rotateRelativeNodes(*nextNode, nodes, pivot, rot);
          break;
        }
      }
    }

    void NodeManager::printNodeMasses(bool onlysum) {
      NodeMap::iterator it;
      double masssum = 0;
      for(it=simNodes.begin(); it!=simNodes.end(); ++it) {
        if (!onlysum)
          fprintf(stderr, "%s: %f\n", it->second->getName().c_str(), it->second->getMass());
        masssum+=it->second->getMass();
      }
      fprintf(stderr, "Sum of masses of imported model: %f\n", masssum);
    }

    void NodeManager::edit(NodeId id, const std::string &key,
                           const std::string &value) {
      iMutex.lock();
      NodeMap::iterator iter;
      // todo: cfdir1 is a vector
      iter = simNodes.find(id);
      if(iter == simNodes.end()) return;
      NodeData nd = iter->second->getSNode();
      //// fprintf(stderr, "change: %s %s\n", key.c_str(), value.c_str());
      if(matchPattern("*/position", key)) {
        //// fprintf(stderr, "position\n");
        double v = atof(value.c_str());
        if(key[key.size()-1] == 'x') nd.pos.x() = v;
        else if(key[key.size()-1] == 'y') nd.pos.y() = v;
        else if(key[key.size()-1] == 'z') nd.pos.z() = v;
        iMutex.unlock();
        control->nodes->editNode(&nd, (EDIT_NODE_POS | EDIT_NODE_MOVE_ALL));
      }
      else if(matchPattern("*/rotation", key)) {
        //// fprintf(stderr, "rotation\n");
        double v = atof(value.c_str());
        sRotation r = quaternionTosRotation(nd.rot);
        bool setEuler = false;
        if(key.find("alpha") != string::npos) r.alpha = v, setEuler = true;
        else if(key.find("beta") != string::npos) r.beta = v, setEuler = true;
        else if(key.find("gamma") != string::npos) r.gamma = v, setEuler = true;
        else if(key[key.size()-1] == 'x') nd.rot.x() = v;
        else if(key[key.size()-1] == 'y') nd.rot.y() = v;
        else if(key[key.size()-1] == 'z') nd.rot.z() = v;
        else if(key[key.size()-1] == 'w') nd.rot.w() = v;
        if(setEuler) nd.rot = eulerToQuaternion(r);
        iMutex.unlock();
        control->nodes->editNode(&nd, (EDIT_NODE_ROT | EDIT_NODE_MOVE_ALL));
      }
      else if(matchPattern("*/extend/*", key)) {
        //// fprintf(stderr, "extend\n");
        double v = atof(value.c_str());
        if(key[key.size()-1] == 'x') nd.ext.x() = v;
        else if(key[key.size()-1] == 'y') nd.ext.y() = v;
        else if(key[key.size()-1] == 'z') nd.ext.z() = v;
        changeNode(iter->second, &nd);
        iMutex.unlock();
      }
      else if(matchPattern("*/material", key)) {
        //// fprintf(stderr, "material\n");
        iMutex.unlock();
        if(control->graphics) {
          std::vector<interfaces::MaterialData> mList;
          std::vector<interfaces::MaterialData>::iterator it;
          mList = control->graphics->getMaterialList();
          for(it=mList.begin(); it!=mList.end(); ++it) {
            if(it->name == value) {
              unsigned long drawID = getDrawID(id);
              control->graphics->setDrawObjectMaterial(drawID, *it);
              iter->second->setMaterialName(it->name);
              break;
            }
          }
        }
      }
      else if(matchPattern("*/cullMask", key)) {
        //// fprintf(stderr, "cullMask\n");
        int v = atoi(value.c_str());
        iter->second->setCullMask(v);
        iMutex.unlock();
      }
      else if(matchPattern("*/c*", key)) {
        //// fprintf(stderr, "contact\n");
        contact_params c = iter->second->getContactParams();
        if(matchPattern("*/cmax_num_contacts", key)) {
          c.max_num_contacts = atoi(value.c_str());;
        }
        else if(matchPattern("*/cerp", key)) c.erp = atof(value.c_str());
        else if(matchPattern("*/ccfm", key)) c.cfm = atof(value.c_str());
        else if(matchPattern("*/cfriction1", key)) c.friction1 = atof(value.c_str());
        else if(matchPattern("*/cfriction2", key)) c.friction2 = atof(value.c_str());
        else if(matchPattern("*/cmotion1", key)) c.motion1 = atof(value.c_str());
        else if(matchPattern("*/cmotion2", key)) c.motion2 = atof(value.c_str());
        else if(matchPattern("*/cfds1", key)) c.fds1 = atof(value.c_str());
        else if(matchPattern("*/cfds2", key)) c.fds2 = atof(value.c_str());
        else if(matchPattern("*/cbounce", key)) c.bounce = atof(value.c_str());
        else if(matchPattern("*/cbounce_vel", key)) c.bounce_vel = atof(value.c_str());
        else if(matchPattern("*/capprox", key)) {
          if(value == "true" || value == "True") c.approx_pyramid = true;
          else c.approx_pyramid = false;
        }
        else if(matchPattern("*/coll_bitmask", key)) c.coll_bitmask = atoi(value.c_str());
        else if(matchPattern("*/cfdir1*", key)) {
          double v = atof(value.c_str());
          if(!c.friction_direction1) c.friction_direction1 = new Vector(0,0,0);
          if(key[key.size()-1] == 'x') c.friction_direction1->x() = v;
          else if(key[key.size()-1] == 'y') c.friction_direction1->y() = v;
          else if(key[key.size()-1] == 'z') c.friction_direction1->z() = v;
          if(c.friction_direction1->norm() < 0.00000001) {
            delete c.friction_direction1;
            c.friction_direction1 = 0;
          }
        }
        iter->second->setContactParams(c);
        iMutex.unlock();
      }
      else if(matchPattern("*/brightness", key)) {
        //// fprintf(stderr, "brightness\n");
        double v = atof(value.c_str());
        iter->second->setBrightness(v);
        iMutex.unlock();
      }
      else if(matchPattern("*/name", key)) {
        //// fprintf(stderr, "name\n");
        nd.name = value;
        changeNode(iter->second, &nd);
        iMutex.unlock();
      }
      else if(matchPattern("*/mass", key)) {
        //// fprintf(stderr, "mass\n");
        nd.mass = atof(value.c_str());
        changeNode(iter->second, &nd);
        iMutex.unlock();
      }
      else if(matchPattern("*/density", key)) {
        //// fprintf(stderr, "density\n");
        nd.density = atof(value.c_str());
        changeNode(iter->second, &nd);
        iMutex.unlock();
      }
      else if(matchPattern("*/movable", key)) {
        //// fprintf(stderr, "movable\n");
        ConfigMap b;
        b["bool"] = value;
        nd.movable = b["bool"];
        changeNode(iter->second, &nd);
        iMutex.unlock();
      }
      else if(matchPattern("*/groupid", key)) {
        //// fprintf(stderr, "groupid\n");
        nd.groupID = atoi(value.c_str());
        changeNode(iter->second, &nd);
        iMutex.unlock();
      }
      else if(matchPattern("*/relativeid", key)) {
        //// fprintf(stderr, "relativeid\n");
        nd.relative_id = atoi(value.c_str());
        iter->second->setRelativeID(nd.relative_id);
        iMutex.unlock();
      }
      else {
        fprintf(stderr, "pattern not found: %s %s\n", key.c_str(),
                value.c_str());
        iMutex.unlock();
      }
    }

    void NodeManager::changeNode(SimNode *editedNode, NodeData *nodeS) {
      NodeData sNode = editedNode->getSNode();
      if(control->graphics) {
        Vector scale;
        if(sNode.filename == "PRIMITIVE") {
          scale = nodeS->ext;
          if(sNode.physicMode == NODE_TYPE_SPHERE) {
            scale.x() *= 2;
            scale.y() = scale.z() = scale.x();
          }
          // todo: set scale for cylinder and capsule
        } else {
          scale = sNode.visual_size-sNode.ext;
          scale += nodeS->ext;
          nodeS->visual_size = scale;
        }
        control->graphics->setDrawObjectScale(editedNode->getGraphicsID(), scale);
        control->graphics->setDrawObjectScale(editedNode->getGraphicsID2(), nodeS->ext);
      }
      editedNode->changeNode(nodeS);
      if(sNode.groupID != 0 || nodeS->groupID != 0) {
        for(auto it: simNodes) {
          if(it.second->getGroupID() == sNode.groupID ||
             it.second->getGroupID() == nodeS->groupID) {
            control->joints->reattacheJoints(it.second->getID());
          }
        }
      }
      control->joints->reattacheJoints(nodeS->index);

      if(nodeS->groupID > maxGroupID) {
        maxGroupID = nodeS->groupID;
      }
    }

  } // end of namespace sim
} // end of namespace mars
