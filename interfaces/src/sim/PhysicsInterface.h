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
 * \file PhysicsInterface.h
 * \author Malte Roemmermann
 * \brief "PhysicsInterface" includes the methods to handle the physically world.
 */

#ifndef PHYSICS_INTERFACE_H
#define PHYSICS_INTERFACE_H

#ifdef _PRINT_HEADER_
  #warning "PhysicsInterface.h"
#endif

#include "../MARSDefs.h"

#include <mars/utils/Vector.h>

#include <memory>
#include <vector>

namespace mars {
  namespace interfaces {

    class NodeInterface;

    enum PhysicsError {
      PHYSICS_NO_ERROR = 0,
      PHYSICS_DEBUG,
      PHYSICS_ERROR,
      PHYSICS_UNKNOWN,
    };

    class PhysicsInterface {

    public:
      sReal ground_friction, ground_cfm, ground_erp;
      sReal step_size; /**< Step size in seconds */
      utils::Vector world_gravity;
      bool fast_step;
      bool draw_contact_points;
      sReal world_cfm, world_erp;

      virtual ~PhysicsInterface() {}
      virtual void initTheWorld(void) = 0;
      virtual void freeTheWorld(void) = 0;
      virtual void stepTheWorld(void) = 0;
      virtual bool existsWorld(void) const = 0;
      virtual const utils::Vector getCenterOfMass(const std::vector<std::shared_ptr<NodeInterface>> &nodes) const = 0;
      virtual int checkCollisions(void) = 0;
      virtual sReal getVectorCollision(const utils::Vector &pos, const utils::Vector &ray) const = 0;
    };

  } // end of namespace interfaces
} // end of namespace mars

#endif // PHYSICS_INTERFACE_H
