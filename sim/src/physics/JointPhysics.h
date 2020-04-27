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
 
#ifndef JOINT_PHYSICS_H
#define JOINT_PHYSICS_H

#ifdef _PRINT_HEADER_
  #warning "JointPhysics.h"
#endif

#include "WorldPhysics.h"

#include <mars/interfaces/sim/JointInterface.h>
#include <mars/interfaces/sim/NodeInterface.h>

namespace mars {
  namespace sim {

    class JointPhysics:	public interfaces::JointInterface {
    public:
      ///the constructor 
      JointPhysics(std::shared_ptr<interfaces::PhysicsInterface> world);
      ///the destructor
      virtual ~JointPhysics(void);
      ///create Joint getting as argument the JointData which give the 
      ///joint informations.
      virtual bool createJoint(interfaces::JointData *joint,
                               const std::shared_ptr<interfaces::NodeInterface> node1, 
                               const std::shared_ptr<interfaces::NodeInterface> node2);
      ///get the anchor of the joint
      virtual void getAnchor(utils::Vector* anchor) const;
      /// set the anchor i.e. the position where the joint is created of the joint 
      virtual void setAnchor(const utils::Vector &anchor);
      ///set the world informations
      virtual void setAxis(const utils::Vector &axis);
      virtual void setAxis2(const utils::Vector &axis);
      virtual void getAxis(utils::Vector* axis) const;
      virtual void getAxis2(utils::Vector* axis) const;
      virtual void setWorldObject(std::shared_ptr<interfaces::PhysicsInterface>  world);
      // methods to controll a joint through a motor
      virtual void setForceLimit(interfaces::sReal max_force);
      virtual void setForceLimit2(interfaces::sReal max_force);
      virtual void setVelocity(interfaces::sReal velocity);
      virtual void setVelocity2(interfaces::sReal velocity);
      virtual interfaces::sReal getVelocity(void) const;
      virtual interfaces::sReal getVelocity2(void) const;
      virtual void setJointAsMotor(int axis);
      virtual void unsetJointAsMotor(int axis);
      virtual interfaces::sReal getPosition(void) const;
      virtual interfaces::sReal getPosition2(void) const;
      virtual void getForce1(utils::Vector *f) const;
      virtual void getForce2(utils::Vector *f) const;
      virtual void getTorque1(utils::Vector *t) const;
      virtual void getTorque2(utils::Vector *t) const;
      virtual void setTorque(interfaces::sReal torque);
      virtual void setTorque2(interfaces::sReal torque);
      virtual void reattacheJoint(void);
      virtual void update(void);
      virtual void getJointLoad(utils::Vector *t) const;
      virtual void getAxisTorque(utils::Vector *t) const;
      virtual void getAxis2Torque(utils::Vector *t) const;
      virtual void changeStepSize(const interfaces::JointData &jointS);
      virtual interfaces::sReal getMotorTorque(void) const;
      virtual interfaces::sReal getLowStop() const;
      virtual interfaces::sReal getHighStop() const;
      virtual interfaces::sReal getLowStop2() const;
      virtual interfaces::sReal getHighStop2() const;
      virtual void setLowStop(interfaces::sReal lowStop);
      virtual void setHighStop(interfaces::sReal highStop);
      virtual void setLowStop2(interfaces::sReal lowStop2);
      virtual void setHighStop2(interfaces::sReal highStop2);

    private:
      std::shared_ptr<WorldPhysics> theWorld;
      dJointID jointId, ball_motor;
      dJointFeedback feedback;
      dBodyID body1, body2;
      int joint_type;
      dReal cfm, cfm1, cfm2, erp1, erp2;
      dReal lo1, lo2, hi1, hi2;
      dReal damping, spring, jointCFM;
      utils::Vector axis1_torque, axis2_torque, joint_load;
      dReal motor_torque;

      void calculateCfmErp(const interfaces::JointData *jointS);

      ///create a joint from type Hing
      void createHinge(interfaces::JointData* jointS,
                       dBodyID body1, dBodyID body2);
      void createHinge2(interfaces::JointData* jointS,
                        dBodyID body1, dBodyID body2);
      void createSlider(interfaces::JointData* jointS,
                        dBodyID body1, dBodyID body2);
      void createBall(interfaces::JointData* jointS,
                      dBodyID body1, dBodyID body2);
      void createUniversal(interfaces::JointData* jointS,
                           dBodyID body1, dBodyID body2);
      void createFixed(interfaces::JointData* jointS,
                       dBodyID body1, dBodyID body2);
    };

  } // end of namespace sim
} // end of namespace mars

#endif     
