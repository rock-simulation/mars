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
 * \file MenuWindow.cpp
 * \author Vladimir Komsiyski
 * \brief MenuWindow creates the menus and menu items in the Window menu of the simulation
 * and manages the window dialogs.
 */

#ifndef MENUWINDOW_H
#define MENUWINDOW_H

#ifdef _PRINT_HEADER_
#warning "MenuWindow.h"
#endif

#include "MainGUIDefs.h"
#include "BlenderExportGUI.h"
#include "ControllerConfigGUI.h"
#include <mars/main_gui/MenuInterface.h>

#include <sstream>

namespace mars {

  namespace main_gui {
    class GuiInterface;
  }

  namespace gui {

    /**
     * \brief MenuWindow creates the menus and menu items in the Window menu of the simulation.
     */
    class MenuWindow :  public QObject, public main_gui::MenuInterface,
                        public cfg_manager::CFGClient {

      Q_OBJECT

    public:

      /** \brief The constructor add the actions to the Window menu */
      MenuWindow(interfaces::ControlCenter *c, main_gui::GuiInterface *gui);
      ~MenuWindow();

      /**
       * \brief Called whenever a menu item is selected.
       * \param action The action that is passed to the GuiInterface 
       * via addGenericMenuAction function.
       * \param checked Indicates if the menu/menu item is checked or not. 
       */
      virtual void menuAction(int action, bool checked = false);

      // CFGClient methods
      virtual void cfgUpdateProperty(cfg_manager::cfgPropertyStruct _property);

    private:
      void menu_addWindow();
      void menu_dock(bool checked);
      void menu_blender();
      void menu_controller_config();

      interfaces::ControlCenter *control;
      main_gui::GuiInterface *mainGui;

      BlenderExportGUI *blender;
      ControllerConfigGUI *cc;
      cfg_manager::cfgPropertyStruct cfgVisRep, cfgShowCoords, cfgShowGrid;
      cfg_manager::cfgPropertyStruct cfgShowContacts, cfgShowSelection;
      bool updateProp;

    private slots:
      void closeWidget(void* widget);

    };

  } // end of namespace gui
} // end of namespace mars

#endif
