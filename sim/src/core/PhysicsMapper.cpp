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
 * \file PhysicsMapper.cpp
 * \author Malte Roemmermann
 * \brief "PhysicsMapper" connects the implemented interface classes to
 * the interface objects that are handled in the core layer.
 *
 */

#include "PhysicsMapper.h"

namespace mars {
  namespace sim {
  
    using namespace mars::interfaces;

    std::shared_ptr<PhysicsInterface> PhysicsMapper::newWorldPhysics(ControlCenter *control) {
      std::shared_ptr<WorldPhysics> worldPhysics = std::make_shared<WorldPhysics>(control);
      return std::static_pointer_cast<PhysicsInterface>(worldPhysics);
    }

    std::shared_ptr<NodeInterface> PhysicsMapper::newODEObject(std::shared_ptr<PhysicsInterface> worldPhysics, NodeData * nodeData) {
      // Create a nodePhysics with the worldPhysics as constructor parameter, then downcast to a Node interface and return
      std::shared_ptr<NodeInterface> nodeInterface;
      // first we create a ode geometry for the node
      switch(nodeData->physicMode) {
//     case NODE_TYPE_MESH:
//       ret = createMesh(node);
//       break;
      case NODE_TYPE_BOX:
          nodeInterface = std::static_pointer_cast<NodeInterface>(std::make_shared<ODEBox>(worldPhysics, nodeData));
        break;
//      case NODE_TYPE_SPHERE:
//        ret = createSphere(node);
//        break;
//      case NODE_TYPE_CAPSULE:
//        ret = createCapsule(node);
//        break;
//      case NODE_TYPE_CYLINDER:
//        ret = createCylinder(node);
//        break;
//      case NODE_TYPE_PLANE:
//        ret = createPlane(node);
//        break;
//      case NODE_TYPE_TERRAIN:
//        ret = createHeightfield(node);
//        break;
      default:
        // no correct type is spezified, so no physically node will be created
        std::cout << "DEBUGGG: default of switch case in PhysicsMapper " << __FILE__ << ":" << __LINE__ << std::endl;
        return 0;
        break;
      }

      if( ! nodeInterface) {
        // Error creating the physical Node
        std::cout << "DEBUGGG: ODEObject creation failed in PhysicsMapper " << __FILE__ << ":" << __LINE__ << std::endl;
        return 0;
      }

      if ( nodeInterface )
      {
        std::cout << "DEBUGGG: After switch: nodeInterface initialized and valid for some reason" << __FILE__ << ":" << __LINE__ << std::endl;
      }
      if ( nodeInterface.get() ) {
        std::cout << "DEBUGGG: After switch: nodeInterface initialized and internal ptr valid for some reason" << __FILE__ << ":" << __LINE__ << std::endl;
      }
      if ( nodeInterface.get() == nullptr ) {
        std::cout << "DEBUGGG: After switch: nodeInterface initialized and internal ptr == nullptr" << __FILE__ << ":" << __LINE__ << std::endl;
      }
      if ( nodeInterface == nullptr ) {
        std::cout << "DEBUGGG: After switch: nodeInterface initialized and shared_ptr == nullptr" << __FILE__ << ":" << __LINE__ << std::endl;
      }

      std::cout << "DEBUGGG: return NodeInterface as static cast of ODEObject" << __FILE__ << ":" << __LINE__ << std::endl;
      return nodeInterface;

    }

      std::shared_ptr<JointInterface> PhysicsMapper::newJointPhysics(std::shared_ptr<PhysicsInterface> worldPhysics) {
      std::shared_ptr<JointPhysics> jointPhysics = std::make_shared<JointPhysics>(worldPhysics);
      return std::static_pointer_cast<JointInterface>(jointPhysics);
    }

  } // end of namespace sim
} // end of namespace mars
