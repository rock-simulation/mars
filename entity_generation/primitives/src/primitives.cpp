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

#include "primitives.h"

#include <mars/data_broker/DataBrokerInterface.h>

#include <mars/interfaces/sim/EntityManagerInterface.h>
#include <mars/interfaces/sim/SimulatorInterface.h>
#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/sim/JointManagerInterface.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>
#include <mars/sim/SimEntity.h>
#include <mars/entity_generation/entity_factory/EntityFactoryManager.h>
#include <mars/interfaces/Logging.hpp>

#include <mars/utils/misc.h>
#include <mars/utils/mathUtils.h>

namespace mars {
  namespace smurf {

    using namespace std;
    using namespace interfaces;
    using namespace utils;
    using namespace configmaps;

    PRIMITIVES::PRIMITIVES(lib_manager::LibManager *theManager): MarsPluginTemplate(theManager, "PRIMITIVES"),
                                                       entity_generation::EntityFactoryInterface("primitive") {
      entity_generation::EntityFactoryManager* factoryManager =
        theManager->acquireLibraryAs<mars::entity_generation::EntityFactoryManager>(
              "mars_entity_factory");
      factoryManager->registerFactory("primitive", this);
      theManager->releaseLibrary("mars_entity_factory");
    }

    PRIMITIVES::~PRIMITIVES(){
    }

    void PRIMITIVES::init() {
      control->sim->switchPluginUpdateMode(0, this);

      reset();
    }

    void PRIMITIVES::reset() {
      primitivename = "";
      entity = NULL;
    }

    void PRIMITIVES::update(sReal time_ms) {
    }

    void PRIMITIVES::handleURI(ConfigMap *map, std::string uri) {
      ConfigMap map2 = ConfigMap::fromYamlFile(uri);
      handleURIs(&map2);
      map->append(map2);
    }

    void PRIMITIVES::handleURIs(ConfigMap *map) {
      if (map->find("URI") != map->end()) {
        std::string file = (std::string) (*map)["URI"][0];
        if (!file.empty() && file[0] != '/') {
          file = tmpPath + file;
        }
        handleURI(map, file);
      }
      if (map->find("URIs") != map->end()) {
        ConfigVector::iterator vIt = (*map)["URIs"].begin();
        for (; vIt != (*map)["URIs"].end(); ++vIt) {
          std::string file = (std::string) (*vIt);
          if (!file.empty() && file[0] != '/') {
            file = tmpPath + file;
          }
          handleURI(map, file);
        }
      }
    }

    sim::SimEntity* PRIMITIVES::createEntity(const ConfigMap& config) {
      reset();
      configmaps::ConfigMap entityconfig;
      entityconfig = config;
      std::string path = (std::string)entityconfig["path"];

      // node mapping and name checking
      std::string primitivename = (std::string)entityconfig["name"];
      if (primitivename == "")
        primitivename = "unnamed_primitive";
      entityconfig["name"] = primitivename;
      entity = new sim::SimEntity(control, entityconfig);
    // loadNode
      configmaps::ConfigMap map;
      if (entityconfig.hasKey("geometry") && entityconfig["geometry"].hasKey("type")) {
        std::string type = entityconfig["geometry"]["type"];
        map["filename"] = "PRIMITIVE";
        if (type == "plane") {
          map["name"] = primitivename;
          map["physicmode"] = "plane";
          map["origname"] = "plane";
          map["extend"]["x"] = 10.;
          map["extend"]["y"] = 10.;
        } else
        if (type == "box") {
          map["name"] = primitivename;
          map["physicmode"] = "box";
          map["origname"] = "box";
          map["extend"]["x"] = 1.;
          map["extend"]["y"] = 1.;
          map["extend"]["z"] = 1.;
          map["position"]["z"] = 0.5;
          map["movable"] = true;
        } else
        if (type == "sphere") {
          map["name"] = primitivename;
          map["physicmode"] = "sphere";
          map["visualType"] = "sphere";
          map["extend"]["x"] = 0.5;
          map["extend"]["y"] = 1.;
          map["extend"]["z"] = 1.;
          map["position"]["z"] = 0.5;
          map["movable"] = true;
        } else
        if (type == "cylinder") {

        } else
        if (type == "capsule") {

        }
      }
      map.append(entityconfig);

      NodeData node;
      int valid = node.fromConfigMap(&map, tmpPath, control->loadCenter);
      if (!valid)
        return NULL;

      // loadMaterial
      configmaps::ConfigMap material;
      material["name"] = "defaultGrey";
      material["diffuseColor"]["a"] = 1.0;
      material["diffuseColor"]["r"] = 0.33;
      material["diffuseColor"]["g"] = 0.39;
      material["diffuseColor"]["b"] = 0.5;
      material["specularColor"]["a"] = 1.0;
      material["specularColor"]["r"] = 0.5;
      material["specularColor"]["g"] = 0.5;
      material["specularColor"]["b"] = 0.5;
      material["ambientColor"]["a"] = 1.0;
      material["ambientColor"]["r"] = 0.53;
      material["ambientColor"]["g"] = 0.59;
      material["ambientColor"]["b"] = 0.7;
      material["shininess"] = 80.0;
      if (entityconfig.hasKey("material")) {
        material.append(entityconfig["material"]);
      }
      node.material.fromConfigMap(&material, tmpPath);

      // check if meshes are stored as `.stl` file
      string suffix = getFilenameSuffix(node.filename);
      if (suffix == ".stl" || suffix == ".STL") {
        // add an additional rotation of -90.0 degree due to wrong definition
        // of which direction is up within .stl (for .stl -Y is up and in MARS
        // Z is up)
        node.visual_offset_rot *= eulerToQuaternion(Vector(-90.0, 0.0, 0.0));
      }
      std::string parentname;
      if (entityconfig.hasKey("parent")) {
          parentname = (std::string)entityconfig["parent"];
            if (parentname == "world") {
               node.movable = false;
               node.groupID = 666;
            }
        }
      NodeId oldId = node.index;
      NodeId newId = control->nodes->addNode(&node);
      if (!newId) {
        LOG_ERROR("addNode returned 0");
        return 0;
      }
      entity->addNode(node.index, node.name);

      /*if(control->loadCenter->getMappedSceneByName(robotname) == 0) {
        control->loadCenter->setMappedSceneName(robotname);
      }
      mapIndex = control->loadCenter->getMappedSceneByName(robotname);*/
      return entity;
    }

  }  // end of namespace smurf
}  // end of namespace mars

DESTROY_LIB(mars::smurf::PRIMITIVES);
CREATE_LIB(mars::smurf::PRIMITIVES);
