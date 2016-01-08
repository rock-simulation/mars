#ifndef NODE_MANAGER_H
#define NODE_MANAGER_H

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
 * \file NodeManager.h
 * \author Malte Roemmermann
 * \brief "NodeManager" is the class that manage all nodes and their
 * operations and communication between the different modules of the simulation.
 */

#ifdef _PRINT_HEADER_
  #warning "NodeManager.h"
#endif

#include <mars/utils/Mutex.h>
#include <mars/interfaces/graphics/GraphicsUpdateInterface.h>
#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/interfaces/sim/NodeManagerInterface.h>

namespace mars {
  namespace sim {

    class SimJoint;
    class SimNode;

    typedef std::map<interfaces::NodeId, SimNode*> NodeMap;

    /**
     * The declaration of the NodeManager class.
     *
     */
    class NodeManager : public interfaces::NodeManagerInterface,
                        public interfaces::GraphicsUpdateInterface {
    public:
      NodeManager(interfaces::ControlCenter *c,
                  lib_manager::LibManager *theManager);
      virtual ~NodeManager(){}

      virtual interfaces::NodeId createPrimitiveNode(const std::string &name,
                                                     interfaces::NodeType type,
                                                     bool movable=false,
                                                     const utils::Vector &pos=utils::Vector::Zero(),
                                                     const utils::Vector &extension=utils::Vector::Identity(),
                                                     double mass=0,
                                                     const utils::Quaternion &orientation=utils::Quaternion::Identity(),
                                                     bool disablePhysics=false);

      virtual interfaces::NodeId addNode(interfaces::NodeData *nodeS,
                                         bool reload = false,
                                         bool loadGraphics = true);
      virtual interfaces::NodeId addTerrain(interfaces::terrainStruct *terrainS);
      virtual std::vector<interfaces::NodeId> addNode(std::vector<interfaces::NodeData> v_NodeData);
      virtual interfaces::NodeId addPrimitive(interfaces::NodeData *snode);
      virtual int getNodeCount() const;
      virtual interfaces::NodeId getNextNodeID() const;
      virtual void editNode(interfaces::NodeData *nodeS, int changes);
      virtual void changeGroup(interfaces::NodeId id, int group);
      virtual void getListNodes(std::vector<interfaces::core_objects_exchange> *nodeList) const;
      virtual void getNodeExchange(interfaces::NodeId id,
                                   interfaces::core_objects_exchange *obj) const;
      virtual const interfaces::NodeData getFullNode(interfaces::NodeId id) const;
      virtual void removeNode(interfaces::NodeId id, bool clearGraphics=true);
      virtual void setNodeState(interfaces::NodeId id, const interfaces::nodeState &state);
      virtual void getNodeState(interfaces::NodeId id, interfaces::nodeState *state) const;
      virtual const utils::Vector getCenterOfMass(const std::vector<interfaces::NodeId> &ids) const;
      virtual void setPosition(interfaces::NodeId id, const utils::Vector &pos);
      virtual const utils::Vector getPosition(interfaces::NodeId id) const;
      virtual void setRotation(interfaces::NodeId id, const utils::Quaternion &rot);
      virtual const utils::Quaternion getRotation(interfaces::NodeId id) const;
      virtual const utils::Vector getLinearVelocity(interfaces::NodeId id) const;
      virtual const utils::Vector getAngularVelocity(interfaces::NodeId id) const;
      virtual const utils::Vector getLinearAcceleration(interfaces::NodeId id) const;
      virtual const utils::Vector getAngularAcceleration(interfaces::NodeId id) const;
      virtual void applyForce(interfaces::NodeId id, const utils::Vector &force,
                              const utils::Vector &pos);
      virtual void applyForce(interfaces::NodeId id, const utils::Vector &force);
      virtual void applyTorque(interfaces::NodeId id, const utils::Vector &torque);
      virtual void setContactParamMotion1(interfaces::NodeId id, interfaces::sReal motion);
      virtual void addNodeSensor(interfaces::BaseNodeSensor *sensor);
      virtual void reloadNodeSensor(interfaces::BaseNodeSensor *sensor);
      virtual SimNode* getSimNode(interfaces::NodeId id);
      virtual const SimNode* getSimNode(interfaces::NodeId id) const;
      virtual void reloadNodes(bool reloadGraphics);
      virtual const utils::Vector setReloadExtent(interfaces::NodeId id, const utils::Vector &ext);
      virtual void setReloadPosition(interfaces::NodeId id, const utils::Vector &pos);
      virtual void setReloadFriction(interfaces::NodeId id, interfaces::sReal friction1,
                                     interfaces::sReal friction2);
      virtual void updateDynamicNodes(interfaces::sReal calc_ms, bool physics_thread = true);
      virtual void clearAllNodes(bool clear_all=false, bool clearGraphics=true);
      virtual void setReloadAngle(interfaces::NodeId id, const utils::sRotation &angle);
      virtual void setContactParams(interfaces::NodeId id, const interfaces::contact_params &cp);
      virtual const interfaces::contact_params getContactParams(interfaces::NodeId id) const;
      virtual void setVelocity(interfaces::NodeId id, const utils::Vector& vel);
      virtual void setAngularVelocity(interfaces::NodeId id, const utils::Vector &vel);
      virtual void scaleReloadNodes(interfaces::sReal x, interfaces::sReal y, interfaces::sReal z);
      virtual void getNodeMass(interfaces::NodeId id, interfaces::sReal *mass, interfaces::sReal *inertia = 0) const;
      virtual void setAngularDamping(interfaces::NodeId id, interfaces::sReal damping);
      virtual void addRotation(interfaces::NodeId id, const utils::Quaternion &q);
      virtual void setReloadQuaternion(interfaces::NodeId id, const utils::Quaternion &q);
      virtual void preGraphicsUpdate(void);
      virtual void exportGraphicNodesByID(const std::string &folder) const;
      virtual void getContactPoints(std::vector<interfaces::NodeId> *ids,
                                    std::vector<utils::Vector> *contact_points) const;
      virtual void getContactIDs(const interfaces::NodeId &id,
                                 std::list<interfaces::NodeId> *ids) const;
      virtual void updateRay(interfaces::NodeId id);
      virtual interfaces::NodeId getDrawID(interfaces::NodeId id) const;
      virtual void setVisualRep(interfaces::NodeId id, int val);
      virtual const utils::Vector getContactForce(interfaces::NodeId id) const;
      virtual void setVisualQOffset(interfaces::NodeId id, const utils::Quaternion &q);

      virtual void updatePR(interfaces::NodeId id, const utils::Vector &pos,
                            const utils::Quaternion &rot,
                            const utils::Vector &visOffsetPos,
                            const utils::Quaternion &visOffsetRot,
                            bool doLock = true);
      /**
       * Retrieve the id of a node by name
       * \param node_name Name of the node to get the id for
       * \return Id of the node if it exists, otherwise 0
       */
      virtual interfaces::NodeId getID(const std::string& node_name) const;
      virtual double getCollisionDepth(interfaces::NodeId id) const;
      virtual bool getDataBrokerNames(interfaces::NodeId id, std::string *groupName,
                                      std::string *dataName) const;

      virtual std::vector<interfaces::NodeId> getConnectedNodes(interfaces::NodeId id);

      virtual bool getIsMovable(interfaces::NodeId id) const;
      virtual void setIsMovable(interfaces::NodeId id, bool isMovable);
      virtual void lock() {iMutex.lock();}
      virtual void unlock() {iMutex.unlock();}
      virtual void rotateNode(interfaces::NodeId id, utils::Vector pivot,
                              utils::Quaternion q,
                              unsigned long excludeJointId, bool includeConnected = true);
      virtual void positionNode(interfaces::NodeId id, utils::Vector pos,
                                unsigned long excludeJointId);
      virtual unsigned long getMaxGroupID() { return maxGroupID; }
      virtual void edit(interfaces::NodeId id, const std::string &key,
                        const std::string &value);

    private:
      interfaces::NodeId next_node_id;
      bool update_all_nodes;
      int visual_rep;
      NodeMap simNodes;
      NodeMap simNodesDyn;
      NodeMap nodesToUpdate;
      std::list<interfaces::NodeData> simNodesReload;
      unsigned long maxGroupID;
      lib_manager::LibManager *libManager;
      mutable utils::Mutex iMutex;

      interfaces::ControlCenter *control;

      std::list<interfaces::NodeData>::iterator getReloadNode(interfaces::NodeId id);

      // interfaces::NodeInterface* getNodeInterface(NodeId node_id);
      struct Params; // see below.
      // recursively walks through the gids and joints and
      // applies the applyFunc with the given parameters.
      void recursiveHelper(interfaces::NodeId id, const Params *params,
                           std::vector<SimJoint*> *joints,
                           std::vector<int> *gids,
                           NodeMap *nodes,
                           void (*applyFunc)(SimNode *node, const Params *params));
      void moveNodeRecursive(interfaces::NodeId id, const utils::Vector &offset,
                             std::vector<SimJoint*> *joints,
                             std::vector<int> *gids,
                             NodeMap *nodes);
      void rotateNodeRecursive(interfaces::NodeId id,
                               const utils::Vector &rotation_point,
                               const utils::Quaternion &rotation,
                               std::vector<SimJoint*> *joints,
                               std::vector<int> *gids,
                               NodeMap *nodes);
      // these static methods are used by moveNodeRecursive and rotateNodeRecursive
      // as applyFuncs for the recursiveHelper method
      static void applyMove(SimNode *node, const Params *params);
      static void applyRotation(SimNode *node, const Params *params);

      void moveRelativeNodes(const SimNode &node, NodeMap *nodes, utils::Vector v);
      void rotateRelativeNodes(const SimNode &node, NodeMap *nodes,
                               utils::Vector pivot, utils::Quaternion rot);

      void resetRelativeNodes(const SimNode &node,
                              NodeMap *nodes,
                              const utils::Quaternion *rotate = 0);
      void resetRelativeJoints(const SimNode &node,
                               NodeMap *nodes,
                               std::vector<SimJoint*> *joints,
                               const utils::Quaternion *rotate = 0);
      void setNodeStructPositionFromRelative(interfaces::NodeData *node) const;
      void clearRelativePosition(interfaces::NodeId id, bool lock);
      void removeNode(interfaces::NodeId id, bool lock,
                      bool clearGraphics=true);
      void pushToUpdate(SimNode* node);

      void printNodeMasses(bool onlysum);

      // for passing parameters to the recursiveHelper.
      struct Params
      {
        // make virtual so we can use polymorphism
        virtual ~Params() {}
      };
      struct MoveParams : Params
      {
        utils::Vector offset;
      };
      struct RotationParams : Params
      {
        utils::Vector rotation_point;
        utils::Quaternion rotation;
      };

    };

  } // end of namespace sim
} // end of namespace mars

#endif  // NODE_MANAGER_H
