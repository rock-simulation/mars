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
 * \file MenuHelp.h
 * \brief Help section of the menu  
 */

#ifndef MENUHELP_H
#define MENUHELP_H

#ifdef _PRINT_HEADER_
  #warning "MenuHelp.h"
#endif

#include "MainGUIDefs.h"
#include <mars/interfaces/sim/ControlCenter.h>
#include "AboutDialog.h"

#include <mars/main_gui/MenuInterface.h>
#include <mars/main_gui/GuiInterface.h>

#include <QtGui>

namespace mars {
  namespace gui {

    /**
     * \brief MenuHelp is the part of the menu bar that manages the help dialogs
     * \todo provide meaningful about and help dialogs; not implemented yet
     */
    class MenuHelp :  public main_gui::MenuInterface {

    public:
      /** \brief The constructor adds the actions to the Remove menu */
      MenuHelp(interfaces::ControlCenter *c, main_gui::GuiInterface *gui);
      ~MenuHelp();

      virtual void menuAction(int action, bool checked = false);

    protected:
      // All these functions open the appropriate dialog
      void menu_about();
      void menu_aboutQt();

    private:
      AboutDialog *da;

    };

  } // end of namespace gui
} // end of namespace mars

#endif
