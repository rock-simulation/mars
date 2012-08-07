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

#ifndef SAVELOADSTRUCTS_H_
#define SAVELOADSTRUCTS_H_

#include <mars/interfaces/sensor_bases.h>
#include <mars/interfaces/MaterialData.h>
#include <mars/interfaces/MotorData.h>
#include <mars/interfaces/NodeData.h>
#include <mars/interfaces/JointData.h>
#include <mars/interfaces/ControllerData.h>
#include <mars/interfaces/core_objects_exchange.h>
#include <mars/interfaces/LightData.h>
#include <mars/interfaces/GraphicData.h>
#include <mars/interfaces/terrainStruct.h>
#include <map>
#include <vector>
#include <string>

namespace mars {
  namespace scene_loader {

    typedef struct material_map {
      unsigned long material_id;
      interfaces::MaterialData material;
    }material_map;

    // and the save->save method
    struct saveStruct_t{
      std::vector<interfaces::LightData> v_lightList;
      std::vector<interfaces::JointData> v_jointList;
      std::vector<interfaces::MotorData> v_motorList;
      std::vector<const interfaces::BaseSensor*> v_sensorList;
      std::vector<interfaces::ControllerData> v_controllerList;
      std::vector<interfaces::NodeData> v_nodeList;
    };

  } // end of namespace scene_loader
} // end of namespace mars

#endif /*SAVELOADSTRUCTS_H_*/
