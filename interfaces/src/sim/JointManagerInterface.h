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
 * \file JointManagerInterface.h
 * \author Malte Roemmermann
 * \brief "JointManagerInterface" declares the interfaces for all
 * JointOperations that are used for the communication between the
 * simulation modules.
 *
 */

#ifndef JOINT_MANAGER_INTERFACE_H
#define JOINT_MANAGER_INTERFACE_H

#ifdef _PRINT_HEADER_
  #warning "JointManagerInterface.h"
#endif

#include "../JointData.h"
#include "../core_objects_exchange.h"

namespace mars {

  namespace sim {
    class SimJoint;
  }

  namespace interfaces {

    /**
     * Interface class for the node organization.
     *
     */
    class JointManagerInterface {
    public:
      virtual ~JointManagerInterface() {}

      /**
       * \brief Add a joint to the joint pool of the simulation
       *
       * It is very important to assure the serialization between the threads to
       * have the desired results.
       *
       * pre:
       */
      virtual unsigned long addJoint(JointData *jointS, bool reload = false) = 0;

      /**
       *\brief Returns the number of joints added to the simulation
       */
      virtual int getJointCount() = 0;

      /**
       * \brief Change a Joint. The simulation must be updated in here.
       * \todo write docs
       */
      virtual void editJoint(JointData *jointS) = 0;

      /**
       * \brief Fills a list of core_object_exchange objects with joint
       *        informations.
       */
      virtual void getListJoints(std::vector<core_objects_exchange> *jointList) = 0;

      /**
       * \brief Fills one core_object_exchange object with joint information
       *        of the joint with the given id.
       */
      virtual void getJointExchange(unsigned long id, core_objects_exchange *obj) = 0;

      /**
       * \brief get the full struct of a joint for editing purposes.
       * \throw std::runtime_error if the joint cannot be found
       */
      virtual const JointData getFullJoint(unsigned long index) = 0;

      /**
       * \brief Removes the joint with the given id
       */
      virtual void removeJoint(unsigned long id) = 0;

      /**
       * \brief removes the joint connecting the two nodes with the given ids
       * \param id1 the id of one of the nodes attached to the joint to be removed
       * \param id2 the id of the other node attached to the joint to be removed
       */
      virtual void removeJointByIDs(unsigned long id1, unsigned long id2) = 0;

      /**
       * \brief Returns a pointer to the SimJoint Object.
       */
      virtual std::shared_ptr<mars::sim::SimJoint> getSimJoint(unsigned long id) = 0;

      /**
       * \brief Return a copy of the SimJointVector;
       */
      virtual std::vector<sim::SimJoint*> getSimJoints(void) = 0;

      /**
       * \brief Reattaches the joints that are connected to the node
       *        with id \a node_id.
       */
      virtual void reattacheJoints(unsigned long node_id) = 0;

      /**
       * \brief Reloads all joints in the simulation.
       */
      virtual void reloadJoints(void) = 0;

      /**
       * \brief Update the Joint values from the physics
       */
      virtual void updateJoints(sReal calc_ms) = 0;

      /**
       * \brief Removes all joints from the simulation to clear the world.
       */
      virtual void clearAllJoints(bool clear_all=false) = 0;

      /**
       * \brief Set a new joint offset for reloading a joint.
       */
      virtual void setReloadJointOffset(unsigned long id, sReal offset) = 0;

      /**
       * \brief Set a new axis for reloading a joint.
       */
      virtual void setReloadJointAxis(unsigned long id, const utils::Vector &axis) = 0;

      /** \todo write docs */
      virtual void scaleReloadJoints(sReal x, sReal y, sReal z) = 0;
      /** \todo write docs */
      virtual void setJointTorque(unsigned long id, sReal torque) = 0;
      /** \todo write docs */
      virtual void changeStepSize(void) = 0;
      /** \todo write docs */
      virtual void setReloadAnchor(unsigned long id, const utils::Vector &anchor) = 0;
      /** \todo write docs */
      virtual void setSDParams(unsigned long id, JointData *sJoint) = 0;

      /**
       * \brief sets the velocity of the first axis of the joint
       * \param id the id of the joint whos velocity should be set
       * \param the new velocity
       */
      virtual void setVelocity(unsigned long id, sReal velocity) = 0;

      /**
       * \brief sets the velocity of the second axis of the joint
       * \param id the id of the joint whos velocity should be set
       * \param the new velocity
       * If the joint has no second axis the behaviour is undefined.
       */
      virtual void setVelocity2(unsigned long id, sReal velocity) = 0;

      /** \todo write docs */
      virtual void setForceLimit(unsigned long id, sReal max_force,
                                 bool first_axis = 1) = 0;


      /**
       * Retrieve the id of a joint by name
       * \param joint_name Name of the joint to get the id for
       * \return Id of the joint if it exists, otherwise 0
       */
      virtual unsigned long getID(const std::string &joint_name) const = 0;

      /**
       * Retrieve the id of a joint by the ids of the connected nodes
       * \param id1, id2 Ids of the connected nodes
       * \return Id of the joint if it exists, otherwise 0
       */
      virtual unsigned long getIDByNodeIDs(unsigned long id1, unsigned long id2) = 0;

      /**
       * Retrieves the \a groupName and \a dataName under which the joint with the
       * specified \a id publishes its data in the DataBroker
       * \return \c true if the names were successfully retrieved. \c false if
       *         no joint with the given \a id exists.
       */
      virtual bool getDataBrokerNames(unsigned long id, std::string *groupName,
                                      std::string *dataName) const = 0;

      virtual void setOfflineValue(unsigned long id, sReal value) = 0;

      virtual interfaces::sReal getLowStop(unsigned long id) const = 0;
      virtual interfaces::sReal getHighStop(unsigned long id) const = 0;
      virtual interfaces::sReal getLowStop2(unsigned long id) const = 0;
      virtual interfaces::sReal getHighStop2(unsigned long id) const = 0;
      virtual void setLowStop(unsigned long id,
                              interfaces::sReal lowStop) = 0;
      virtual void setHighStop(unsigned long id,
                               interfaces::sReal highStop) = 0;
      virtual void setLowStop2(unsigned long id,
                               interfaces::sReal lowStop2) = 0;
      virtual void setHighStop2(unsigned long id,
                                interfaces::sReal highStop2) = 0;
      virtual void edit(interfaces::JointId id, const std::string &key,
                        const std::string &value) = 0;
    };

  } // end of namespace interfaces
} // end of namespace mars

#endif  // JOINT_MANAGER_INTERFACE_H
