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

#include <mars/lib_manager/LibManager.h>

//#include "MarsStyle.h"

#include "MenuFile.h"
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
        if(control->cfg) {
          string saveFile = configPath.sValue;
          saveFile.append("/mars_Gui.yaml");

          if(gui) {
            main_gui::MyQMainWindow *mqmw = ((main_gui::MainGUI*)(gui))->mainWindow_p();
            int windowTop = mqmw->geometry().y();
            int windowLeft = mqmw->geometry().x();
            int windowWidth = mqmw->geometry().width();
            int windowHeight = mqmw->geometry().height();
            bool docking = gui->getDocking();
            vector<main_gui::dockState> docks = mqmw->getDockGeometry();

 
            control->cfg->setPropertyValue("MarsGui", "Main Window/Window Top",
                                           "value", windowTop);
            control->cfg->setPropertyValue("MarsGui", "Main Window/Window Left",
                                           "value", windowLeft);
            control->cfg->setPropertyValue("MarsGui", "Main Window/Window Width",
                                           "value", windowWidth);
            control->cfg->setPropertyValue("MarsGui", "Main Window/Window Height",
                                           "value", windowHeight);
            control->cfg->setPropertyValue("MarsGui", "Main Window/docking",
                                           "value", docking);
	
            string allDocks = "";

            for (unsigned int i = 0; i <docks.size(); i++) {
              allDocks.append(docks[i].title.toStdString() + "%%"); // %% is delimiter
              int a = 0;
              switch (docks[i].area) {
              case Qt::LeftDockWidgetArea: a = 1; break;
              case Qt::RightDockWidgetArea: a = 2; break;
              case Qt::TopDockWidgetArea: a = 4; break;
              case Qt::BottomDockWidgetArea: a = 8; break;
              }

              if (control->cfg->setPropertyValue("MarsGui", docks[i].title.toStdString() + "/area", 
                                                 "value", a) == false)
                control->cfg->getOrCreateProperty("MarsGui", docks[i].title.toStdString() + "/area", 
                                                  a);
	  
              if (control->cfg->setPropertyValue("MarsGui", docks[i].title.toStdString() + "/floating",
                                                 "value", docks[i].floating) == false)
                control->cfg->getOrCreateProperty("MarsGui", docks[i].title.toStdString() + "/floating",
                                                  docks[i].floating);
	  
              if (control->cfg->setPropertyValue("MarsGui", docks[i].title.toStdString() + "/left",
                                                 "value", docks[i].rect.x()) == false)
                control->cfg->getOrCreateProperty("MarsGui", docks[i].title.toStdString() + "/left",
                                                  docks[i].rect.x());

              if (control->cfg->setPropertyValue("MarsGui", docks[i].title.toStdString() + "/top",
                                                 "value", docks[i].rect.y()) == false)
                control->cfg->getOrCreateProperty("MarsGui", docks[i].title.toStdString() + "/top",
                                                  docks[i].rect.y());
	  
              if (control->cfg->setPropertyValue("MarsGui", docks[i].title.toStdString() + "/width",
                                                 "value", docks[i].rect.width()) == false)
                control->cfg->getOrCreateProperty("MarsGui", docks[i].title.toStdString() + "/width",
                                                  docks[i].rect.width());
	  
              if (control->cfg->setPropertyValue("MarsGui", docks[i].title.toStdString() + "/height",
                                                 "value", docks[i].rect.height()) == false)
                control->cfg->getOrCreateProperty("MarsGui", docks[i].title.toStdString() + "/height",
                                                  docks[i].rect.height());

            }

            control->cfg->setPropertyValue("MarsGui", "Docks", "value", allDocks);
          }
      
          control->cfg->writeConfig(saveFile.c_str(), "MarsGui");
          saveFile = configPath.sValue;
          saveFile.append("/mars_Windows.yaml");
          control->cfg->writeConfig(saveFile.c_str(), "Windows");

        }
    
        libManager->releaseLibrary("mars_sim");
      }
  
      if(gui) {
        libManager->releaseLibrary("main_gui");
      }
  
      fprintf(stderr, "Delete mars_gui\n");
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
          loadFile = configPath.sValue;
          loadFile.append("/mars_Windows.yaml");
          control->cfg->loadConfig(loadFile.c_str());      

          resourcesPath = control->cfg->getOrCreateProperty("MarsGui", "resources_path",
                                                            string(MARS_GUI_DEFAULT_RESOURCES_PATH));

      
          cfgW_top = control->cfg->getOrCreateProperty("MarsGui",
                                                       "Main Window/Window Top", (int)400,
                                                       dynamic_cast<cfg_manager::CFGClient*>(this));
          cfgW_left = control->cfg->getOrCreateProperty("MarsGui", "Main Window/Window Left", (int)400,
                                                        dynamic_cast<cfg_manager::CFGClient*>(this));
          cfgW_width = control->cfg->getOrCreateProperty("MarsGui", "Main Window/Window Width", (int)400,
                                                         dynamic_cast<cfg_manager::CFGClient*>(this));
          cfgW_height = control->cfg->getOrCreateProperty("MarsGui", "Main Window/Window Height", (int)200,
                                                          dynamic_cast<cfg_manager::CFGClient*>(this));
      
          dockStyle = control->cfg->getOrCreateProperty("MarsGui", "Main Window/docking", false,
                                                        dynamic_cast<cfg_manager::CFGClient*>(this));

          marsStyle = control->cfg->getOrCreateProperty("MarsGui", "Mars Style", false,
                                                        dynamic_cast<cfg_manager::CFGClient*>(this));
   
          stateNamesProp = control->cfg->getOrCreateProperty("MarsGui", "Docks", string("."),
                                                             dynamic_cast<cfg_manager::CFGClient*>(this));
      
          vector<string> dockNames;
          string tmp = stateNamesProp.sValue;
      
          while (tmp != "") {
            size_t pos = tmp.find("%%");
            if (pos == string::npos) break;
            dockNames.push_back(tmp.substr(0, pos));
            tmp.erase(0, pos+2);
          }
      
       

          vector<main_gui::dockState> states;
          for (size_t i = 0; i < dockNames.size(); i++) {
            dockArea = control->cfg->getOrCreateProperty("MarsGui", dockNames[i] + "/area", 1, 
                                                         dynamic_cast<cfg_manager::CFGClient*>(this));
            dockFloat = control->cfg->getOrCreateProperty("MarsGui", dockNames[i] + "/floating", false, 
                                                          dynamic_cast<cfg_manager::CFGClient*>(this));
            dockLeft = control->cfg->getOrCreateProperty("MarsGui", dockNames[i] + "/left", 0, 
                                                         dynamic_cast<cfg_manager::CFGClient*>(this));
            dockTop = control->cfg->getOrCreateProperty("MarsGui", dockNames[i] + "/top", 0, 
                                                        dynamic_cast<cfg_manager::CFGClient*>(this));
            dockWidth = control->cfg->getOrCreateProperty("MarsGui", dockNames[i] + "/width", 0, 
                                                          dynamic_cast<cfg_manager::CFGClient*>(this));
            dockHeight = control->cfg->getOrCreateProperty("MarsGui", dockNames[i] + "/height", 0, 
                                                           dynamic_cast<cfg_manager::CFGClient*>(this));
	
            Qt::DockWidgetArea a= Qt::LeftDockWidgetArea;
            switch (dockArea.iValue) {
            case 1: a = Qt::LeftDockWidgetArea; break;
            case 2: a = Qt::RightDockWidgetArea; break;
            case 4: a = Qt::TopDockWidgetArea; break;
            case 8: a = Qt::BottomDockWidgetArea; break;
            default: a = Qt::NoDockWidgetArea; break;
            }
            QRect r(dockLeft.iValue, dockTop.iValue, dockWidth.iValue, dockHeight.iValue);

            main_gui::dockState add = {QString::fromStdString(dockNames[i]), a, dockFloat.bValue, r}; 
            states.push_back(add);
          }

          main_gui::MainGUI *mainGui;
          if((mainGui = dynamic_cast<main_gui::MainGUI*>(gui))) {
            mainGui->mainWindow_p()->setWindowIcon(QIcon(":/images/mars_icon.ico"));
            mainGui->mainWindow_p()->setMinimumSize(0, 0);

            mainGui->mainWindow_p()->setGeometry(cfgW_left.iValue, cfgW_top.iValue,
                                                 cfgW_width.iValue,
                                                 cfgW_height.iValue); 
            mainGui->mainWindow_p()->setDockGeometry(states);
            mainGui->dock(dockStyle.bValue);

          }

          /*
          if (marsStyle.bValue) {
            qApp->setStyle(new MarsStyle(resourcesPath.sValue+"/styles"));
          }
          */
        }

        new MenuFile(control, gui, resourcesPath.sValue, libManager);
        new MenuWindow(control, gui);
        new MenuSimulation(control, gui, resourcesPath.sValue);
    
        //    cfgUpdateProperty(cfgW_left);
    
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

      bool change_view = 0;
  
      if(_property.paramId == cfgW_top.paramId) {
        cfgW_top.iValue = _property.iValue;
        change_view = 1;
      } else if(_property.paramId == cfgW_left.paramId) {
        cfgW_left.iValue = _property.iValue;
        change_view = 1;
      } else if(_property.paramId == cfgW_width.paramId) {
        cfgW_width.iValue = _property.iValue;
        change_view = 1;
      } else if(_property.paramId == cfgW_height.paramId) {
        cfgW_height.iValue = _property.iValue;
        change_view = 1;
      }
      if(change_view && gui) {
        main_gui::MainGUI *mainGui;
        if((mainGui = dynamic_cast<main_gui::MainGUI*>(gui))) {
          mainGui->mainWindow_p()->setGeometry(cfgW_left.iValue, cfgW_top.iValue,
                                               cfgW_width.iValue,
                                               cfgW_height.iValue);
        }
      }

    }

  } // end of namespace gui
} // end of namespace mars

DESTROY_LIB(mars::gui::MarsGui);
CREATE_LIB(mars::gui::MarsGui);

