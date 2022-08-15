/*
 *  Copyright 2022, DFKI GmbH Robotics Innovation Center
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
 * \file CoreODEJoints.h
 * \author Malte Roemmermann, Muhammad Haider Khan Lodhi
 * \brief A collection of OBE joint classes
 *
 */

#ifndef CORE_ODE_OBJECTS_H
#define CORE_ODE_OBJECTS_H

#ifdef _PRINT_HEADER_
  #warning "CoreODEJoints.h"
#endif

#include <mars/utils/MutexLocker.h>
#include "ODEJoint.h"
#include <string>

//TODO remove?
#ifndef ODE11
  #define dTriIndex int
#endif

namespace mars {
namespace sim {

    class ODEHingeJoint : public ODEJoint {
    public:
      ODEHingeJoint(std::shared_ptr<interfaces::PhysicsInterface> world, 
                               interfaces::JointData *joint,
                               const std::shared_ptr<interfaces::NodeInterface> node1, 
                               const std::shared_ptr<interfaces::NodeInterface> node2);
      virtual ~ODEHingeJoint(void);
      virtual bool createODEJoint(JointData *jointS, dBodyID body1, dBodyID body2) override; 
    };

    class ODEHinge2Joint : public ODEJoint {
    public:
      ODEHinge2Joint(std::shared_ptr<interfaces::PhysicsInterface> world, 
                               interfaces::JointData *joint,
                               const std::shared_ptr<interfaces::NodeInterface> node1, 
                               const std::shared_ptr<interfaces::NodeInterface> node2);
      virtual ~ODEHinge2Joint(void);
      virtual bool createODEJoint(JointData *jointS, dBodyID body1, dBodyID body2) override;
    };

    class ODESliderJoint : public ODEJoint {
    public:
      ODESliderJoint(std::shared_ptr<interfaces::PhysicsInterface> world, 
                               interfaces::JointData *joint,
                               const std::shared_ptr<interfaces::NodeInterface> node1, 
                               const std::shared_ptr<interfaces::NodeInterface> node2);
      virtual ~ODESliderJoint(void);
      virtual bool createODEJoint(JointData *jointS, dBodyID body1, dBodyID body2) override;
    };

    class ODEBallJoint : public ODEJoint {
    public:
      ODEBallJoint(std::shared_ptr<interfaces::PhysicsInterface> world, 
                               interfaces::JointData *joint,
                               const std::shared_ptr<interfaces::NodeInterface> node1, 
                               const std::shared_ptr<interfaces::NodeInterface> node2);
      virtual ~ODEBallJoint(void);
      virtual bool createODEJoint(JointData *jointS, dBodyID body1, dBodyID body2) override;
    };

    class ODEUniversalJoint : public ODEJoint {
    public:
      ODEUniversalJoint(std::shared_ptr<interfaces::PhysicsInterface> world, 
                               interfaces::JointData *joint,
                               const std::shared_ptr<interfaces::NodeInterface> node1, 
                               const std::shared_ptr<interfaces::NodeInterface> node2);
      virtual ~ODEUniversalJoint(void);
      virtual bool createODEJoint(interfaces::NodeData *node) override;
    };

    class ODEFixedJoint : public ODEJoint {
    public:
      ODEFixedJoint(std::shared_ptr<interfaces::PhysicsInterface> world, 
                               interfaces::JointData *joint,
                               const std::shared_ptr<interfaces::NodeInterface> node1, 
                               const std::shared_ptr<interfaces::NodeInterface> node2);
      virtual ~ODEFixedJoint(void);
      virtual bool createODEJoint(JointData *jointS, dBodyID body1, dBodyID body2) override;
    };

} // end of namespace sim
} // end of namespace mars

#endif  // CORE_ODE_JOINTS_H
