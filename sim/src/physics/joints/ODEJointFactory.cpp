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
#include <mars/interfaces/utils.h>

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
  std::map<const std::string, instantiateJointfPtr>::iterator it = availableJoints.find(std::string(getJointTypeString(jointData->type)));
  if(it == availableJoints.end()){
    throw std::runtime_error("Could not load unknown Physics Joint with name: \"" + jointData->name + "\"" );
  }

  std::shared_ptr<ODEJoint> newJoint = std::make_shared<ODEJoint>(*(it->second(worldPhysics, jointData, node1, node2)));
  if(newJoint->isJointCreated()){
    return newJoint;
  }
  else{
    std::cerr << "Failed to create Physics Joint with name: \"" + jointData->name + "\"" << std::endl;
    return std::shared_ptr<JointInterface>(nullptr);
  }

}

void ODEJointFactory::addJointType(const std::string& type, instantiateJointfPtr funcPtr){

  availableJoints.insert(std::pair<const std::string, instantiateJointfPtr>(type, funcPtr));
}

}
}
