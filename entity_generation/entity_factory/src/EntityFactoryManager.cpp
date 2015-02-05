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
 * \file EntityFactoryManager.cpp
 * \author Kai von Szadkowski (kai.von-szadkowski@uni-bremen.de)
 * \brief a
 *
 * Version 0.1
 */

#include "EntityFactoryManager.h"
#include "EntityFactoryInterface.h"
#include <mars/data_broker/DataBrokerInterface.h>
#include <mars/data_broker/DataPackage.h>
#include <mars/sim/SimEntity.h>
#include <mars/utils/MutexLocker.h>
#include <mars/interfaces/sim/EntityManagerInterface.h>

namespace mars {

  namespace plugins {
    namespace entity_generation {

      using namespace mars::interfaces;

      EntityFactoryManager::EntityFactoryManager(lib_manager::LibManager *theManager) :
          MarsPluginTemplate(theManager, "EntityFactoryManager") {
      }

      void EntityFactoryManager::init() {
        control->sim->switchPluginUpdateMode(0, this);
      }

      void EntityFactoryManager::reset() {
      }

      EntityFactoryManager::~EntityFactoryManager() {
      }

      void EntityFactoryManager::update(sReal time_ms) {
      }

      void EntityFactoryManager::cfgUpdateProperty(cfg_manager::cfgPropertyStruct _property) {
      }

      void EntityFactoryManager::registerFactory(const std::string type,
          EntityFactoryInterface* factory) {
        utils::MutexLocker locker(&iMutex);
        factories[type] = factory;
        fprintf(stderr, "EntityFactory: registering factory for type '%s'", type.c_str());
      }

      unsigned long EntityFactoryManager::createEntity(configmaps::ConfigMap& config) {
        unsigned long id = 0;
        utils::MutexLocker locker(&iMutex);
        std::map<std::string, EntityFactoryInterface*>::iterator it;
        bool knowntype = false;
        for(it=factories.begin(); it!=factories.end(); ++it) {
          //fprintf(stderr, "Listing types: %s.\n", (it->second->getType()).c_str());
          if((std::string)config["type"] == (std::string)(it->first))
            knowntype = true;
        }
        if(knowntype) {
          fprintf(stderr, "Loading Entity of type %s.\n", ((std::string)config["type"]).c_str());
          sim::SimEntity* newentity = factories[config["type"]]->createEntity(config);
          id = control->entities->addEntity(newentity);
        }
        else {
          fprintf(stderr, "No EntityFactory known for type %s.\n", ((std::string)config["type"]).c_str());
          id = 0;
        }
        return id;
      }

      unsigned long EntityFactoryManager::createEntity(std::string configfile) {
        configmaps::ConfigMap config = configmaps::ConfigMap::fromYamlFile(configfile);
        return createEntity(config);
      }

    } // end of namespace entity_generation
  } // end of namespace plugins
} // end of namespace mars

DESTROY_LIB(mars::plugins::entity_generation::EntityFactoryManager);
CREATE_LIB(mars::plugins::entity_generation::EntityFactoryManager);
