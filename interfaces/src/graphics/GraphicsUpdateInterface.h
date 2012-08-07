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
 * \file GraphicsUpdateInterface.h
 * \author Malte Roemmermann
 * \brief "GraphicsUpdateInterface"
 *
 */

#ifndef MARS_INTERFACES_GRAPHICS_UPDATE_INTERFACE_H
#define MARS_INTERFACES_GRAPHICS_UPDATE_INTERFACE_H

#ifdef _PRINT_HEADER_
  #warning "GraphicsUpdateInterface.h"
#endif

namespace mars {
  namespace interfaces {

    /**
     * The Interface to load controller dynamically into the simulation
     *
     */
    class GraphicsUpdateInterface {
    public:
      GraphicsUpdateInterface(void) {}
      virtual ~GraphicsUpdateInterface(void) {}
      virtual void preGraphicsUpdate(void) {}
      virtual void postGraphicsUpdate(void) {}
    }; // end of class GraphicsUpdateInterface

  } // end of namespace interfaces
} // end of namespace mars

#endif  /* MARS_INTERFACES_GRAPHICS_UPDATE_INTERFACE_H */
