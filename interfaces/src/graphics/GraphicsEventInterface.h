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


#ifndef MARS_INTERFACES_GRAPHICS_EVENT_INTERFACE_H
#define MARS_INTERFACES_GRAPHICS_EVENT_INTERFACE_H

#ifdef _PRINT_HEADER_
  #warning "GraphicsEventInterface.h"
#endif

#include "GuiEventInterface.h"

namespace mars {
  namespace interfaces {

    class GraphicsEventInterface {
    public:
      virtual ~GraphicsEventInterface() {}
      virtual void emitKeyUpEvent(int key, unsigned int modKey,
                                  unsigned long win_id) {}
      virtual void emitQuitEvent(unsigned long win_id) {}
      virtual void emitSetAppActive(unsigned long win_id = 0) {}
      virtual void emitNodeSelectionChange(unsigned long win_id, int mode) {}
      virtual void emitGeometryChange(unsigned long win_id,
                                      int left, int top,
                                      int width, int height) {}
      virtual void emitPickEvent(int x, int y) {}
    }; // end of class GraphicsEventInterface

  } // end of namespace interfaces
} // end of namespace mars

#endif  /* MARS_INTERFACES_GRAPHICS_EVENT_INTERFACE_H */
