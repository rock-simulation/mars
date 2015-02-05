/*
 *  Copyright 2014 DFKI GmbH Robotics Innovation Center
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
 * \file EntityFactoryInterface.h
 * \author Kai von Szadkowski (kai.von-szadkowski@uni-bremen.de)
 * \brief "EntityFactoryInterface" is an interface to dynamically create simulation
 *        entities such as robots or environment items from a set of configuration
 *        parameters.
 */

#ifndef ENTITY_FACTORY_INTERFACE_H
#define ENTITY_FACTORY_INTERFACE_H

#ifdef _PRINT_HEADER_
#warning "EntityFactoryInterface.h"
#endif

namespace configmaps {
    class ConfigMap;
  }
  
namespace mars {

  namespace sim {
    class SimEntity;
  }

  namespace plugins {
    namespace entity_generation {

      /**
       * The interface for plugins creating dynamic simulation objects
       *
       */
      class EntityFactoryInterface {

      public:
        EntityFactoryInterface(const std::string type) {
          this->type = type;
        }
        virtual ~EntityFactoryInterface(void) {
        }
        ;
        virtual sim::SimEntity* createEntity(const configmaps::ConfigMap& config) = 0;
        virtual std::string getType() {
          return this->type;
        }

      protected:
        std::string type;

      };

    } // end of namespace entity_generation
  } // end of namespace plugins
} // end of namespace mars

#endif  // ENTITY_FACTORY_INTERFACE_H
