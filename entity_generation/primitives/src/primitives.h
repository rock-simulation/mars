/*
 *  Copyright 2011, 2012, 2014, DFKI GmbH Robotics Innovation Center
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

#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#ifdef _PRINT_HEADER_
#warning "primitives.h"
#endif

#include <map>

#include <configmaps/ConfigData.h>
#include <mars/interfaces/MaterialData.h>
#include <mars/interfaces/sim/MarsPluginTemplate.h>
#include <mars/entity_generation/entity_factory/EntityFactoryInterface.h>

namespace mars {

  namespace smurf {

    class PRIMITIVES: public interfaces::MarsPluginTemplate,
        public mars::entity_generation::EntityFactoryInterface {

    public:
      PRIMITIVES(lib_manager::LibManager *theManager);
      ~PRIMITIVES();

      // LibInterface methods
      int getLibVersion() const {return 1;}
      const std::string getLibName() const {return std::string("PRIMITIVES");}
      CREATE_MODULE_INFO();

      // EntityFactoryInterface
      virtual sim::SimEntity* createEntity(const configmaps::ConfigMap& config);

      // MarsPlugin methods
      void init();
      void reset();
      void update(mars::interfaces::sReal time_ms);

    private:
      std::string primitivename;
      sim::SimEntity* entity;
      std::string tmpPath;
      void handleURI(configmaps::ConfigMap *map, std::string uri);
      void handleURIs(configmaps::ConfigMap *map);
    };

  } // end of namespace smurf
} // end of namespace mars

#endif  // PRIMITIVES_H
