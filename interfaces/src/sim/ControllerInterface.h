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
 * \file ControllerInterface.h
 * \author Malte Roemmermann
 * \brief "ControllerInterface" is an interface to load dynamically Controller
 *        into the simulation
 *
 */

#ifndef CONTOLLER_INTERFACE_H
#define CONTOLLER_INTERFACE_H

#ifdef _PRINT_HEADER_
  #warning "ControllerInterface.h"
#endif


#include "../sim_common.h"

namespace mars {
  namespace interfaces {

    /**
     * The Interface to load controller dynamically into the simulation
     *
     */
    class ControllerInterface {
    public:
      ControllerInterface(void) {};
      virtual ~ControllerInterface(void) {};
      virtual void update(sReal time_ms, sReal *sensors,
                          sReal *motors, int *flags, char **other) = 0;
      virtual void handleError(void) {}
    };

    typedef ControllerInterface* create_controller(void);
    typedef void destroy_controller(ControllerInterface*);

  } // end of namespace interfaces
} // end of namespace mars

#endif  // CONTOLLER_INTERFACE_H
