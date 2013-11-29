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

namespace mars {
  namespace gui {

    MenuWindow::MenuWindow(interfaces::ControlCenter *c, main_gui::GuiInterface *gui) 
      : control(c), mainGui(gui) {

      cc = NULL; 
      blender = NULL;
      std::string path;

      mainGui->addGenericMenuAction("../Windows/New 3D Window",
                                    GUI_ACTION_ADD_WINDOW, 
                                    (main_gui::MenuInterface*)this, 0);


      mainGui->addGenericMenuAction("../Windows/", 0, NULL, 0, "", 0, -1);
      // negative last argument meaning this is separator 

      if (control->cfg) {
        cfg_manager::cfgPropertyStruct r_path;
        r_path = control->cfg->getOrCreateProperty("MarsGui", "resources_path",
                                                   std::string(MARS_GUI_DEFAULT_RESOURCES_PATH));
        path = r_path.sValue;
      }

      path.append("/images/blender.png");
      mainGui->addGenericMenuAction("../Windows/Blender Export",
                                    GUI_ACTION_BLENDER_EXPORT,
                                    (main_gui::MenuInterface*)this, 
                                    0, path, true);
      mainGui->addGenericMenuAction("../Windows/Controller Configuration", 
                                    GUI_ACTION_CONTROLLER_CONFIG,
                                    (main_gui::MenuInterface*)this, 0);
			       
      new CameraConfigurator(control, mainGui);
      new CaptureWindow(control, mainGui);
  
  
      mainGui->addGenericMenuAction("../Windows/", 0, NULL, 0, "", 0, -1);
      mainGui->addGenericMenuAction("../Windows/Dock",
                                    GUI_ACTION_DOCK_WINDOWS, 
                                    (main_gui::MenuInterface*)this,
                                    QKeySequence("CTRL+D")[0], "", 0, 
                                    1+mainGui->getDocking());
      // checkable dock action initialized with the current dock view
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
        delete (QObject*)*toClose;
        *toClose = NULL;
      }

    }

  } // end of namespace gui
} // end of namespace mars
