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

    PhysicsInterface* PhysicsMapper::newWorldPhysics(ControlCenter *control) {
      return (PhysicsInterface*) (new WorldPhysics(control));
    }

    NodeInterface* PhysicsMapper::newNodePhysics(PhysicsInterface *worldPhysics) {
      return (NodeInterface*) (new NodePhysics(worldPhysics));
    }

    JointInterface* PhysicsMapper::newJointPhysics(PhysicsInterface *worldPhysics) {
      return (JointInterface*) (new JointPhysics(worldPhysics));
    }

  } // end of namespace sim
} // end of namespace mars
