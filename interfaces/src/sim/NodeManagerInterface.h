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
 * \file NodeManagerInterface.h
 * \author Malte Roemmermann \n
 * \brief "NodeManagerInterface" declares the interfaces for all NodeOperations
 * that are used for the communication between the simulation modules.
 *
 * \version 1.2
 * \date 02.01.2009
 */

#ifndef NODE_MANAGER_INTERFACE_H
#define NODE_MANAGER_INTERFACE_H

#ifdef _PRINT_HEADER_
  #warning "NodeManagerInterface.h"
#endif

#include "../sensor_bases.h"
#include "../NodeData.h"
#include "../nodeState.h"

#include <mars/utils/Vector.h>
#include <mars/utils/Quaternion.h>

namespace mars {

  namespace sim {
    class SimNode;
  };

  namespace interfaces {

    /**
     * \author Malte Langosz, Lorenz Quack \n
     * \brief "NodeManagerInterface" declares the interfaces for all NodeOperations
     * that are used for the communication between the simulation modules.
     *
     */
    class NodeManagerInterface {
    public:
      virtual ~NodeManagerInterface() {}

      /**
       *\brief Add a node to the node pool of the simulation.
       * 
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread).
       * \warning This method only adds the physical representation of a node. To
       * add a node completely use the other addNode function!
       *
       * \param nodeS Is a pointer to the NodeData that defines the new node.

       * Generally the default value should be used.
       * \param reload This param is used internally by the simulation. The
       * default value is \c false. If this param is set to \c true the new node
       * will not be reloaded by a reset of the simulation. But a simulation reset
       * only destroys the physical representation of all nodes. Thus a node
       * created with this param set to \c true will produce visual waste if the
       * node is not removed manually before reseting the simulation.
       * \return The unique id of the newly added node is returned.
       */
      virtual NodeId addNode(NodeData *nodeS,
                             bool reload = false,
                             bool loadGraphics = true) = 0;

  
  
      virtual NodeId createPrimitiveNode(const std::string &name, NodeType type,
                                         bool movable=false,
                                         const utils::Vector &pos=utils::Vector::Zero(),
                                         const utils::Vector &extension=utils::Vector::Identity(),
                                         double mass=0,
                                         const utils::Quaternion &orientation=utils::Quaternion::Identity(),
                                         bool disablePhysics=false) = 0;
  
  
      /**
       *\brief Add a terrain node to the node pool of the simulation.
       * 
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread).
       *
       * \param terrainS Is a pointer to the terrainStruct that defines the new
       * terrain node.
       * \return The unique id of the new added node is returned.
       */
      virtual NodeId addTerrain(terrainStruct *terrainS) = 0;

      /**
       *\brief Add a vector of nodes to the node pool of the simulation.
       * 
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread).
       * \warning This function is the right one to add a mesh to the simulation.
       * To add a primitive node use i_NodeManager::addPrimitive!
       *
       * \param v_NodeData Is a vector of NodeDatas that have to be added
       * to the simulation. In this method, the addNode function above is called
       * indirectly for every NodeData of the vector.
       * \return Returns a vector with the unique ids of the new added nodes.
       */
      virtual std::vector<NodeId> addNode(std::vector<NodeData> v_NodeData) = 0;

      /**
       *\brief Add a node of type primitive to the node pool of the simulation.
       * 
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread).
       * \warning This function is the right one to add a primitive node to
       * the simulation!
       *
       * \param snode The NodeData that defines the node to be added.
       * \return The unique id of the new added node is returned.
       */
      virtual NodeId addPrimitive(NodeData *snode) = 0;

      /**
       *\brief Returns the number of nodes that are added to the simulation.
       * 
       * It is very important to assure the serializaion between the threads to
       * have the desired results. This function should be thread save.
       *
       * \return The number of all added nodes is returned.
       */
      virtual int getNodeCount() const = 0;

      /**
       *\brief Returns the ID for the next node which will be added to the simulation.
       *
       * It is very important to assure the serializaion between the threads to
       * have the desired results. This function should be thread save.
       *
       * \return The ID for the next inserted node is returned.
       */
      virtual NodeId getNextNodeID() const = 0;

      /**
       *\brief This function supports the possibility to change some properties
       * of a node.
       * 
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread).
       *
       * \param nodeS The NodeData refereed by this pointer have to store
       * the node_id of the node to edit and the properties that have to be
       * changed. Which properties have to be changed is given by the next param.
       * \param changes This parameter defines what properties of the node have
       * to be edited. Possible options are:
       * - EDIT_NODE_POS
       * - EDIT_NODE_ROT
       * - EDIT_NODE_MOVE_ALL
       * - EDIT_NODE_SIZE
       * - EDIT_NODE_TYPE
       * - EDIT_NODE_MASS
       */
      virtual void editNode(NodeData *nodeS, int changes) = 0;

      /**
       * \brief This function is not implemented yet. To change a group of
       * a node it has to be removed and added again with a new group_id.
       */
      virtual void changeGroup(NodeId id, int group) = 0;

      /**
       * \brief Gives information about core exchange data for nodes.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread).
       *
       * \param nodeList A pointer to a vector that is filled with a
       * core_objects_exchange struct for every node. The vector is cleared
       * in the beginning of this function.
       */
      virtual void getListNodes(std::vector<core_objects_exchange> *nodeList) const = 0;

      /**
       * \brief Gives information about core exchange data for a certain node.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread).
       * \param id The unique id of the node to get information from.
       * \param obj A pointer to a core_objects_exchange struct that will be
       * filled with a the core exchange data of the node with the given id.
       * \sa i_SimLog
       */
      virtual void getNodeExchange(NodeId id, core_objects_exchange *obj) const = 0;

      /**
       * \brief Gives all information of a certain node.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread).
       * \param id The unique id of the node to get information from.
       * \returns A copy of the NodeData of the node with the given id.
       */
      virtual const NodeData getFullNode(NodeId id) const = 0;

      /**
       * \brief Removes a node from the simulation.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread).
       * \warning Currently this function only removes the physical representation
       * of a node. To remove a node completely use
       * MainWindowInterface::removeNode() instead.
       * \param id The unique id of the node to remove form the simulation.
       */
      virtual void removeNode(NodeId id, bool clearGraphics=true) = 0;

      /**
       * \brief Set a state of a node. ## This function is not fully implemented
       * yet ##
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread).
       *
       * \param id The id of the node to edit.
       * \param state The new state of the node. Currently the angular and linear
       * velocity of the state is applied to the node. \sa nodeState
       */
      virtual void setNodeState(NodeId id, const nodeState &state) = 0;
      virtual void getNodeState(NodeId id, nodeState *state) const = 0;

      /**
       * \brief Gives the center of mass of a set of nodes.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread).
       *
       * \param ids This vector defines the ids of the nodes that will be
       * included into the calculation of the center of mass.
       * \returns A Vector representing the center of mass for the specified ids
       * is returned.
       */   
      virtual const utils::Vector getCenterOfMass(const std::vector<NodeId> &ids) const = 0;

      /**
       * \brief Sets the new size of a node after a reset of the simulation.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread).
       *
       * \param id The id of the node to edit.
       * \param ext The new size of the node. For more information of how the
       * vector is interpreted by the simulation see NodeData::ext.
       * \returns A Vector with the ratio of the old and the new size for every
       * dimension.
       */   
      virtual const utils::Vector setReloadExtent(NodeId id, const utils::Vector &ext) = 0;

      /**
       * \brief Sets the new position of a node after a reset of the simulation.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread).
       *
       * \param id The id of the node to edit.
       * \param pos The new position of the node.
       */   
      virtual void setReloadPosition(NodeId id, const utils::Vector &pos) = 0;

      /**
       * \brief Sets the new contact friction of a node after a reset of the
       * simulation.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread).
       *
       * \param id The id of the node to edit.
       * \param friction1 The new contact friction coefficient for the first
       * friction direction.
       * \param friction2 The new contact friction coefficient for the second
       * friction direction.
       * \sa contact_params
       */   
      virtual void setReloadFriction(NodeId id, sReal friction1,
                                     sReal friction2) = 0;

      /**
       * \brief Sets the current position of a node.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread).
       *
       * \param id The id of the node to edit.
       * \param pos The new position of the node.
       */
      virtual void setPosition(NodeId id, const utils::Vector &pos) = 0;

      /**
       * \brief Returns the current position of a node.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread).
       *
       * \param id The id of the node to get the position from.
       * \returns The position of the node is returned.
       */
      virtual const utils::Vector getPosition(NodeId id) const = 0;

      /**
       * \brief Returns the current orientation of a node.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread).
       *
       * \param id The id of the node to get the orientation from.
       * \returns The orientation of the node is returned.
       */
      virtual const utils::Quaternion getRotation(NodeId id) const = 0;

      /**
       * \brief Sets the current orientation of a node.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread).
       *
       * \param id The id of the node to edit.
       * \param rot The new orientation of the node.
       */
      virtual void setRotation(NodeId id, const utils::Quaternion &rot) = 0;

      /**
       * \brief Gets the current linear velocity of a node.
       *
       * \param id The id of the node to get the velocity from.
       * \returns The linear velocity of the node.
       */
      virtual const utils::Vector getLinearVelocity(NodeId id) const = 0;

      /**
       * \brief Gets the current angular velocity of a node.
       *
       * \param id The id of the node to get the angular/rotational velocity from.
       * \returns The angular/rotational velocity of the node.
       */
      virtual const utils::Vector getAngularVelocity(NodeId id) const = 0;

      /**
       * \brief Gets the current linear acceleration of a node.
       *
       * \param id The id of the node to get the acceleration from.
       * \returns The linear acceleration of the node.
       */
      virtual const utils::Vector getLinearAcceleration(NodeId id) const = 0;

      /**
       * \brief Gets the current angular/rotational acceleration of a node.
       *
       * \param id The id of the node to get the angular/rotational acceleration
       *           from.
       * \returns The angular/rotational acceleration of the node.
       */
      virtual const utils::Vector getAngularAcceleration(NodeId id) const = 0;

      /**
       * \brief Applies a off-center force to a physical node.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread). \n \n This function applies forces to the simulation,
       * which can produce instability if it is done with a high frequency.
       *
       * \param id The id of the node to apply a force.
       * \param force A Vector that defines the direction and the strength (length
       * of the vector) of the force to be applied.
       * \param pos The force is applied to the given position.
       */
      virtual void applyForce(NodeId id, const utils::Vector &force,
                              const utils::Vector &pos) = 0;

      /**
       * \brief Applies a force to a physical node.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread). \n \n This function applies forces to the simulation,
       * which can produce instability if it is done with a high frequency.
       *
       * \param id The id of the node to apply a force.
       * \param force A Vector that defines the direction and the strength (length
       * of the vector) of the force to be applied.
       */
      virtual void applyForce(NodeId id, const utils::Vector &force) = 0;

      /**
       * \brief Applies a torque to a physical node.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread). \n \n This function applies forces to the simulation,
       * which can produce instability if it is done with a high frequency.
       *
       * \param id The id of the node to apply a torque.
       * \param force A Vector that defines the axis and the strength (length
       * of the vector) of the torque to be applied. The torque will be applied at.
       * the center of mass of the node.
       */
      virtual void applyTorque(NodeId id, const utils::Vector &torque) = 0;

      /**
       * \brief Sets a specific contact parameter; the motion of the contact.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread). \n \n This contact parameter must be enabled by
       * the contact_params of the node first. If it is enabled, it can be used
       * to simulate effects like a convener belt. \sa contact_params
       *
       * \param id The id of the node to set the motion contact parameter.
       * \param motion The velocity to set for the contact motion.
       */
      virtual void setContactParamMotion1(NodeId id, sReal motion) = 0;

      /**
       * \brief This function is used to add sensor to a node that need more
       * specific physical implementations.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread). \n \n Right now, this method is used to add range
       * sensors to the simulation by adding a necessary sensor configuration
       * to a node. \warning This method is used internally by the simulation,
       * more specific by some sensor implementations. This method don't have
       * to be used elsewhere, normally. \sa RaySensor RayGridSensor
       *
       * \param id The id of the node to add a sensor.
       * \param s_cfg The sensor configuration struct. \sa sensor_config
       */
      virtual void addNodeSensor(BaseNodeSensor *sensor) = 0;

      /**
       * \brief This function resets a added sensor by removing it and add a new
       * defined by the sensor config struct.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread). \n \n This method is implemented for a special case
       * where the distance sensors have to be reconfigured. Don't know if it's
       * used right now (maybe in the A3 project).
       *
       * \param id The id of the node to reload a sensor.
       * \param s_cfg The sensor configuration struct. \sa sensor_config
       */
      virtual void reloadNodeSensor(BaseNodeSensor *sensor) = 0;

      /**
       * \brief This function returns the SimNode object for a given id.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread). \warning This method is only internal used by the
       * JointManager. Generally no other modules know the SimNode class and
       * shouldn't use this method. All node operations from outside the core
       * should be done over the NodeManager.
       *
       * \param id The id of the node to get the core node object.
       * \returns Returns a pointer to the corresponding node object.
       */
      virtual sim::SimNode *getSimNode(NodeId id) = 0;
      virtual const sim::SimNode *getSimNode(NodeId id) const = 0;

      /**
       * \brief This function reloads all node from a temporally NodeData pool.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread). \warning This method should only be used internally.
       * This function don't removes any nodes, before calling this function
       * i_NodeManager::clearAllNodes should be executed. To reload the simulation
       * the function i_GuiToSim::spotReload should be used.
       * \sa i_GuiToSim::spotReload
       */
      virtual void reloadNodes(bool reloadGraphics) = 0;

      /**
       * \brief Updates the node values of dynamic nodes from the physics.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread). \warning This method is called after each simulation
       * step and don't have to be used in other cases, generally.
       */
      virtual void updateDynamicNodes(sReal calc_ms, bool physics_thread=true) = 0;

      /**
       * \brief This function destroys all nodes within the simulation.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread). \warning This method don't destroys the visual
       * object, nor the joints of the simulation. To clear the whole world call
       * i_GuiToSim::newWorld. But also that function don't clears the visual
       * nodes right now.
       * \sa i_GuiToSim::newWorld
       */
      virtual void clearAllNodes(bool clear_all=false,
                                 bool clearGraphics=true) = 0;

      /**
       * \brief Sets the new orientation of a node after a reset of the simulation.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread).
       *
       * \param id The id of the node to edit.
       * \param angle The new orientation given in euler angles.
       */   
      virtual void setReloadAngle(NodeId id, const utils::sRotation &angle) = 0;

      /**
       * \brief Sets the contact properties of the node.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread).
       *
       * \param id The id of the node to edit.
       * \param pos The contact parameter struct. For detailed information see
       * contact_params. \sa contact_params
       */   
      virtual void setContactParams(NodeId id, const contact_params &cp) = 0;

      /**
       * \brief Returns the contact properties of a certain node.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread).
       *
       * \param id The id of the node to get the contact properties from.
       * \returns A contact parameter struct which stores the contact properties.
       * \sa contact_params
       */   
      virtual const contact_params getContactParams(NodeId id) const = 0;

      /**
       * \brief Sets the linear velocity of a node.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread).
       *
       * \param id The id of the node to edit.
       * \param vel The new linear velocity of the node. The vector describes the
       * direction and the velocity (length of the vector).
       */
      virtual void setVelocity(NodeId id, const utils::Vector &vel) = 0;

      /**
       * \brief Sets the angular velocity of a node.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread).
       *
       * \param id The id of the node to edit.
       * \param vel The new angular velocity of the node. The vector describes the
       * axis of the rotation and the angular velocity (length of the vector).
       */
      virtual void setAngularVelocity(NodeId id, const utils::Vector &vel) = 0;

      /**
       * \brief Scales the size of all node after a reset of the simulation.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread).
       *
       * \param factor The scaling factor for the size of the nodes.
       */   
      virtual void scaleReloadNodes(sReal x, sReal y, sReal z) = 0;

      /**
       * \brief Returns the mass and inertia of a certain node.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread).
       *
       * \param id The id of the node to get the mass information from.
       * \param mass A pointer to the sReal variable, where the mass will be stored.
       * \param inertia A pointer to a sReal array with a dimension of 9. If the
       * pointer is unequal 0, the inertia information will be stored in this array.
       */   
      virtual void getNodeMass(NodeId id, sReal *mass, sReal *inertia=0) const = 0;

      /**
       * \brief Sets a angular damping factor to the node dynamics.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread).
       *
       * \param id The id of the node to add the angular damping.
       * \param damping Factor to multiply with the velocity of the node after
       * every simulation step. The value should be between 0 and 1, while 0 means
       * no damping and 1 the maximum damping.
       * \f$ angular\_velocity = angular\_velocity * (1-damping)\f$
       */   
      virtual void setAngularDamping(NodeId id, sReal damping) = 0;

      /**
       * \brief Adds a additional rotation to the orientation of a node.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread).
       *
       * \param id The id of the node to add the rotation.
       * \param q The rotation to be added (see Quaternion).
       */   
      virtual void addRotation(NodeId id, const utils::Quaternion &q) = 0;

      /**
       * \brief Sets the new orientation of a node after a reset of the simulation.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread).
       *
       * \param id The id of the node to edit.
       * \param q The new orientation given as quaternion.
       */   
      virtual void setReloadQuaternion(NodeId id, const utils::Quaternion &q) = 0;

      /**
       * \brief If a graphicsManager is loaded, exports the graphical models
       * of all nodes. Every node is exported by its id to a seperated file into
       * the given folder.
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results. Currently the verified use of this function is
       * only guaranteed by calling it within the main thread (update callback
       * from \c gui_thread).
       *
       * \param folder The folder where to exort the models.
       */   
      virtual void exportGraphicNodesByID(const std::string &folder) const = 0;

      /** \todo write docs */
      virtual std::vector<NodeId> getConnectedNodes(NodeId id) = 0;

      /** \todo write docs */
      virtual void getContactPoints(std::vector<NodeId> *ids,
                                    std::vector<utils::Vector> *contact_points) const = 0;

      /** \todo write docs */
      virtual void updateRay(NodeId id) = 0;
      /** \todo write docs */
      virtual NodeId getDrawID(NodeId id) const = 0;
      /** \todo write docs */
      virtual void setVisualRep(NodeId id, int val) = 0;
      /** \todo write docs */
      virtual const utils::Vector getContactForce(NodeId id) const = 0;

      /**
       * Retrieve the id of a node by name
       * \param node_name Name of the node to get the id for
       * \return Id of the node if it exists, otherwise 0
       */
      virtual NodeId getID(const std::string& node_name) const = 0;
      /** \todo write docs */
      virtual double getCollisionDepth(NodeId id) const = 0;

      /**
       * Retrieves the \a groupName and \a dataName under which the node with the
       * specified \a id publishes its data in the DataBroker
       * \return \c true if the names were successfully retrieved. \c false if
       *         no node with the given \a id exists.
       */ 
      virtual bool getDataBrokerNames(NodeId id, std::string *groupName, 
                                      std::string *dataName) const = 0;

      /** \todo write docs */
      virtual void setVisualQOffset(NodeId id, const utils::Quaternion &q) = 0;

      /** \todo write docs */
      virtual void updatePR(unsigned long id, const utils::Vector &pos,
                            const utils::Quaternion &rot,
                            const utils::Vector &visOffsetPos,
                            const utils::Quaternion &visOffsetRot,
                            bool doLock = true) = 0;

      virtual bool getIsMovable(NodeId id) const = 0;
      virtual void setIsMovable(NodeId id, bool isMovable) = 0;
      virtual void lock() = 0;
      virtual void unlock() = 0;
      
      /** Rotates the specified node according to the provided quaternion.
       * \param id The \c id of the node to be rotated.
       * \param pivot The utils::Vector defining the pivot point around which to rotate.
       * \param q The utils::Quaternion defining the rotation vector and angle.
       * \param excludeJointId ?
       */
      virtual void rotateNode(NodeId id, utils::Vector pivot, utils::Quaternion q,
          unsigned long excludeJointId, bool includeConnected = true) = 0;

      /** Positions the node according to the provided vector.
       * \param id The \c id of the node to be moved.
       * \param pos The utils::Vector defining the new position of the node.
       * \param excludeJointId ?
       */
      virtual void positionNode(NodeId id, utils::Vector pos, unsigned long excludeJointId) = 0;
      virtual unsigned long getMaxGroupID() = 0;

      virtual void printNodeMasses(bool onlysum) = 0;

      /** Edit a node property by giving a key and value.
       * \param id The \c id of the node to be edit.
       * \param key Defines the key in the ConfigMap. Could also include
                      the path, in the end pattern matching is used.
       * \param value String containing the value.
       */
      virtual void edit(NodeId id, const std::string &key,
                        const std::string &value) = 0;
    };

  } // end of namespace interfaces
} // end of namespace mars

#endif  // NODE_MANAGER_INTERFACE_H
