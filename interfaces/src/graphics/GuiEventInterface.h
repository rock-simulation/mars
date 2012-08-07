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


#ifndef MARS_INTERFACES_GRAPHICS_GUI_EVENT_INTERFACE_H
#define MARS_INTERFACES_GRAPHICS_GUI_EVENT_INTERFACE_H

#ifdef _PRINT_HEADER_
  #warning "GuiEventInterface.h"
#endif

namespace mars {
  namespace interfaces {

    class GuiEventInterface {
    public:
      enum KeyType {
        Key_Backspace,
        Key_Tab,
        Key_Return,
        Key_Pause,
        Key_ScrollLock,
        Key_Escape,
        Key_Delete,
        Key_Home,
        Key_Left,
        Key_Up,
        Key_Right,
        Key_Down,
        Key_PageUp,
        Key_PageDown,
        Key_End,
        Key_Print,
        Key_Space,
        Key_Enter,
        Key_Insert,
        Key_NumLock,
        Key_Equal,
        Key_multiply,
        Key_Plus,
        Key_Minus,
        Key_Slash,
        Key_0,
        Key_1,
        Key_2,
        Key_3,
        Key_4,
        Key_5,
        Key_6,
        Key_7,
        Key_8,
        Key_9,
        Key_F1,
        Key_F2,
        Key_F3,
        Key_F4,
        Key_F5,
        Key_F6,
        Key_F7,
        Key_F8,
        Key_F9,
        Key_F10,
        Key_F11,
        Key_F12
      };

      enum KeyModifiers {
        ShiftModifier    =  1,
        ControlModifier  =  2,
        AltModifier      =  4,
        MetaModifier     =  8,
        KeypadModifier   = 16
      };

      virtual ~GuiEventInterface() {}

      virtual void keyUpEvent(int key, unsigned int mod, unsigned long win_id) = 0;
      virtual void quitEvent(unsigned long win_id) = 0;
      virtual void setAppActive(unsigned long win_id = 0) = 0;

    }; // end of class GuiEventInterface

  } // end of namespace interfaces
} // end of namespace mars

#endif  /* MARS_INTERFACES_GRAPHICS_GUI_EVENT_INTERFACE_H */
