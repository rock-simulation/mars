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

#include <ODEObjectFactory.h>
#include <ODEBox.h>
#include <ODECapsule.h>
#include <ODECylinder.h>
#include <ODEMesh.h>
#include <ODEPlane.h>
#include <ODESphere.h>
#include <ODEHeightField.h>

#include <memory>

namespace mars{
namespace sim{

using namespace ::mars::interfaces;

ODEObjectFactory* ODEObjectFactory::instance = NULL;

ODEObjectFactory* ODEObjectFactory::getInstance(){
  if (NULL == instance){
    instance = new ODEObjectFactory();
  }
  return instance;
} 

ODEObjectFactory::ODEObjectFactory(){
}

ODEObjectFactory::~ODEObjectFactory(){
}

std::shared_ptr<NodeInterface> ODEObjectFactory::createObject(std::shared_ptr<PhysicsInterface> worldPhysics, NodeData * nodeData){
  switch(nodeData->physicMode) {
  case NODE_TYPE_BOX:
    return std::static_pointer_cast<NodeInterface>(std::make_shared<ODEBox>(worldPhysics, nodeData));
    break;
  case NODE_TYPE_CAPSULE:
    return std::static_pointer_cast<NodeInterface>(std::make_shared<ODECapsule>(worldPhysics, nodeData));
    break; 
  case NODE_TYPE_CYLINDER:
    return std::static_pointer_cast<NodeInterface>(std::make_shared<ODECylinder>(worldPhysics, nodeData));
    break; 
  case NODE_TYPE_MESH:
    return std::static_pointer_cast<NodeInterface>(std::make_shared<ODEMesh>(worldPhysics, nodeData));
    break;  
  case NODE_TYPE_PLANE:
    return std::static_pointer_cast<NodeInterface>(std::make_shared<ODEPlane>(worldPhysics, nodeData));
    break; 
  case NODE_TYPE_SPHERE:
    return std::static_pointer_cast<NodeInterface>(std::make_shared<ODESphere>(worldPhysics, nodeData));
    break;    
  case NODE_TYPE_TERRAIN:
    return std::static_pointer_cast<NodeInterface>(std::make_shared<ODEHeightField>(worldPhysics, nodeData));
    break; 
  default:
    // no correct type is spezified, so no physically node will be created
    std::cout << "Unknown type of object requested " << std::endl;
    return std::shared_ptr<NodeInterface>(nullptr);
    break;
  }
}
}
}
