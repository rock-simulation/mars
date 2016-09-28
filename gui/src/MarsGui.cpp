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

#include "config.h"
#include "MarsGui.h"

#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/interfaces/sim/SimulatorInterface.h>

#include <mars/main_gui/GuiInterface.h>
#include <mars/main_gui/MainGUI.h>
#include <mars/main_gui/MyQMainWindow.h>

#include "MenuFile.h"
#include "MenuAdd.h"
#include "MenuSimulation.h"
#include "MenuWindow.h"

using namespace std;

namespace mars {
  namespace gui {

    MarsGui::MarsGui(lib_manager::LibManager *theManager) : MarsGuiInterface(theManager), 
                                                            control(NULL), gui(NULL) {

    }

    MarsGui::~MarsGui() {
      if(control) {
        libManager->releaseLibrary("mars_sim");
      }
  
      if(gui) {
        libManager->releaseLibrary("main_gui");
      }  
      //fprintf(stderr, "Delete mars_gui\n");
    }


    void MarsGui::setupGui() {
      interfaces::SimulatorInterface *sim;
      sim =libManager->getLibraryAs<interfaces::SimulatorInterface>("mars_sim");
      if(sim) {
        control = sim->getControlCenter();
      }

      gui = libManager->getLibraryAs<main_gui::GuiInterface>("main_gui");
      if(!gui) {
        fprintf(stderr, "No main_gui lib found!\n");
      }
  
      resourcesPath.propertyType = cfg_manager::stringProperty;
      resourcesPath.propertyIndex = 0;
      resourcesPath.sValue = MARS_GUI_DEFAULT_RESOURCES_PATH;

      if(control && gui) {
        if(control->cfg) {
          configPath = control->cfg->getOrCreateProperty("Config", "config_path",
                                                         string(MARS_GUI_DEFAULT_RESOURCES_PATH));
      
          string loadFile = configPath.sValue;
          loadFile.append("/mars_Gui.yaml");
          control->cfg->loadConfig(loadFile.c_str());

          resourcesPath = control->cfg->getOrCreateProperty("MarsGui", "resources_path",
                                                            string(MARS_GUI_DEFAULT_RESOURCES_PATH));

          main_gui::MainGUI *mainGui;
          if((mainGui = dynamic_cast<main_gui::MainGUI*>(gui))) {
            mainGui->mainWindow_p()->setWindowIcon(QIcon(":/images/mars_icon.ico"));

            mainGui->addGenericMenuAction("../File/", 0, NULL, 0, "", 0, 0);
            mainGui->addGenericMenuAction("../Edit/", 0, NULL, 0, "", 0, 0);
            mainGui->addGenericMenuAction("../Control/", 0, NULL, 0, "", 0, 0);
            mainGui->addGenericMenuAction("../View/", 0, NULL, 0, "", 0, 0);
            mainGui->addGenericMenuAction("../Data/", 0, NULL, 0, "", 0, 0);
            mainGui->addGenericMenuAction("../Tools/", 0, NULL, 0, "", 0, 0);
            mainGui->addGenericMenuAction("../Plugins/", 0, NULL, 0, "", 0, 0);
            mainGui->addGenericMenuAction("../Options/", 0, NULL, 0, "", 0, 0);
          }

          /*
          if (marsStyle.bValue) {
            qApp->setStyle(new MarsStyle(resourcesPath.sValue+"/styles"));
          }
          */
        }

        new MenuFile(control, gui, resourcesPath.sValue, libManager);
        new MenuAdd(control, gui, resourcesPath.sValue, libManager);
        new MenuSimulation(control, gui, resourcesPath.sValue);
        new MenuWindow(control, gui);
      }
    }

    void MarsGui::cfgUpdateProperty(cfg_manager::cfgPropertyStruct _property) {

      if (_property.paramId == marsStyle.paramId) {
        /*        if (_property.bValue)
          QApplication::setStyle(new MarsStyle(resourcesPath.sValue+"/styles"));
        else
          QApplication::setStyle(new QPlastiqueStyle);
        */
        return;
      }	
    }
  } // end of namespace gui
} // end of namespace mars

DESTROY_LIB(mars::gui::MarsGui);
CREATE_LIB(mars::gui::MarsGui);

