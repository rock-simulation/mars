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
 * \file JointManager.h
 * \author Malte Roemmermann
 * \brief "JointManager" is the class that manage all joints and their
 * operations and communication between the different modules of the simulation.
 *
 */

#ifndef JOINT_MANAGER_H
#define JOINT_MANAGER_H

#ifdef _PRINT_HEADER_
  #warning "JointManager.h"
#endif

#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/interfaces/sim/JointManagerInterface.h>
#include <mars/utils/Mutex.h>

namespace mars {
  namespace sim {

    class SimJoint;

    /**
     * The declaration of the JointManager class.
     */
    class JointManager : public interfaces::JointManagerInterface {
    public:
      JointManager(interfaces::ControlCenter *c);
      virtual ~JointManager(){}
      virtual unsigned long addJoint(interfaces::JointData *jointS, bool reload = false);
      virtual int getJointCount();
      virtual void editJoint(interfaces::JointData *jointS);
      virtual void getListJoints(std::vector<interfaces::core_objects_exchange> *jointList);
      virtual void getJointExchange(unsigned long id, interfaces::core_objects_exchange *obj);
      virtual const interfaces::JointData getFullJoint(unsigned long index);
      virtual void removeJoint(unsigned long index);
      virtual void removeJointByIDs(unsigned long id1, unsigned long id2);
      virtual std::shared_ptr<mars::sim::SimJoint> getSimJoint(unsigned long id);
      virtual std::vector<std::shared_ptr<mars::sim::SimJoint>> getSimJoints(void);
      virtual void reattacheJoints(unsigned long node_id);
      virtual void reloadJoints(void);
      virtual void updateJoints(interfaces::sReal calc_ms);
      virtual void clearAllJoints(bool clear_all=false);
      virtual void setReloadJointOffset(unsigned long id, interfaces::sReal offset);
      virtual void setReloadJointAxis(unsigned long id, const utils::Vector &axis);
      virtual void scaleReloadJoints(interfaces::sReal x, interfaces::sReal y, interfaces::sReal z);
      virtual void setJointTorque(unsigned long id, interfaces::sReal torque);
      virtual void changeStepSize(void);
      virtual void setReloadAnchor(unsigned long id, const utils::Vector &anchor);
      virtual void setSDParams(unsigned long id, interfaces::JointData *sJoint);

      virtual void setVelocity(unsigned long id, interfaces::sReal velocity);
      virtual void setVelocity2(unsigned long id, interfaces::sReal velocity);
      virtual void setForceLimit(unsigned long id, interfaces::sReal max_force,
                                 bool first_axis = 1);

      virtual unsigned long getID(const std::string &joint_name) const;
      virtual unsigned long getIDByNodeIDs(unsigned long id1, unsigned long id2);
      virtual bool getDataBrokerNames(unsigned long id, std::string *groupName,
                                      std::string *dataName) const;
      virtual void setOfflineValue(unsigned long id, interfaces::sReal value);

      virtual interfaces::sReal getLowStop(unsigned long id) const;
      virtual interfaces::sReal getHighStop(unsigned long id) const;
      virtual interfaces::sReal getLowStop2(unsigned long id) const;
      virtual interfaces::sReal getHighStop2(unsigned long id) const;
      virtual void setLowStop(unsigned long id, interfaces::sReal lowStop);
      virtual void setHighStop(unsigned long id, interfaces::sReal highStop);
      virtual void setLowStop2(unsigned long id, interfaces::sReal lowStop2);
      virtual void setHighStop2(unsigned long id, interfaces::sReal highStop2);
      virtual void edit(interfaces::JointId id, const std::string &key,
                        const std::string &value);

    private:
      unsigned long next_joint_id;
      std::map<unsigned long, std::shared_ptr<SimJoint>> simJoints;
      std::list<interfaces::JointData> simJointsReload;
      interfaces::ControlCenter *control;
      mutable utils::Mutex iMutex;
      interfaces::JointManagerInterface* getJointInterface(unsigned long node_id);
      std::list<interfaces::JointData>::iterator getReloadJoint(unsigned long id);

    };

  } // end of namespace sim
} // end of namespace mars

#endif  // JOINT_MANAGER_H
