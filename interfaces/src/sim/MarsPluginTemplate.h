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

#ifndef MARS_PLUGIN_TEMPLATE_H
#define MARS_PLUGIN_TEMPLATE_H

#include "ControlCenter.h"
#include "../MARSDefs.h"
#include "PluginInterface.h"
#include "SimulatorInterface.h"

#include <mars/lib_manager/LibInterface.h>
#include <mars/cfg_manager/CFGManagerInterface.h>

#include <string>

#ifdef PLUGIN_WITH_MARS_GUI
#include <mars/main_gui/GuiInterface.h>
#endif

namespace mars {
  namespace interfaces {

#ifdef PLUGIN_WITH_MARS_GUI
    class MarsPluginTemplateGUI : public lib_manager::LibInterface,
#else
    class MarsPluginTemplate : public lib_manager::LibInterface,
#endif
                               public PluginInterface {

    public:
  
#ifdef PLUGIN_WITH_MARS_GUI
      MarsPluginTemplateGUI(lib_manager::LibManager *theManager, std::string libName) : 
#else
        MarsPluginTemplate(lib_manager::LibManager *theManager, std::string libName) : 
#endif
        LibInterface(theManager), PluginInterface(NULL), mars(NULL) {

        lib_manager::LibInterface *lib;
        lib = libManager->getLibrary("mars_sim");

        if(lib) {
          if( (mars = dynamic_cast<SimulatorInterface*>(lib)) ) {
            control = mars->getControlCenter();
            pluginStruct newplugin;
            newplugin.name = libName;
            newplugin.p_interface = dynamic_cast<PluginInterface*>(this);
            newplugin.p_destroy = 0;
            mars->addPlugin(newplugin);
      
            if(control->cfg) {
              cfg_manager::cfgPropertyStruct cfgPath;
              cfgPath = control->cfg->getOrCreateProperty("Preferences",
                                                          "resources_path",
                                                          std::string("."));
          
              resourcesPath = cfgPath.sValue;

              cfgPath = control->cfg->getOrCreateProperty("Config",
                                                          "config_path",
                                                          std::string("."));
      
              configPath = cfgPath.sValue;
            }
          }
        }

#ifdef PLUGIN_WITH_MARS_GUI
        gui = NULL;
        lib = libManager->getLibrary("main_gui");
        if(lib) {
          gui = dynamic_cast<main_gui::GuiInterface*>(lib);
        }
#endif
      }

#ifdef PLUGIN_WITH_MARS_GUI
      ~MarsPluginTemplateGUI() {
        if(mars) libManager->unloadLibrary("mars_sim");
        if(gui) libManager->unloadLibrary("main_gui");
#else
        ~MarsPluginTemplate() {
          if(mars) libManager->unloadLibrary("mars_sim");
#endif
        }

      private:
        SimulatorInterface *mars;

      protected:
        std::string configPath;
        std::string resourcesPath;
#ifdef PLUGIN_WITH_MARS_GUI
        main_gui::GuiInterface *gui;
#endif
      };

  } // end of namespace interfaces
} // end of namespace mars

#endif // MARS_PLUGIN_TEMPLATE_H
