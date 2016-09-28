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

#include "config.h"
#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>

#include "MenuWindow.h"

#include <osgDB/WriteFile>
#include <vector>

#include <mars/main_gui/GuiInterface.h>

#include <QKeySequence>
#include <QWidget>
#include <QSize>

#ifdef WIN32
#include <sys/timeb.h>
#include <windows.h>
#else
#include <sys/time.h>
#include <dlfcn.h>
#endif

#include "CameraConfigurator.h"
#include "CaptureWindow.h"

#define MENU_SHOW_VISUAL    -1
#define MENU_SHOW_PHYSICAL  -2
#define MENU_SHOW_COORDS    -3
#define MENU_SHOW_GRID      -4
#define MENU_VIEW_TOP       -5
#define MENU_VIEW_FRONT     -6
#define MENU_VIEW_RIGHT     -7
#define MENU_VIEW_BACK      -8
#define MENU_VIEW_LEFT      -9
#define MENU_VIEW_BOTTOM    -10
#define MENU_SHOW_CONTACTS  -11
#define MENU_SHOW_SELECTION -12

namespace mars {
  namespace gui {

    MenuWindow::MenuWindow(interfaces::ControlCenter *c, main_gui::GuiInterface *gui)
      : control(c), mainGui(gui) {

      cc = NULL;
      blender = NULL;
      std::string path;
      updateProp = true;
      mainGui->addGenericMenuAction("../View/New 3DView",
                                    GUI_ACTION_ADD_WINDOW,
                                    (main_gui::MenuInterface*)this, 0);


      // mainGui->addGenericMenuAction("../Windows/", 0, NULL, 0, "", 0, -1);
      // negative last argument meaning this is separator

      if (control->cfg) {
        cfg_manager::cfgPropertyStruct r_path;
        r_path = control->cfg->getOrCreateProperty("MarsGui", "resources_path",
                                                   std::string(MARS_GUI_DEFAULT_RESOURCES_PATH));
        path = r_path.sValue;
      }

      path.append("/images/blender.png");
      mainGui->addGenericMenuAction("../File/Export/Blender",
                                    GUI_ACTION_BLENDER_EXPORT,
                                    (main_gui::MenuInterface*)this,
                                    0, path, false);

      mainGui->addGenericMenuAction("../Control/Connect External Controllers",
                                    GUI_ACTION_CONTROLLER_CONFIG,
                                    (main_gui::MenuInterface*)this, 0);

      new CameraConfigurator(control, mainGui);
      new CaptureWindow(control, mainGui);


      //mainGui->addGenericMenuAction("../Windows/", 0, NULL, 0, "", 0, -1);
      // checkable dock action initialized with the current dock view
      mainGui->addGenericMenuAction("../View/Dock",
                                    GUI_ACTION_DOCK_WINDOWS,
                                    (main_gui::MenuInterface*)this,
                                    QKeySequence("CTRL+D")[0], "", 0,
                                    1+mainGui->getDocking());

      cfgVisRep = control->cfg->getOrCreateProperty("Simulator", "visual rep.",
                                                    (int)1, this);
      cfgShowCoords = control->cfg->getOrCreateProperty("Graphics", "showCoords",
                                                        true, this);
      cfgShowGrid = control->cfg->getOrCreateProperty("Graphics", "showGrid",
                                                        false, this);
      cfgShowContacts = control->cfg->getOrCreateProperty("Simulator", "draw contacts",
                                                        false, this);
      cfgShowSelection = control->cfg->getOrCreateProperty("Graphics", "showSelection",
                                                        true, this);

      // todo: update state if value in cfg_manager changes
      mainGui->addGenericMenuAction("../View/visual representation",
                                    MENU_SHOW_VISUAL,
                                    (main_gui::MenuInterface*)this,
                                    0, "", 0,
                                    1+(cfgVisRep.iValue & 1));
      mainGui->addGenericMenuAction("../View/physical representation",
                                    MENU_SHOW_PHYSICAL,
                                    (main_gui::MenuInterface*)this,
                                    0, "", 0,
                                    1+(cfgVisRep.iValue & 2));

      mainGui->addGenericMenuAction("../View/Show Coordinates",
                                    MENU_SHOW_COORDS,
                                    (main_gui::MenuInterface*)this,
                                    0, "", 0,
                                    1+cfgShowCoords.bValue);
      mainGui->addGenericMenuAction("../View/Show Grid",
                                    MENU_SHOW_GRID,
                                    (main_gui::MenuInterface*)this,
                                    0, "", 0,
                                    1+cfgShowGrid.bValue);

      mainGui->addGenericMenuAction("../View/Show Contacts",
                                    MENU_SHOW_CONTACTS,
                                    (main_gui::MenuInterface*)this,
                                    0, "", 0,
                                    1+cfgShowContacts.bValue);
      mainGui->addGenericMenuAction("../View/Show Selection",
                                    MENU_SHOW_SELECTION,
                                    (main_gui::MenuInterface*)this,
                                    0, "", 0,
                                    1+cfgShowSelection.bValue);

      mainGui->addGenericMenuAction("../View/Camera/Top",
                                    MENU_VIEW_TOP,
                                    (main_gui::MenuInterface*)this);
      mainGui->addGenericMenuAction("../View/Camera/Front",
                                    MENU_VIEW_FRONT,
                                    (main_gui::MenuInterface*)this);
      mainGui->addGenericMenuAction("../View/Camera/Right",
                                    MENU_VIEW_RIGHT,
                                    (main_gui::MenuInterface*)this);
      mainGui->addGenericMenuAction("../View/Camera/Back",
                                    MENU_VIEW_BACK,
                                    (main_gui::MenuInterface*)this);
      mainGui->addGenericMenuAction("../View/Camera/Left",
                                    MENU_VIEW_LEFT,
                                    (main_gui::MenuInterface*)this);
      mainGui->addGenericMenuAction("../View/Camera/Bottom",
                                    MENU_VIEW_BOTTOM,
                                    (main_gui::MenuInterface*)this);
    }


    //! The destructor destroys the dynamically loaded dialogs.
    MenuWindow::~MenuWindow() {

    }

    void MenuWindow::menuAction(int action, bool checked)
    {
      switch (action) {
      case GUI_ACTION_ADD_WINDOW:
        menu_addWindow();
        break;
      case GUI_ACTION_DOCK_WINDOWS:
        menu_dock(checked);
        break;
      case GUI_ACTION_BLENDER_EXPORT:
        menu_blender();
        break;
      case GUI_ACTION_CONTROLLER_CONFIG:
        menu_controller_config();
        break;
      case MENU_SHOW_VISUAL:
        if(updateProp) {
          if(checked)
            cfgVisRep.iValue |= 1;
          else
            cfgVisRep.iValue ^= 1;
          updateProp = false;
          control->cfg->setProperty(cfgVisRep);
          updateProp = true;
        }
        break;
      case MENU_SHOW_PHYSICAL:
        if(updateProp) {
          if(checked)
            cfgVisRep.iValue |= 2;
          else
            cfgVisRep.iValue ^= 2;
          updateProp = false;
          control->cfg->setProperty(cfgVisRep);
          updateProp = true;
        }
        break;
      case MENU_SHOW_COORDS:
        if(updateProp) {
          cfgShowCoords.bValue = checked;
          updateProp = false;
          control->cfg->setProperty(cfgShowCoords);
          updateProp = true;
        }
        break;
      case MENU_SHOW_GRID:
        if(updateProp) {
          cfgShowGrid.bValue = checked;
          updateProp = false;
          control->cfg->setProperty(cfgShowGrid);
          updateProp = true;
        }
        break;
      case MENU_SHOW_CONTACTS:
        if(updateProp) {
          cfgShowContacts.bValue = checked;
          updateProp = false;
          control->cfg->setProperty(cfgShowContacts);
          updateProp = true;
        }
        break;
      case MENU_SHOW_SELECTION:
        if(updateProp) {
          cfgShowSelection.bValue = checked;
          updateProp = false;
          control->cfg->setProperty(cfgShowSelection);
          updateProp = true;
        }
        break;
      case MENU_VIEW_TOP:
      case MENU_VIEW_FRONT:
      case MENU_VIEW_RIGHT:
      case MENU_VIEW_BACK:
      case MENU_VIEW_LEFT:
      case MENU_VIEW_BOTTOM:
        control->graphics->setCameraDefaultView(-action-4);
        break;
      }
    }

    void MenuWindow::menu_controller_config() {
      //close and delete existing dialog
      if (cc) {
        cc->close();
      }
      else {
        //create dialog
        cc = new ControllerConfigGUI(control, mainGui);
        connect(cc, SIGNAL(closeSignal(void*)),
                this, SLOT(closeWidget(void*)));
        mainGui->addDockWidget((void*)cc);
        cc->show();
      }
    }

    void MenuWindow::menu_blender() {
      //close and delete existing dialog
      if(blender) {
        blender->close();
      }
      else {
        //create dialog
        blender = new BlenderExportGUI(control, mainGui);
        connect(blender, SIGNAL(closeSignal(void*)),
                this, SLOT(closeWidget(void*)));
        mainGui->addDockWidget((void*)blender);
        blender->show();
      }
    }

    void MenuWindow::menu_addWindow(){
      QWidget* newWidget = new QWidget();

      newWidget->resize(QSize(720, 405));
      newWidget->setWindowTitle("marsGraphics");
      control->graphics->new3DWindow(newWidget);
      mainGui->addDockWidget((void*)newWidget,1);
      newWidget->show();
    }

    void MenuWindow::menu_dock(bool checked) {
      mainGui->dock(checked);
    }

    void MenuWindow::closeWidget(void* widget) {
      void **toClose = NULL;

      if(widget == blender) toClose = (void**)&blender;
      else if(widget == cc) toClose = (void**)&cc;

      if(toClose && *toClose) {
        mainGui->removeDockWidget(*toClose);
        if(*toClose) {
          delete (QObject*)*toClose;
          *toClose = NULL;
        }
      }
    }

    void MenuWindow::cfgUpdateProperty(cfg_manager::cfgPropertyStruct _property) {
      if(cfgVisRep.paramId == _property.paramId) {
        cfgVisRep.iValue = _property.iValue;
        if(updateProp) {
          updateProp = false;
          mainGui->setMenuActionSelected("../View/visual representation",
                                         cfgVisRep.iValue & 1);
          mainGui->setMenuActionSelected("../View/physical representation",
                                         cfgVisRep.iValue & 2);
          updateProp = true;
        }
      }
      if(cfgShowCoords.paramId == _property.paramId) {
        cfgShowCoords.bValue = _property.bValue;
        if(updateProp) {
          updateProp = false;
          mainGui->setMenuActionSelected("../View/Show Coordinates",
                                         cfgShowCoords.bValue);
          updateProp = true;
        }
      }
      if(cfgShowGrid.paramId == _property.paramId) {
        cfgShowGrid.bValue = _property.bValue;
        if(updateProp) {
          updateProp = false;
          mainGui->setMenuActionSelected("../View/Show Grid",
                                         cfgShowGrid.bValue);
          updateProp = true;
        }
      }
      if(cfgShowContacts.paramId == _property.paramId) {
        cfgShowContacts.bValue = _property.bValue;
        if(updateProp) {
          updateProp = false;
          mainGui->setMenuActionSelected("../View/Show Contacts",
                                         cfgShowContacts.bValue);
          updateProp = true;
        }
      }
      if(cfgShowSelection.paramId == _property.paramId) {
        cfgShowSelection.bValue = _property.bValue;
        if(updateProp) {
          updateProp = false;
          mainGui->setMenuActionSelected("../View/Show Selection",
                                         cfgShowSelection.bValue);
          updateProp = true;
        }
      }
    }

  } // end of namespace gui
} // end of namespace mars
