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
 * \file JointManager.cpp
 * \author Malte Roemmermann
 * \brief "JointManager" is the class that manage all joints and their
 * operations and communication between the different modules of the simulation.
 *
 */

#include "SimNode.h"
#include "SimJoint.h"
#include "JointManager.h"
#include "NodeManager.h"
#include "PhysicsMapper.h"


#include <stdexcept>

#include <mars/interfaces/sim/SimulatorInterface.h>
#include <mars/interfaces/sim/MotorManagerInterface.h>
#include <mars/utils/mathUtils.h>
#include <mars/utils/MutexLocker.h>
#include <mars/interfaces/Logging.hpp>
#include <mars/data_broker/DataBrokerInterface.h>

namespace mars {
  namespace sim {

    using namespace utils;
    using namespace interfaces;
    using namespace std;

    /**
     *\brief Initialization of a new JointManager
     *
     * pre:
     *     - a pointer to a ControlCenter is needed
     * post:
     *     - next_node_id should be initialized to one
     */
    JointManager::JointManager(ControlCenter *c) {
      control = c;
      next_joint_id = 1;
    }

    unsigned long JointManager::addJoint(JointData *jointS, bool reload) {
      JointInterface *newJointInterface;
      std::vector<SimNode*>::iterator iter;
      SimNode *node1 = 0;
      SimNode *node2 = 0;
      NodeInterface *i_node1 = 0;
      NodeInterface *i_node2 = 0;
      Vector an;

      if (!reload) {
        iMutex.lock();
        simJointsReload.push_back(*jointS);
        iMutex.unlock();
      }

      //if(jointS->axis1.lengthSquared() < Vector::EPSILON && jointS->type != JOINT_TYPE_FIXED) {
      if(jointS->axis1.squaredNorm() < EPSILON && jointS->type != JOINT_TYPE_FIXED) {
        LOG_ERROR("Cannot create joint without axis1");
        return 0;
      }

      // create an interface object to the physics
      newJointInterface = PhysicsMapper::newJointPhysics(control->sim->getPhysics());
      // reset the anchor
      //if node index is 0, the node connects to the environment.
      node1 = control->nodes->getSimNode(jointS->nodeIndex1);
      if (node1) i_node1 = node1->getInterface();
      node2 = control->nodes->getSimNode(jointS->nodeIndex2);
      if (node2) i_node2 = node2->getInterface();

      // ### important! how to deal with different load options? ###
      //if (load_option == OPEN_INITIAL)
      //jointS->angle1_offset = jointS->angle2_offset = 0;
      if (jointS->anchorPos == ANCHOR_NODE1) {
        assert(node1);
        jointS->anchor = node1->getPosition();
      } else if (jointS->anchorPos == ANCHOR_NODE2) {
        assert(node2);
        jointS->anchor = node2->getPosition();
      } else if (jointS->anchorPos == ANCHOR_CENTER) {
        assert(node1);
        assert(node2);
        jointS->anchor = (node1->getPosition() + node2->getPosition()) / 2.;
      }

      // create the physical node data
      if (newJointInterface->createJoint(jointS, i_node1, i_node2)) {
        // put all data to the correct place
        iMutex.lock();
        // set the next free id
        jointS->index = next_joint_id;
        next_joint_id++;
        SimJoint* newJoint = new SimJoint(control, *jointS);
        newJoint->setAttachedNodes(node1, node2);
        //    newJoint->setSJoint(*jointS);
        newJoint->setPhysicalJoint(newJointInterface);
        simJoints[jointS->index] = newJoint;
        iMutex.unlock();
        control->sim->sceneHasChanged(false);
        return jointS->index;
      } else {
        std::cerr << "JointManager: Could not create new joint (JointInterface::createJoint() returned false)." << std::endl;
        // if no node was created in physics
        // delete the objects
        delete newJointInterface;
        // and return false
        return 0;
      }
    }

    int JointManager::getJointCount() {
      MutexLocker locker(&iMutex);
      return simJoints.size();
    }

    void JointManager::editJoint(JointData *jointS) {
      MutexLocker locker(&iMutex);
      std::map<unsigned long, SimJoint*>::iterator iter = simJoints.find(jointS->index);
      if (iter != simJoints.end()) {
        iter->second->setAnchor(jointS->anchor);
        iter->second->setAxis(jointS->axis1);
        iter->second->setAxis(jointS->axis2, 2);
        iter->second->setLowerLimit(jointS->lowStopAxis1);
        iter->second->setUpperLimit(jointS->highStopAxis1);
        iter->second->setLowerLimit(jointS->lowStopAxis2, 2);
        iter->second->setUpperLimit(jointS->highStopAxis2, 2);
      }
    }

    void JointManager::getListJoints(std::vector<core_objects_exchange>* jointList) {
      core_objects_exchange obj;
      std::map<unsigned long, SimJoint*>::iterator iter;
      MutexLocker locker(&iMutex);
      jointList->clear();
      for (iter = simJoints.begin(); iter != simJoints.end(); iter++) {
        iter->second->getCoreExchange(&obj);
        jointList->push_back(obj);
      }
    }

    void JointManager::getJointExchange(unsigned long id,
                                        core_objects_exchange* obj) {
      MutexLocker locker(&iMutex);
      std::map<unsigned long, SimJoint*>::iterator iter = simJoints.find(id);
      if (iter != simJoints.end())
        iter->second->getCoreExchange(obj);
      else
        obj = NULL;
    }


    const JointData JointManager::getFullJoint(unsigned long index) {
      MutexLocker locker(&iMutex);
      map<unsigned long, SimJoint*>::iterator iter = simJoints.find(index);
      if (iter != simJoints.end())
        return iter->second->getSJoint();
      else {
        char msg[128];
        sprintf(msg, "could not find joint with index: %lu", index);
        throw std::runtime_error(msg);
      }
    }

    void JointManager::removeJoint(unsigned long index) {
      SimJoint* tmpJoint = 0;
      MutexLocker locker(&iMutex);
      map<unsigned long, SimJoint*>::iterator iter = simJoints.find(index);

      if (iter != simJoints.end()) {
        tmpJoint = iter->second;
        simJoints.erase(iter);
      }

      control->motors->removeJointFromMotors(index);

      if (tmpJoint)
        delete tmpJoint;
      control->sim->sceneHasChanged(false);
    }

    void JointManager::removeJointByIDs(unsigned long id1, unsigned long id2) {
      map<unsigned long, SimJoint*>::iterator iter;
      iMutex.lock();

      for (iter = simJoints.begin(); iter != simJoints.end(); iter++)
        if((iter->second->getNodeId() == id1 &&
            iter->second->getNodeId(2) == id2) ||
           (iter->second->getNodeId() == id2 &&
            iter->second->getNodeId(2) == id1)) {
          iMutex.unlock();
          removeJoint(iter->first);
          return;
        }
      iMutex.unlock();
    }

    SimJoint* JointManager::getSimJoint(unsigned long id){
      MutexLocker locker(&iMutex);
      map<unsigned long, SimJoint*>::iterator iter = simJoints.find(id);
      if (iter != simJoints.end())
        return iter->second;
      else
        return NULL;
    }


    std::vector<SimJoint*> JointManager::getSimJoints(void) {
      vector<SimJoint*> v_simJoints;
      map<unsigned long, SimJoint*>::iterator iter;
      MutexLocker locker(&iMutex);
      for (iter = simJoints.begin(); iter != simJoints.end(); iter++)
        v_simJoints.push_back(iter->second);
      return v_simJoints;
    }


    void JointManager::reattacheJoints(unsigned long node_id) {
      map<unsigned long, SimJoint*>::iterator iter;
      MutexLocker locker(&iMutex);
      for (iter = simJoints.begin(); iter != simJoints.end(); iter++) {
        if (iter->second->getSJoint().nodeIndex1 == node_id ||
            iter->second->getSJoint().nodeIndex2 == node_id) {
          iter->second->reattachJoint();
        }
      }
    }

    void JointManager::reloadJoints(void) {
      list<JointData>::iterator iter;
      //MutexLocker locker(&iMutex);
      for(iter = simJointsReload.begin(); iter != simJointsReload.end(); iter++)
        addJoint(&(*iter), true);
    }

    void JointManager::updateJoints(sReal calc_ms) {
      MutexLocker locker(&iMutex);
      map<unsigned long, SimJoint*>::iterator iter;
      for(iter = simJoints.begin(); iter != simJoints.end(); iter++) {
        iter->second->update(calc_ms);
      }
    }

    void JointManager::clearAllJoints(bool clear_all) {
      map<unsigned long, SimJoint*>::iterator iter;
      MutexLocker locker(&iMutex);
      if(clear_all) simJointsReload.clear();

      while(!simJoints.empty()) {
        control->motors->removeJointFromMotors(simJoints.begin()->first);
        delete simJoints.begin()->second;
        simJoints.erase(simJoints.begin());
      }
      control->sim->sceneHasChanged(false);

      next_joint_id = 1;
    }

    std::list<JointData>::iterator JointManager::getReloadJoint(unsigned long id) {
      std::list<JointData>::iterator iter = simJointsReload.begin();
      for(;iter!=simJointsReload.end(); ++iter) {
        if(iter->index == id) break;
      }
      return iter;
    }

    void JointManager::setReloadJointOffset(unsigned long id, sReal offset) {
      MutexLocker locker(&iMutex);
      list<JointData>::iterator iter = getReloadJoint(id);
      if (iter != simJointsReload.end())
        iter->angle1_offset = offset;
    }

    void JointManager::setReloadJointAxis(unsigned long id, const Vector &axis) {
      MutexLocker locker(&iMutex);
      list<JointData>::iterator iter = getReloadJoint(id);
      if (iter != simJointsReload.end())
        iter->axis1 = axis;
    }


    void JointManager::scaleReloadJoints(sReal x_factor, sReal y_factor, sReal z_factor)
    {
      list<JointData>::iterator iter;
      MutexLocker locker(&iMutex);
      for(iter = simJointsReload.begin(); iter != simJointsReload.end(); iter++) {
        iter->anchor.x() *= x_factor;
        iter->anchor.y() *= y_factor;
        iter->anchor.z() *= z_factor;
      }
    }


    void JointManager::setJointTorque(unsigned long id, sReal torque) {
      MutexLocker locker(&iMutex);
      map<unsigned long, SimJoint*>::iterator iter = simJoints.find(id);
      if (iter != simJoints.end())
        iter->second->setEffort(torque, 0);
    }


    void JointManager::changeStepSize(void) {
      map<unsigned long, SimJoint*>::iterator iter;
      MutexLocker locker(&iMutex);
      for (iter = simJoints.begin(); iter != simJoints.end(); iter++) {
        iter->second->updateStepSize();
      }
    }

    void JointManager::setReloadAnchor(unsigned long id, const Vector &anchor) {
      MutexLocker locker(&iMutex);
      list<JointData>::iterator iter = getReloadJoint(id);
      if (iter != simJointsReload.end())
        iter->anchor = anchor;
    }


    void JointManager::setSDParams(unsigned long id, JointData *sJoint) {
      MutexLocker locker(&iMutex);
      map<unsigned long, SimJoint*>::iterator iter = simJoints.find(id);
      if (iter != simJoints.end())
        iter->second->setSDParams(sJoint);
    }


    void JointManager::setVelocity(unsigned long id, sReal velocity) {
      MutexLocker locker(&iMutex);
      map<unsigned long, SimJoint*>::iterator iter = simJoints.find(id);
      if (iter != simJoints.end())
        iter->second->setVelocity(velocity);
    }


    void JointManager::setVelocity2(unsigned long id, sReal velocity) {
      MutexLocker locker(&iMutex);
      map<unsigned long, SimJoint*>::iterator iter = simJoints.find(id);
      if (iter != simJoints.end())
        iter->second->setVelocity(velocity, 2);
    }


    void JointManager::setForceLimit(unsigned long id, sReal max_force,
                                     bool first_axis) {
      MutexLocker locker(&iMutex);
      map<unsigned long, SimJoint*>::iterator iter = simJoints.find(id);
      if (iter != simJoints.end()) {
        if (first_axis)
          iter->second->setEffortLimit(max_force);
        else
          iter->second->setEffortLimit(max_force, 2);
      }
    }


    unsigned long JointManager::getID(const std::string& joint_name) const {
      map<unsigned long, SimJoint*>::const_iterator iter;
      MutexLocker locker(&iMutex);
      for(iter = simJoints.begin(); iter != simJoints.end(); iter++) {
        JointData joint = iter->second->getSJoint();
        if (joint.name == joint_name)
          return joint.index;
      }
      return 0;
    }

    bool JointManager::getDataBrokerNames(unsigned long id, std::string *groupName,
                                          std::string *dataName) const {
      map<unsigned long, SimJoint*>::const_iterator iter;
      iter = simJoints.find(id);
      if(iter == simJoints.end())
        return false;
      iter->second->getDataBrokerNames(groupName, dataName);
      return true;
    }

    void JointManager::setOfflineValue(unsigned long id, sReal value) {
      map<unsigned long, SimJoint*>::const_iterator iter;
      iter = simJoints.find(id);
      if(iter == simJoints.end())
        return;
      iter->second->setOfflinePosition(value);
    }

    sReal JointManager::getLowStop(unsigned long id) const {
      map<unsigned long, SimJoint*>::const_iterator iter;
      iter = simJoints.find(id);
      if(iter == simJoints.end())
        return 0.;
      return iter->second->getLowerLimit();
    }
    sReal JointManager::getHighStop(unsigned long id) const {
      map<unsigned long, SimJoint*>::const_iterator iter;
      iter = simJoints.find(id);
      if(iter == simJoints.end())
        return 0.;
      return iter->second->getUpperLimit();
    }
    sReal JointManager::getLowStop2(unsigned long id) const {
      map<unsigned long, SimJoint*>::const_iterator iter;
      iter = simJoints.find(id);
      if(iter == simJoints.end())
        return 0.;
      return iter->second->getLowerLimit(2);
    }
    sReal JointManager::getHighStop2(unsigned long id) const {
      map<unsigned long, SimJoint*>::const_iterator iter;
      iter = simJoints.find(id);
      if(iter == simJoints.end())
        return 0.;
      return iter->second->getUpperLimit(2);
    }
    void JointManager::setLowStop(unsigned long id, sReal lowStop) {
      map<unsigned long, SimJoint*>::const_iterator iter;
      iter = simJoints.find(id);
      if(iter == simJoints.end())
        return;
      return iter->second->setLowerLimit(lowStop);
    }
    void JointManager::setHighStop(unsigned long id, sReal highStop) {
      map<unsigned long, SimJoint*>::const_iterator iter;
      iter = simJoints.find(id);
      if(iter == simJoints.end())
        return;
      return iter->second->setUpperLimit(highStop);
    }
    void JointManager::setLowStop2(unsigned long id, sReal lowStop2) {
      map<unsigned long, SimJoint*>::const_iterator iter;
      iter = simJoints.find(id);
      if(iter == simJoints.end())
        return;
      return iter->second->setLowerLimit(lowStop2, 2);
    }
    void JointManager::setHighStop2(unsigned long id, sReal highStop2) {
      map<unsigned long, SimJoint*>::const_iterator iter;
      iter = simJoints.find(id);
      if(iter == simJoints.end())
        return;
      return iter->second->setUpperLimit(highStop2, 2);
    }

  } // end of namespace sim
} // end of namespace mars
