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
 * \file MainGUI.h
 * \author Malte Römmermann
 * \author Vladimir Komsiyski
 */

#ifndef MAINGUI_H
#define MAINGUI_H

#ifdef _PRINT_HEADER_
  #warning "MainGUI.h"
#endif

#include "MenuInterface.h"
#include "GuiInterface.h"
#include "MyQMdiArea.h"
#include "MyQMainWindow.h"

#include <vector>
#include <QtGui>

namespace mars {
  namespace main_gui {

    /**
     * \brief Holds the properties of a menu item.
     */
    struct menuStruct {
      QMenu *menu;
      QToolBar *toolbar;
      std::string label;
    };

    /**
     * \brief Holds the callback properties of a menu item.
     */
    struct genericMenu {
      std::string path;
      QAction *genericAction;
      MenuInterface *menu;
      int action;
    };


    /**
     * \brief The main part of the gui that manages all dialogs and menus.
     * \sa GuiInterface
     */
    class MainGUI : public QObject, public GuiInterface {

      Q_OBJECT

      public:

      /**
       * \biref The constructor initializes the library manager.
       */
      MainGUI(lib_manager::LibManager *theManager);

      /**
       * \brief A destructor.
       */
      virtual ~MainGUI();

      /**
       * \brief Adds a menu/menu item with its corresponding action.
       * \see GuiInterface::addGenericMenuAction(string, int, MenuInterface*, int, string, bool, int)
       */
      virtual void addGenericMenuAction(const std::string &path, int action,
                                        MenuInterface *menu, int qtKey=0,
                                        const std::string &icon = "",
                                        bool toolbar = 0, int checkable=0);

      virtual void setMenuActionSelected(const std::string &path, bool checked);

      /**
       * \brief Sets an image as a central widget in the main window.
       */
      virtual void setBackgroundImage(const std::string &path = "");

      /**
       * \brief Returns a pointer to the main window.
       */
      MyQMainWindow* mainWindow_p(void);

      /**
       * \brief Shows the main window with a predefined geometry.
       */
      void show(void);

      /**
       * \brief Docks/undocks all widgets in the main window.
       */
      void dock(bool checked);
      bool getDocking() const {return mainWindow->dockView;}

      // methods from LibInterface

      /**
       * \brief Returns the current version of the library.
       * \see LibInterface::getLibVersion(void)
       */
      int getLibVersion() const;

      /**
       * \brief Returns the name of the library.
       * \see LibInterface::getLibName(void)
       */
      const std::string getLibName() const;
      CREATE_MODULE_INFO();


    protected slots:

      /**
       * \brief Makes a widget dockable in the main window.
       * \see GuiInterface::addDockWidget(void*, int, int)
       */
      void addDockWidget(void *window, int p=0, int a=0);

      /**
       * \brief Removes a widget from the dockables.
       * \see GuiInterface::removeDockWidget(void*, int)
       */
      void removeDockWidget(void *window, int p=0);

      /**
       * \brief Called when a menu item is selected. Calls the
       * corresponding MenuInterface instance with the selected action.
       * \param checked Indicates if the menu item is checked or not.
       */
      void genericAction(bool checked = false);

      /**
       * \brief Shows the standard qt about dialog
       */
      void aboutQt() const;


    private:
      /**
       * \brief Holds the main window instance.
       */
      MyQMainWindow *mainWindow;

      /**
       * \brief Holds the mdi area instance.
       */
      MyQMdiArea *mdiArea;

      /**
       * \brief Holds the menu bar instance.
       */
      QMenuBar *menuBar;

      /**
       * \brief Holds the pointer to the helpMenu
       */
      QMenu *helpMenu;

      /**
       * \brief Holds the pointer to the aboutQt action
       */
      QAction *actionAboutQt;

      /**
       * \brief Indicates if a toolbar is to be shown or not.
       */
      bool allow_toolbar;

      // Hold the created menus and their properties
      /* todo: this only works well if the menu entries are unique,
               also between different submenus!
               Also using a map from name to menu might be better.
      */
      std::vector<menuStruct> v_qmenu;
      std::vector<genericMenu> genericMenus;

    }; // end class MainGUI

  } // end namespace main_gui
} // end namespace mars

#endif /* MAINGUI_H */
