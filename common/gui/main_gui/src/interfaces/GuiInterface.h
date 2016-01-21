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
 * \file GuiInterface.h
 * \author Malte Roemmermann
 * \author Vladimir Komsiyski
 */

#ifndef GUI_INTERFACE_H
#define GUI_INTERFACE_H

#ifdef _PRINT_HEADER_
  #warning "GuiInterface.h"
#endif

#include "MenuInterface.h"

#include <string>
#include <lib_manager/LibInterface.hpp>

namespace mars {
  namespace main_gui {


    /** \brief The abstract GuiInterface class provides the menu and 
     *  docking functionality of the GUI. Inherits from LibInterface.
     */
    class GuiInterface : public lib_manager::LibInterface {

    public:

      /** \biref The constructor initializes the library manager.*/
      GuiInterface(lib_manager::LibManager *theManager) : 
        lib_manager::LibInterface(theManager) {}

      /** \brief A destructor. */
      virtual ~GuiInterface() {}
  
      /** \brief Adds a menu/menu item with its corresponding action. 
       * \param path The menu path of the item being added. Should begin 
       * with "../". If a parent item does not exist, it is created.
       * \param action An unique for the class action used to recognize
       * the selected menu item.
       * \param menu The inheriting class, which implements the menuAction function.
       * \param qKey A key or key sequence that selects this menu entry.
       * \param icon A path to an icon file for the menu item.
       * \param toolbar A boolean value indicating if the item is also shown
       * in the toolbar of the main window.
       * \param checkable A boolean value indicating if the menu item is 
       * checkable or not.
       * \Example
       * \code
       * this->addGenericMenuAction("../File/New", ACTION_FILE_NEW, 
       (main_gui::MenuInterface*)this, QKeySequence("CTRL+N"), 
       "images/empty_file.jpg", 1, 0);
       * \endcode   
       * This example produces a menu item File->New with Ctrl+N shortcut to it,
       * with an icon, shown in the toolbar, not checkable.
       */
      virtual void addGenericMenuAction(const std::string &path, int action,
                                        MenuInterface* menu, int qtKey = 0,
                                        const std::string &icon = "", 
                                        bool toolbar = 0, int checkable = 0) = 0;
 
      /**
       * \brief Set the selection state of an menu action.
       */
      virtual void setMenuActionSelected(const std::string &path, bool checked) = 0;

      /** \brief Makes a widget dockable in the main window. 
       *  \param window A pointer to the widget being docked.
       * \param priority 0 indicates that the widget is closable, 1 - not.
       * \param area An indicator to the initial docking area of the widget.
       * Default area is Qt::LeftDockWidgetArea.
       */
      virtual void addDockWidget(void* window, int priority = 0, int area = 0) = 0;
  
      /** \brief Removes a widget from the dockables.
       * \param window The widget being removed.
       * \param priority If 0, the widget is closed; otherwise it is hidden.
       */
      virtual void removeDockWidget(void *window, int priority=0) = 0;
  
      /** \brief Docks/undocks all widgets in the main window. */
      virtual void dock(bool dockView) = 0;

      /** \brief Returns if docking mode is currently enabled. */
      virtual bool getDocking() const = 0;

      /** \brief Sets an image as a central widget in the main window. */ 
      virtual void setBackgroundImage(const std::string &path = "") = 0;

      /** \brief Shows the main window with a predefined geometry. */
      virtual void show(void) = 0;
  
    }; //end class

  } // end of namespace main_gui
} // end of namespace mars

#endif // GUI_INTERFACE_H
