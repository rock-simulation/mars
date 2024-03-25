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

#include "objects/ODEObjectFactory.h"
#include "objects/ODEBox.h"
#include "objects/ODECapsule.h"
#include "objects/ODECylinder.h"
#include "objects/ODEMesh.h"
#include "objects/ODEPlane.h"
#include "objects/ODESphere.h"
#include "objects/ODEHeightField.h"

#include <memory>

namespace mars{
namespace sim{

using namespace ::mars::interfaces;

ODEObjectFactory& ODEObjectFactory::Instance(){
  static ODEObjectFactory instance;
  return instance;
}

ODEObjectFactory::ODEObjectFactory(){
}

ODEObjectFactory::~ODEObjectFactory(){
}

std::shared_ptr<NodeInterface> ODEObjectFactory::createObject(std::shared_ptr<PhysicsInterface> worldPhysics, NodeData * nodeData){

  std::map<const std::string, instantiateObjectfPtr>::iterator it = availableObjects.find(nodeData->nodeType);
  if(it == availableObjects.end()){
    throw std::runtime_error("Could not load unknown Physics Object with name: \"" + nodeData->name + "\"" );
  }

  std::shared_ptr<ODEObject> newObject = std::make_shared<ODEObject>(*(it->second(worldPhysics, nodeData)));
  if(newObject->isObjectCreated()){
    return newObject;
  }
  else{
    std::cerr << "Failed to create Physics Object with name: \"" + nodeData->name + "\"" << std::endl;
    return std::shared_ptr<NodeInterface>(nullptr);
  }
}

void ODEObjectFactory::addObjectType(const std::string& type, instantiateObjectfPtr funcPtr){
  availableObjects.insert(std::pair<const std::string, instantiateObjectfPtr>(type,funcPtr));
}

}
}
