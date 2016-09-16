/*
 *  Copyright 2013, DFKI GmbH Robotics Innovation Center
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
 * \file Connectors.h
 * \author Kai (kai.von-szadkowski@uni-bremen.de)
 * \brief Allows
 *
 * Version 0.1
 */

#ifndef MARS_PLUGINS_CONNECTORS_H
#define MARS_PLUGINS_CONNECTORS_H

#ifdef _PRINT_HEADER_
  #warning "Connectors.h"
#endif


//#include <mars/interfaces/sim/MarsPluginTemplate.h>
#include <mars/interfaces/sim/MarsPluginTemplateGUI.h>
#include <mars/interfaces/MARSDefs.h>
#include <mars/interfaces/sim/EntitySubscriberInterface.h>
#include <mars/data_broker/ReceiverInterface.h>
#include <mars/cfg_manager/CFGManagerInterface.h>
#include <configmaps/ConfigData.h>

#include <string>

namespace mars {

  namespace plugins {
    namespace connectors {

      class Connectors: public mars::interfaces::MarsPluginTemplateGUI,
        public mars::data_broker::ReceiverInterface,
        public mars::main_gui::MenuInterface,
        public mars::cfg_manager::CFGClient,
        public mars::interfaces::EntitySubscriberInterface {

      public:
        Connectors(lib_manager::LibManager *theManager);
        ~Connectors();

        // LibInterface methods
        int getLibVersion() const
        { return 1; }
        const std::string getLibName() const
        { return std::string("connectors"); }
        CREATE_MODULE_INFO();

        // MarsPlugin methods
        void init();
        void reset();
        void update(mars::interfaces::sReal time_ms);
        void connect(std::string male, std::string female);
        void disconnect(std::string connector);

        // DataBrokerReceiver methods
        virtual void receiveData(const data_broker::DataInfo &info,
                                 const data_broker::DataPackage &package,
                                 int callbackParam);
        // CFGClient methods
        virtual void cfgUpdateProperty(cfg_manager::cfgPropertyStruct _property);

        // MenuInterface methods
        void menuAction(int action, bool checked = false);

        // Connectors methods
        void registerEntity(sim::SimEntity* entity);

      private:
        cfg_manager::cfgPropertyStruct cfgautoconnect, cfgbreakable;
        std::map<std::string, configmaps::ConfigMap> maleconnectors;
        std::map<std::string, configmaps::ConfigMap> femaleconnectors;
        std::map<std::string, configmaps::ConfigMap> connectortypes;
        std::map<std::string, std::string> connections;
        bool mated(std::string malename, std::string femalename);
        void checkForPossibleConnections();


      }; // end of class definition Connectors

    } // end of namespace connectors
  } // end of namespace plugins
} // end of namespace mars

#endif // MARS_PLUGINS_CONNECTORS_H
