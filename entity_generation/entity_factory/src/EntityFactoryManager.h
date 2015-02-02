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
 * \file EntityFactoryManager.h
 * \author Kai von Szadkowski (kai.von-szadkowski@uni-bremen.de)
 * \brief a
 *
 * Version 0.1
 */

#ifndef MARS_PLUGINS_ENTITY_FACTORY_MANAGER_H
#define MARS_PLUGINS_ENTITY_FACTORY_MANAGER_H

#ifdef _PRINT_HEADER_
#warning "EntityFactoryManager.h"
#endif

#include <mars/interfaces/sim/MarsPluginTemplate.h>
#include <mars/interfaces/MARSDefs.h>
#include <mars/data_broker/ReceiverInterface.h>
#include <mars/cfg_manager/CFGManagerInterface.h>
#include <mars/utils/Mutex.h>

namespace mars {

  namespace plugins {
    namespace entity_generation {

      class EntityFactoryInterface;

// inherit from MarsPluginTemplateGUI for extending the gui
      class EntityFactoryManager: public interfaces::MarsPluginTemplate,
          public cfg_manager::CFGClient {

      public:
        EntityFactoryManager(lib_manager::LibManager *theManager);
        ~EntityFactoryManager();

        // LibInterface methods
        int getLibVersion() const {
          return 1;
        }
        const std::string getLibName() const {
          return std::string("entity_factory_manager");
        }
        CREATE_MODULE_INFO()
        ;

        // MarsPlugin methods
        void init();
        void reset();
        void update(interfaces::sReal time_ms);

        // CFGClient methods
        virtual void cfgUpdateProperty(cfg_manager::cfgPropertyStruct _property);
        virtual void registerFactory(const std::string type, EntityFactoryInterface* factory);

        // creates new entity using an entity factory
        virtual unsigned long createEntity(utils::ConfigMap& config);
        virtual unsigned long createEntity(std::string configfile);

        // EntityFactoryManager methods

      private:
        std::map<std::string, EntityFactoryInterface*> factories;
        mutable utils::Mutex iMutex;

      };
// end of class definition EntityFactoryManager

    }// end of namespace entity_generation
  } // end of namespace plugins
} // end of namespace mars

#endif // MARS_PLUGINS_ENTITY_FACTORY_MANAGER_H
