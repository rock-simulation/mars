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

#ifdef PLUGIN_WITH_MARS_GUI
#include "MarsPluginTemplateGUI.h"
#else

#ifndef MARS_PLUGIN_TEMPLATE_H
#define MARS_PLUGIN_TEMPLATE_H

#include "ControlCenter.h"
#include "../MARSDefs.h"
#include "PluginInterface.h"
#include "SimulatorInterface.h"

#include <lib_manager/LibInterface.hpp>
#include <mars/cfg_manager/CFGManagerInterface.h>

#include <string>

namespace mars {
  namespace interfaces {

    class MarsPluginTemplate : public lib_manager::LibInterface,
                               public PluginInterface {

    public:

        MarsPluginTemplate(lib_manager::LibManager *theManager, std::string libName) :
        LibInterface(theManager), PluginInterface(NULL), mars(NULL) {

        lib_manager::LibInterface *lib;
        lib = libManager->getLibrary("mars_sim");
        pluginStruct newplugin;
        if(lib) {
          if( (mars = dynamic_cast<SimulatorInterface*>(lib)) ) {
            control = mars->getControlCenter();
            newplugin.name = libName;
            newplugin.p_interface = dynamic_cast<PluginInterface*>(this);
            newplugin.p_destroy = 0;
            newplugin.timer = newplugin.timer_gui = 0;
            newplugin.t_count = newplugin.t_count_gui = 0;

            if(control->cfg) {
              resourcesPath = control->cfg->getOrCreateProperty("Preferences",
                                                          "resources_path",
                                                          "").sValue;

              configPath = control->cfg->getOrCreateProperty("Config",
							     "config_path",
							     ".").sValue;
            }
          }
        }

        // this part should be the last line of the contructor
        // we can get a timing problem if mars want to use the
        // plugin before the contructor is finished -> so the last part here
        // is to register the plugin to mars
        if(mars) mars->addPlugin(newplugin);
      }

        ~MarsPluginTemplate() {
          if(mars) libManager->releaseLibrary("mars_sim");
        }

      private:
        SimulatorInterface *mars;

      protected:
        std::string configPath;
        std::string resourcesPath;
      };

  } // end of namespace interfaces
} // end of namespace mars

#endif // MARS_PLUGIN_TEMPLATE_H

#endif
