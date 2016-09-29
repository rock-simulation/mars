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
 * \file TerrainPlugin.cpp
 * \author Malte (malte.langosz@dfki.de)
 * \brief 
 *
 * Version 0.1
 */


#include "TerrainPlugin.h"
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>
#include <mars/utils/misc.h>

namespace mars {
  namespace plugin {
    namespace TerrainPlugin {

      using namespace mars::utils;
      using namespace mars::interfaces;

      TerrainPlugin::TerrainPlugin(lib_manager::LibManager *theManager)
        : MarsPluginTemplate(theManager, "TerrainPlugin"),
          materialManager(NULL) {
      }

      void TerrainPlugin::init() {
        if(!control->graphics) return;
        scene = static_cast<osg::Group*>(control->graphics->getScene2());

        materialManager = libManager->getLibraryAs<osg_material_manager::OsgMaterialManager>("osg_material_manager", true);
        terrain = new osg_terrain::Terrain(materialManager);
        scene->addChild(terrain.get());
        control->sim->switchPluginUpdateMode(PLUGIN_GUI_MODE, this);
      }

      void TerrainPlugin::reset() {
      }

      TerrainPlugin::~TerrainPlugin() {
        if(materialManager) libManager->releaseLibrary("osg_material_manager");
      }


      void TerrainPlugin::update(sReal time_ms) {
        cameraStruct cs;
        if(terrain.valid()) {
          control->graphics->getCameraInfo(&cs);
          terrain->setCameraPos(cs.pos[0], cs.pos[1], cs.pos[2]);
        }
        // control->motors->setMotorValue(id, value);
      }

    } // end of namespace TerrainPlugin
  } // end of namespace plugin
} // end of namespace mars

DESTROY_LIB(mars::plugin::TerrainPlugin::TerrainPlugin);
CREATE_LIB(mars::plugin::TerrainPlugin::TerrainPlugin);
