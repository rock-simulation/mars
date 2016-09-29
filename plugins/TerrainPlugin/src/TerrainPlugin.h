/*
 *  Copyright 2016, DFKI GmbH Robotics Innovation Center
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
 * \file TerrainPlugin.h
 * \author Malte (malte.langosz@dfki.de)
 * \brief SkyDom
 *
 * Version 0.1
 */

#ifndef MARS_PLUGINS_TERRAINPLUGIN_H
#define MARS_PLUGINS_TERRAINPLUGIN_H

#ifdef _PRINT_HEADER_
  #warning "TerrainPlugin.h"
#endif

// set define if you want to extend the gui
#include <mars/interfaces/sim/MarsPluginTemplate.h>
#include <mars/interfaces/MARSDefs.h>
#include <mars/osg_material_manager/OsgMaterialManager.h>
#include <mars/osg_terrain/Terrain.h>

#include <osg/Group>
#include <string>

namespace mars {

  namespace plugin {
    namespace TerrainPlugin {
      
      class TerrainPlugin: public mars::interfaces::MarsPluginTemplate{
      public:
        TerrainPlugin(lib_manager::LibManager *theManager);
        ~TerrainPlugin();

        // LibInterface methods
        int getLibVersion() const
        { return 1; }
        const std::string getLibName() const
        { return std::string("TerrainPlugin"); }
        CREATE_MODULE_INFO();

        // MarsPlugin methods
        void init();
        void reset();
        void update(mars::interfaces::sReal time_ms);

        // TerrainPlugin methods

      private:
        osg::ref_ptr<osg_terrain::Terrain> terrain;
        osg::ref_ptr<osg::Group> scene;
        osg_material_manager::OsgMaterialManager *materialManager;
      }; // end of class definition TerrainPlugin

    } // end of namespace TerrainPlugin
  } // end of namespace plugin
} // end of namespace mars

#endif // MARS_PLUGINS_TERRAINPLUGIN_H
