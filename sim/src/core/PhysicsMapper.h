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
 * \file PhysicsMapper.h
 * \author Malte Roemmermann
 * \brief "PhysicsMapper" connects the implemented interface classes to
 * the interface objects that are handled in the core layer.
 *
 */

#ifndef PHYSICS_MAPPER_H
#define PHYSICS_MAPPER_H

#ifdef _PRINT_HEADER_
  #warning "PhysicsMapper.h"
#endif

#include "WorldPhysics.h"
#include "NodePhysics.h"
#include "JointPhysics.h"

namespace mars {
  namespace sim {

    class PhysicsMapper {
    public:
      static std::shared_ptr<interfaces::PhysicsInterface> newWorldPhysics(interfaces::ControlCenter *control);
      static std::shared_ptr<interfaces::NodeInterface> newNodePhysics(std::shared_ptr<interfaces::PhysicsInterface> worldPhysics);
      static std::shared_ptr<interfaces::JointInterface> newJointPhysics(std::shared_ptr<interfaces::PhysicsInterface> worldPhysics);
    
    };

  } // end of namespace sim
} // end of namespace mars

#endif  // PHYSICS_MAPPER_H
