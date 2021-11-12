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

#ifndef JOINT_INTERFACE_H
#define JOINT_INTERFACE_H

#ifdef _PRINT_HEADER_
  #warning "JointInterface.h"
#endif

#include "NodeInterface.h"
#include "../JointData.h" // her is declared the jointStruct

namespace mars {
  namespace interfaces {

    class JointInterface {
    public:
      virtual ~JointInterface() {}
      // create the joint using the joint structure given as argument
      virtual bool createJoint(JointData *joint,
                               const std::shared_ptr<NodeInterface> node1, 
                               const std::shared_ptr<NodeInterface> i_node2) = 0; // physic interfaces for the node
      virtual void getAnchor(utils::Vector *anchor) const = 0;
      virtual void setAnchor(const utils::Vector &anchor) = 0;
      virtual void setAxis(const utils::Vector &axis) = 0;
      virtual void setAxis2(const utils::Vector &axis) = 0;
      virtual void getAxis(utils::Vector *axis) const = 0;
      virtual void getAxis2(utils::Vector *axis) const = 0;
      virtual void setWorldObject(std::shared_ptr<PhysicsInterface> world) = 0;
      virtual void setForceLimit(sReal max_force) = 0;
      virtual void setForceLimit2(sReal max_force) = 0;
      virtual void setVelocity(sReal velocity) = 0;
      virtual void setVelocity2(sReal velocity) = 0;
      virtual sReal getVelocity(void) const = 0;
      virtual sReal getVelocity2(void) const = 0;
      virtual void setJointAsMotor(int axis) = 0;
      virtual void unsetJointAsMotor(int axis) = 0;
      virtual sReal getPosition(void) const = 0;
      virtual sReal getPosition2(void) const = 0;
      virtual void getForce1(utils::Vector *f) const = 0;
      virtual void getForce2(utils::Vector *f) const = 0;
      virtual void getTorque1(utils::Vector *t) const = 0;
      virtual void getTorque2(utils::Vector *t) const = 0;
      virtual void setTorque(sReal torque) = 0;
      virtual void setTorque2(sReal torque) = 0;
      virtual void reattacheJoint(void) = 0;
      virtual void getAxisTorque(utils::Vector *t) const = 0;
      virtual void getAxis2Torque(utils::Vector *t) const = 0;
      virtual void update(void) = 0;
      virtual void getJointLoad(utils::Vector *t) const = 0;
      virtual void changeStepSize(const JointData &jointS) = 0;
      virtual sReal getMotorTorque(void) const = 0;
      virtual sReal getLowStop() const = 0;
      virtual sReal getHighStop() const = 0;
      virtual sReal getLowStop2() const = 0;
      virtual sReal getHighStop2() const = 0;
      virtual void setLowStop(sReal lowStop) = 0;
      virtual void setHighStop(sReal lowStop) = 0;
      virtual void setLowStop2(sReal lowStop) = 0;
      virtual void setHighStop2(sReal lowStop) = 0;
    };

  } // end of namespace interfaces
} // end of namespace mars

#endif // JOINT_INTERFACE_H
