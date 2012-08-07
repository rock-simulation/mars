/*
 *  Copyright 2012, DFKI GmbH Robotics Innovation Center
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
 * \file MantisPlugin.h
 * \author Lorenz Quack (lorenz.quack@dfki.de)
 * \brief A MARS plugin to test mantis configurations.
 *
 * Version 0.1
 */

#ifndef MANTIS_PLUGIN_H
#define MANTIS_PLUGIN_H

#ifdef _PRINT_HEADER_
  #warning "MantisPlugin.h"
#endif

#include <mars/interfaces/sim/MarsPluginTemplate.h>
#include <mars/interfaces/MARSDefs.h>
#include <mars/data_broker/ReceiverInterface.h>
#include <mars/main_gui/MenuInterface.h>
#include <mars/main_gui/GuiInterface.h>

#include <string>

namespace mars {

  namespace lib_manager {
    class LibManager;
  }

  namespace plugins {
    namespace mantis_plugin {

      class MantisPlugin: public mars::interfaces::MarsPluginTemplate, 
                          public mars::data_broker::ReceiverInterface,
                          public mars::main_gui::MenuInterface {

      public:
        MantisPlugin(mars::lib_manager::LibManager *theManager);
        ~MantisPlugin();
        
        // LibInterface methods
        int getLibVersion() const
        { return 1; }
        const std::string getLibName() const
        { return std::string("MantisPlugin"); }

        // MarsPlugin methods
        void init();
        void reset();
        void update(mars::interfaces::sReal time_ms);
        
        // DataBrokerReceiver methods
        virtual void receiveData(const data_broker::DataInfo &info,
                                 const data_broker::DataPackage &package,
                                 int callbackParam);

        // MenuInterface methods
        void menuAction(int action, bool checked);

        // MantisPlugin methods
        void saveConfiguration(std::string filename="") const;
        void saveConfigurationAs() const;
        
      private:
        mars::main_gui::GuiInterface *gui;

      }; // end of class definition MantisPlugin
      
    } // end of namespace mantis_plugin
  } // end of namespace plugins
} // end of namespace mars

#endif // MANTIS_PLUGIN_H
