#pragma once

#include <ODEObjectCreator.h>
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

ODEObjectCreator* ODEObjectCreator::instance = NULL;

ODEObjectCreator* ODEObjectCreator::getInstance(){
  if (NULL == instance){
    instance = new ODEObjectCreator();
  }
  return instance;
} 

ODEObjectCreator::ODEObjectCreator(){
}

ODEObjectCreator::~ODEObjectCreator(){
}

std::shared_ptr<NodeInterface> ODEObjectCreator::createObject(std::shared_ptr<PhysicsInterface> worldPhysics, NodeData * nodeData){
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
