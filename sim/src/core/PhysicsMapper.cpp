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

    std::shared_ptr<NodeInterface> PhysicsMapper::newNodePhysics(std::shared_ptr<PhysicsInterface> worldPhysics) {
      // Create a nodePhysics with the worldPhysics as constructor parameter, then downcast to a Node interface and return
      std::shared_ptr<NodePhysics> nodePhysics = std::make_shared<NodePhysics>(worldPhysics);
      return std::static_pointer_cast<NodeInterface>(nodePhysics);
    }

    std::shared_ptr<JointInterface> PhysicsMapper::newJointPhysics(std::shared_ptr<PhysicsInterface> worldPhysics) {
      std::shared_ptr<JointPhysics> jointPhysics = std::make_shared<JointPhysics>(worldPhysics);
      return std::static_pointer_cast<JointInterface>(jointPhysics);
    }

  } // end of namespace sim
} // end of namespace mars
