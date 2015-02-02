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
 * \file MenuFile.h
 * \author Vladimir Komsiyski
 * \brief MenuFile creates the menus and menu items in the File menu of the simulation.
 */

#ifndef MENUFILE_H
#define MENUFILE_H

#ifdef _PRINT_HEADER_
#warning "MenuFile.h"
#endif

#include "MainGUIDefs.h"

#include <mars/main_gui/MenuInterface.h>

#include <string>

namespace lib_manager {
  class LibManager;
}

namespace mars {

  namespace interfaces {
    class ControlCenter;
  }

  namespace main_gui {
    class GuiInterface;
  }

  namespace gui {

    /**
     * \brief MenuFile creates the menus and menu items in the File menu of the simulation.
     */
    class MenuFile :  public main_gui::MenuInterface {

    public:

      /** \brief The constructor adds the actions to the File menu */
      MenuFile(interfaces::ControlCenter *c, main_gui::GuiInterface *gui,
               std::string resPath, lib_manager::LibManager *theManager);
      ~MenuFile();

      /**
       * \brief Called whenever a menu item is selected.
       * \param action The action that is passed to the GuiInterface 
       * via addGenericMenuAction function.
       * \param checked Indicates if the menu/menu item is checked or not. 
       */
      virtual void menuAction(int action, bool checked = false);

    private:
 
      lib_manager::LibManager *libManager;
      main_gui::GuiInterface *mainGui;
      interfaces::ControlCenter *control;

      void menu_exportScene();
      void menu_openPlugin();
      void menu_openSceneFile();
      void menu_saveSceneFile();
      void menu_resetScene();
      void menu_newScene();

    };

  } // end of namespace gui
} // end of namespace mars

#endif
