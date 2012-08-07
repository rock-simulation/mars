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

#ifndef MARS_INTERFACES_GRAPHICS_DRAW_INTERFACE_H
#define MARS_INTERFACES_GRAPHICS_DRAW_INTERFACE_H

#ifdef _PRINT_HEADER_
  #warning "DrawInterface.h"
#endif

#include "draw_structs.h"
#include <vector>

namespace mars {
  namespace interfaces {

    struct draw_item;

    /**
     * The interface DrawInterface is used for updating draw_item structs via an update function
     */
    class DrawInterface {

    public:
      /** 
       * This is the update function that updates the position of the vertices of all draw_item
       * structs contained in drawItems
       */
      virtual void update(std::vector<draw_item> *drawItems) = 0;
      virtual ~DrawInterface(){}
    }; // end of class DrawInterface

  } // end of namespace interfaces
} // end of namespace mars

#endif  /* MARS_INTERFACES_GRAPHICS_DRAW_INTERFACE_H */
