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

#ifndef MARS_INTERFACES_NODE_STATE_H
#define MARS_INTERFACES_NODE_STATE_H

#include <mars/utils/Vector.h>

namespace mars {

  namespace interfaces {

    /**
     * \brief A physical state of the node.
     *
     * This struct should include the position, orientation, and velocities of a
     * node. Currently only the linear and angular velocity is used. While the
     * position and orientation is not included.
     * \deprecated A force and a torque of the nodeState is not futher used by
     *             the simulation.
     */
    struct nodeState {
      /**
       * The linear velocity of a node.
       */
      utils::Vector l_vel;

      /**
       * The angular velocity of a node.
       */
      utils::Vector a_vel;

      /**
       * The force velocity of a node.
       */
      utils::Vector f;

      /**
       * The torque of a node.
       */
      utils::Vector t;
    }; // end of struct nodeState

  } // end of namespace interfaces

} // end of namespace mars

#endif /* MARS_INTERFACES_NODE_STATE_H */
