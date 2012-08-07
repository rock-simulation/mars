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
 * \file MenuInterface.h
 * \author Malte Römmermann
 * \author Vladimir Komsiyski
 */


#ifndef MENU_INTERFACE_H
#define MENU_INTERFACE_H

#ifdef _PRINT_HEADER_
#warning "MenuInterface.h"
#endif

namespace mars {
  namespace main_gui {

    /**
     * \brief Provides the menuAction function. */
    class MenuInterface {
    public:
      virtual ~MenuInterface() {}

      /**
       * \brief Called whenever a menu item is selected.
       * \param action The action that is passed to the GuiInterface 
       * via addGenericMenuAction function.
       * \param checked Indicates if the menu/menu item is checked or not. 
       */
      virtual void menuAction(int action, bool checked = false) = 0;

    };

  } // end of namespace main_gui
} // end of namespace mars

#endif  // MENU_INTERFACE_H
