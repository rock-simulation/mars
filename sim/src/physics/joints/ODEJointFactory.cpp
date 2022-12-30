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

#include "joints/ODEJointFactory.h"
#include "joints/ODEHingeJoint.h"
#include "joints/ODEHinge2Joint.h"
#include "joints/ODESliderJoint.h"
#include "joints/ODEBallJoint.h"
#include "joints/ODEUniversalJoint.h"
#include "joints/ODEFixedJoint.h"

#include <memory>

namespace mars{
namespace sim{

using namespace ::mars::interfaces;

ODEJointFactory& ODEJointFactory::Instance(){
  static ODEJointFactory instance;
  return instance;
}

ODEJointFactory::ODEJointFactory(){
}

ODEJointFactory::~ODEJointFactory(){
}

std::shared_ptr<JointInterface> ODEJointFactory::createJoint(std::shared_ptr<interfaces::PhysicsInterface> worldPhysics,
                               interfaces::JointData *jointData,
                               const std::shared_ptr<interfaces::NodeInterface> node1,
                               const std::shared_ptr<interfaces::NodeInterface> node2){

  std::shared_ptr<ODEJoint> newJoint;

  switch(jointData->type) {
  case JOINT_TYPE_HINGE:
    newJoint = std::make_shared<ODEHingeJoint>(worldPhysics, jointData, node1, node2);
    break;
  case JOINT_TYPE_HINGE2:
    newJoint = std::make_shared<ODEHinge2Joint>(worldPhysics, jointData, node1, node2);
    break;
  case JOINT_TYPE_SLIDER:
    newJoint = std::make_shared<ODESliderJoint>(worldPhysics, jointData, node1, node2);
    break;
  case JOINT_TYPE_BALL:
    newJoint = std::make_shared<ODEBallJoint>(worldPhysics, jointData, node1, node2);
    break;
  case JOINT_TYPE_UNIVERSAL:
    newJoint = std::make_shared<ODEUniversalJoint>(worldPhysics, jointData, node1, node2);
    break;
  case JOINT_TYPE_FIXED:
    newJoint = std::make_shared<ODEFixedJoint>(worldPhysics, jointData, node1, node2);
    break;
  default:
    // no correct type is spezified, so no physically node will be created
    std::cout << "Unknown type of ODEJoint requested. No physical joint was created." << std::endl;
    return std::shared_ptr<JointInterface>(nullptr);
    break;
  }

  if(newJoint->isJointCreated()){
    return newJoint;
  }
  else{
    return std::shared_ptr<JointInterface>(nullptr);
  }

}
}
}
