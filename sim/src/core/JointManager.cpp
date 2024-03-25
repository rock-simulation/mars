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
#include "joints/ODEJointFactory.h"
#include "joints/ODEBallJoint.h"
#include "joints/ODEFixedJoint.h"
#include "joints/ODEHinge2Joint.h"
#include "joints/ODEHingeJoint.h"
#include "joints/ODESliderJoint.h"
#include "joints/ODEUniversalJoint.h"

#include <stdexcept>

#include <mars/interfaces/sim/SimulatorInterface.h>
#include <mars/interfaces/sim/MotorManagerInterface.h>
#include <mars/interfaces/utils.h>
#include <mars/utils/misc.h>
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

      ODEJointFactory::Instance().addJointType("ball",      &ODEBallJoint::instanciate);
      ODEJointFactory::Instance().addJointType("fixed",     &ODEFixedJoint::instanciate);
      ODEJointFactory::Instance().addJointType("hinge2",    &ODEHinge2Joint::instanciate);
      ODEJointFactory::Instance().addJointType("hinge",     &ODEHingeJoint::instanciate);
      ODEJointFactory::Instance().addJointType("slider",    &ODESliderJoint::instanciate);
      ODEJointFactory::Instance().addJointType("universal", &ODEUniversalJoint::instanciate);
    }

    unsigned long JointManager::addJoint(JointData *jointS, bool reload) {
      std::shared_ptr<interfaces::JointInterface> newJointInterface(nullptr);
      std::vector<std::shared_ptr<SimNode>>::iterator iter;
      std::shared_ptr<SimNode> node1 = 0;
      std::shared_ptr<SimNode> node2 = 0;
      std::shared_ptr<NodeInterface> i_node1 = 0;
      std::shared_ptr<NodeInterface> i_node2 = 0;
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

      // create an interface object to the physics
      newJointInterface = ODEJointFactory::Instance().createJoint(control->sim->getPhysics(), jointS, i_node1, i_node2);

      // create the physical node data
      if (newJointInterface.get() == nullptr) {
        LOG_ERROR("JointManager::addJoint: No joint was created in physics.");
        // if no node was created in physics
        // delete the objects
        newJointInterface.reset();
        // and return false
        return 0;
      }
      else {
        // put all data to the correct place
        iMutex.lock();
        // set the next free id
        jointS->index = next_joint_id;
        next_joint_id++;
        if (jointS->config.hasKey("desired_id"))
        {
          unsigned long des_id=jointS->config["desired_id"];
          bool found = false;
          for (auto j : simJoints)
          {
            if (j.first == des_id) {
              found = true;
            }
          }
          if (!found) {
            jointS->index = des_id;
            next_joint_id--;
            if (des_id>= next_joint_id)
              next_joint_id = des_id + 1;
          }
        }
	      std::shared_ptr<SimJoint> newJoint = std::make_shared<SimJoint>(control, *jointS);
        newJoint->setAttachedNodes(node1, node2);
        //    newJoint->setSJoint(*jointS);
        newJoint->setPhysicalJoint(newJointInterface);
        simJoints[jointS->index] = newJoint;
        iMutex.unlock();
        control->sim->sceneHasChanged(false);
        return jointS->index;
      }
    }

    int JointManager::getJointCount() {
      MutexLocker locker(&iMutex);
      return simJoints.size();
    }

    void JointManager::editJoint(JointData *jointS) {
      MutexLocker locker(&iMutex);
      std::map<unsigned long, std::shared_ptr<SimJoint>>::iterator iter = simJoints.find(jointS->index);
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
      std::map<unsigned long, std::shared_ptr<SimJoint>>::iterator iter;
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
      std::map<unsigned long, std::shared_ptr<SimJoint>>::iterator iter = simJoints.find(id);
      if (iter != simJoints.end())
        iter->second->getCoreExchange(obj);
      else
        obj = NULL;
    }


    const JointData JointManager::getFullJoint(unsigned long index) {
      MutexLocker locker(&iMutex);
      map<unsigned long, std::shared_ptr<SimJoint>>::iterator iter = simJoints.find(index);
      if (iter != simJoints.end())
        return iter->second->getSJoint();
      else {
        char msg[128];
        sprintf(msg, "could not find joint with index: %lu", index);
        throw std::runtime_error(msg);
      }
    }

    void JointManager::removeJoint(unsigned long index) {
      std::shared_ptr<SimJoint> tmpJoint = 0;
      MutexLocker locker(&iMutex);
      map<unsigned long, std::shared_ptr<SimJoint>>::iterator iter = simJoints.find(index);

      if (iter != simJoints.end()) {
        tmpJoint = iter->second;
        simJoints.erase(iter);
      }

      control->motors->removeJointFromMotors(index);

      if (tmpJoint)
        tmpJoint.reset();
      control->sim->sceneHasChanged(false);
    }

    void JointManager::removeJointByIDs(unsigned long id1, unsigned long id2) {
      unsigned long id = getIDByNodeIDs(id1, id2);
      if (id != 0) {
          removeJoint(id);
          return;
      }
    }

    std::shared_ptr<mars::sim::SimJoint> JointManager::getSimJoint(unsigned long id){
      MutexLocker locker(&iMutex);
      map<unsigned long, std::shared_ptr<mars::sim::SimJoint>>::iterator iter = simJoints.find(id);
      if (iter != simJoints.end())
        return iter->second;
      else
        return NULL;
    }


    std::vector<std::shared_ptr<mars::sim::SimJoint>> JointManager::getSimJoints(void) {
      vector<std::shared_ptr<mars::sim::SimJoint>> v_simJoints;
      map<unsigned long, std::shared_ptr<mars::sim::SimJoint>>::iterator iter;
      MutexLocker locker(&iMutex);
      for (iter = simJoints.begin(); iter != simJoints.end(); iter++)
        v_simJoints.push_back(iter->second);
      return v_simJoints;
    }


    void JointManager::reattacheJoints(unsigned long node_id) {
      map<unsigned long, std::shared_ptr<SimJoint>>::iterator iter;
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
      map<unsigned long, std::shared_ptr<SimJoint>>::iterator iter;
      for(iter = simJoints.begin(); iter != simJoints.end(); iter++) {
        iter->second->update(calc_ms);
      }
    }

    void JointManager::clearAllJoints(bool clear_all) {
      map<unsigned long, std::shared_ptr<SimJoint>>::iterator iter;
      MutexLocker locker(&iMutex);
      if(clear_all) simJointsReload.clear();

      while(!simJoints.empty()) {
        control->motors->removeJointFromMotors(simJoints.begin()->first);
        simJoints.begin()->second.reset();
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
      map<unsigned long, std::shared_ptr<SimJoint>>::iterator iter = simJoints.find(id);
      if (iter != simJoints.end())
        iter->second->setEffort(torque, 0);
    }


    void JointManager::changeStepSize(void) {
      map<unsigned long, std::shared_ptr<SimJoint>>::iterator iter;
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
      map<unsigned long, std::shared_ptr<SimJoint>>::iterator iter = simJoints.find(id);
      if (iter != simJoints.end())
        iter->second->setSDParams(sJoint);
    }


    void JointManager::setVelocity(unsigned long id, sReal velocity) {
      MutexLocker locker(&iMutex);
      map<unsigned long, std::shared_ptr<SimJoint>>::iterator iter = simJoints.find(id);
      if (iter != simJoints.end())
        iter->second->setVelocity(velocity);
    }


    void JointManager::setVelocity2(unsigned long id, sReal velocity) {
      MutexLocker locker(&iMutex);
      map<unsigned long, std::shared_ptr<SimJoint>>::iterator iter = simJoints.find(id);
      if (iter != simJoints.end())
        iter->second->setVelocity(velocity, 2);
    }


    void JointManager::setForceLimit(unsigned long id, sReal max_force,
                                     bool first_axis) {
      MutexLocker locker(&iMutex);
      map<unsigned long, std::shared_ptr<SimJoint>>::iterator iter = simJoints.find(id);
      if (iter != simJoints.end()) {
        if (first_axis)
          iter->second->setEffortLimit(max_force);
        else
          iter->second->setEffortLimit(max_force, 2);
      }
    }


    unsigned long JointManager::getID(const std::string& joint_name) const {
      map<unsigned long, std::shared_ptr<SimJoint>>::const_iterator iter;
      MutexLocker locker(&iMutex);
      for(iter = simJoints.begin(); iter != simJoints.end(); iter++) {
        JointData joint = iter->second->getSJoint();
        if (joint.name == joint_name)
          return joint.index;
      }
      return 0;
    }

    std::vector<unsigned long> JointManager::getIDsByNodeID(unsigned long node_id) {
      map<unsigned long, std::shared_ptr<SimJoint>>::iterator iter;
      MutexLocker locker(&iMutex);
      std::vector<unsigned long> out;
      for (iter = simJoints.begin(); iter != simJoints.end(); iter++)
        if((iter->second->getNodeId() == node_id ||
            iter->second->getNodeId(2) == node_id) ||
           (iter->second->getNodeId() == node_id ||
            iter->second->getNodeId(2) == node_id)) {
          out.push_back(iter->first);
        }
      return out;
    }

    unsigned long JointManager::getIDByNodeIDs(unsigned long id1, unsigned long id2) {
      map<unsigned long, std::shared_ptr<SimJoint>>::iterator iter;
      MutexLocker locker(&iMutex);

      for (iter = simJoints.begin(); iter != simJoints.end(); iter++)
        if((iter->second->getNodeId() == id1 &&
            iter->second->getNodeId(2) == id2) ||
           (iter->second->getNodeId() == id2 &&
            iter->second->getNodeId(2) == id1)) {
          return iter->first;
        }
      return 0;
    }

    bool JointManager::getDataBrokerNames(unsigned long id, std::string *groupName,
                                          std::string *dataName) const {
      map<unsigned long, std::shared_ptr<SimJoint>>::const_iterator iter;
      iter = simJoints.find(id);
      if(iter == simJoints.end())
        return false;
      iter->second->getDataBrokerNames(groupName, dataName);
      return true;
    }

    void JointManager::setOfflineValue(unsigned long id, sReal value) {
      map<unsigned long, std::shared_ptr<SimJoint>>::const_iterator iter;
      iter = simJoints.find(id);
      if(iter == simJoints.end())
        return;
      iter->second->setOfflinePosition(value);
    }

    sReal JointManager::getLowStop(unsigned long id) const {
      map<unsigned long, std::shared_ptr<SimJoint>>::const_iterator iter;
      iter = simJoints.find(id);
      if(iter == simJoints.end())
        return 0.;
      return iter->second->getLowerLimit();
    }
    sReal JointManager::getHighStop(unsigned long id) const {
      map<unsigned long, std::shared_ptr<SimJoint>>::const_iterator iter;
      iter = simJoints.find(id);
      if(iter == simJoints.end())
        return 0.;
      return iter->second->getUpperLimit();
    }
    sReal JointManager::getLowStop2(unsigned long id) const {
      map<unsigned long, std::shared_ptr<SimJoint>>::const_iterator iter;
      iter = simJoints.find(id);
      if(iter == simJoints.end())
        return 0.;
      return iter->second->getLowerLimit(2);
    }
    sReal JointManager::getHighStop2(unsigned long id) const {
      map<unsigned long, std::shared_ptr<SimJoint>>::const_iterator iter;
      iter = simJoints.find(id);
      if(iter == simJoints.end())
        return 0.;
      return iter->second->getUpperLimit(2);
    }
    void JointManager::setLowStop(unsigned long id, sReal lowStop) {
      map<unsigned long, std::shared_ptr<SimJoint>>::const_iterator iter;
      iter = simJoints.find(id);
      if(iter == simJoints.end())
        return;
      return iter->second->setLowerLimit(lowStop);
    }
    void JointManager::setHighStop(unsigned long id, sReal highStop) {
      map<unsigned long, std::shared_ptr<SimJoint>>::const_iterator iter;
      iter = simJoints.find(id);
      if(iter == simJoints.end())
        return;
      return iter->second->setUpperLimit(highStop);
    }
    void JointManager::setLowStop2(unsigned long id, sReal lowStop2) {
      map<unsigned long, std::shared_ptr<SimJoint>>::const_iterator iter;
      iter = simJoints.find(id);
      if(iter == simJoints.end())
        return;
      return iter->second->setLowerLimit(lowStop2, 2);
    }
    void JointManager::setHighStop2(unsigned long id, sReal highStop2) {
      map<unsigned long, std::shared_ptr<SimJoint>>::const_iterator iter;
      iter = simJoints.find(id);
      if(iter == simJoints.end())
        return;
      return iter->second->setUpperLimit(highStop2, 2);
    }

    // todo: do we need to edit angle offsets
    void JointManager::edit(interfaces::JointId id, const std::string &key,
                            const std::string &value) {
      MutexLocker locker(&iMutex);
      std::map<unsigned long, std::shared_ptr<SimJoint>>::iterator iter = simJoints.find(id);
      if (iter != simJoints.end()) {
        if(matchPattern("*/type", key)) {
        }
        else if(matchPattern("*/axis1/*", key)) {
          double v = atof(value.c_str());
          Vector axis = iter->second->getAxis();
          if(key[key.size()-1] == 'x') axis.x() = v;
          else if(key[key.size()-1] == 'y') axis.y() = v;
          else if(key[key.size()-1] == 'z') axis.z() = v;
          iter->second->setAxis(axis);
        }
        else if(matchPattern("*/lowStopAxis1", key)) {
          iter->second->setLowerLimit(atof(value.c_str()));
        }
        else if(matchPattern("*/highStopAxis1", key)) {
          iter->second->setUpperLimit(atof(value.c_str()));
        }
        else if(matchPattern("*/damping_const_constraint_axis1", key)) {
          JointData jd = iter->second->getSJoint();
          jd.damping_const_constraint_axis1 = atof(value.c_str());
          iter->second->setSDParams(&jd);
        }
        else if(matchPattern("*/spring_const_constraint_axis1", key)) {
          JointData jd = iter->second->getSJoint();
          jd.spring_const_constraint_axis1 = atof(value.c_str());
          iter->second->setSDParams(&jd);
        }
        else if(matchPattern("*/axis2/*", key)) {
          double v = atof(value.c_str());
          Vector axis = iter->second->getAxis(2);
          if(key[key.size()-1] == 'x') axis.x() = v;
          else if(key[key.size()-1] == 'y') axis.y() = v;
          else if(key[key.size()-1] == 'z') axis.z() = v;
          iter->second->setAxis(axis, 2);
        }
        else if(matchPattern("*/lowStopAxis2", key)) {
          iter->second->setLowerLimit(atof(value.c_str()), 2);
        }
        else if(matchPattern("*/highStopAxis2", key)) {
          iter->second->setUpperLimit(atof(value.c_str()), 2);
        }
        else if(matchPattern("*/damping_const_constraint_axis2", key)) {
          JointData jd = iter->second->getSJoint();
          jd.damping_const_constraint_axis2 = atof(value.c_str());
          iter->second->setSDParams(&jd);
        }
        else if(matchPattern("*/spring_const_constraint_axis2", key)) {
          JointData jd = iter->second->getSJoint();
          jd.spring_const_constraint_axis2 = atof(value.c_str());
          iter->second->setSDParams(&jd);
        }
        else if(matchPattern("*/anchorpos", key)) {
          NodeId id1 = iter->second->getNodeId();
          NodeId id2 = iter->second->getNodeId(2);
          if(value == "node1") {
            iter->second->setAnchor(control->nodes->getPosition(id1));
          }
          else if(value == "node2") {
            iter->second->setAnchor(control->nodes->getPosition(id2));
          }
          else if(value == "center") {
            Vector pos1 = control->nodes->getPosition(id1);
            Vector pos2 = control->nodes->getPosition(id2);
            iter->second->setAnchor((pos1 + pos2) / 2.);
          }
        }
        else if(matchPattern("*/anchor/*", key)) {
          double v = atof(value.c_str());
          Vector anchor = iter->second->getAnchor();
          if(key[key.size()-1] == 'x') anchor.x() = v;
          else if(key[key.size()-1] == 'y') anchor.y() = v;
          else if(key[key.size()-1] == 'z') anchor.z() = v;
          iter->second->setAnchor(anchor);

        }
        else if(matchPattern("*/invertAxis", key)) {
          ConfigItem b;
          b = key;
          iter->second->setInvertAxis(b);
        }
      }
    }

} // end of namespace sim
} // end of namespace mars
