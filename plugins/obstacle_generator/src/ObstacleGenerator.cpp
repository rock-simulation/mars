/*
 *  Copyright 2013, DFKI GmbH Robotics Innovation Center
 *
 *  This file.is part of the MARS simulation framework.
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
 * \file.ObstacleGenerator.cpp
 * \author Kai von Szadkowski (kavo01@dfki.de)
 * \brief A
 *
 * Version 0.1
 */


#include "ObstacleGenerator.h"
#include <mars/data_broker/DataBrokerInterface.h>
#include <mars/data_broker/DataPackage.h>

namespace mars {
  namespace plugins {
    namespace obstacle_generator {

      using namespace mars::utils;
      using namespace mars::interfaces;

      ObstacleGenerator::ObstacleGenerator(lib_manager::LibManager *theManager)
        : MarsPluginTemplate(theManager, "ObstacleGenerator") {
        params["field_width"] = 1.0;
        params["field_length"] = 1.0;
        params["field_distance"] = 0.0;
        params["mean_obstacle_radius"] = 0.1;
        params["std_obstacle_radius"] = 0.1;
        params["min_obstacle_radius"] = 0.1;
        params["max_obstacle_radius"] = 0.1;
        params["mean_obstacle_height"] = 0.1;
        params["std_obstacle_height"] = 0.1;
        params["min_obstacle_height"] = 0.01;
        params["max_obstacle_height"] = 1.0;
        params["obstacle_number"] = 20.0;
        params["incline_angle"] = 0.0;
      }
  
      void ObstacleGenerator::init() {
        //create properties
        cfg_manager::cfgParamIdn id;
        for (it = params.begin(); it != params.end(); ++it) {
          id = control->cfg->getOrCreateProperty("obstacle_generator", it->first, it->second, this);
          paramIds[id, it->first];
        }
      }

      void ObstacleGenerator::clearObstacleField() {
        for(std::vector<int>::iterator it = oldNodeIDs.begin(); it != oldNOdeIDs.end(); ++it) {
          control->nodes->removeNode(it);
        }
        oldNodeIDs.clear();
      }



        // Load a scene file.
        // control->sim->loadScene("some_file.scn");

        // Register for node information:
        /*
          std::string groupName, dataName;
          control->nodes->getDataBrokerNames(id, &groupName, &dataName);
          control->dataBroker->registerTimedReceiver(this, groupName, dataName, "mars_sim/simTimer", 10, 0);
        */

        /* get or create cfg_param
           example = control->cfg->getOrCreateProperty("plugin", "example",
           0.0, this);
        */
        //control->cfg->loadConfig("obstacle_generator.yaml");


        // Create a nonphysical box:

        // Create a camera fixed on the box:

        // Create a HUD texture element:

        //gui->addGenericMenuAction("../ObstacleGenerator/entry", 1, this);

      }

      void ObstacleGenerator::createObstacleField() {
        int n = static_cast<int>(params["obstacle_number"]);
        if params["incline_angle"] > sigma {
          Vector boxposition = (params["field_distance"] + 0.5 * params["field_length"],
                0,
                -0.5 + degToRad(cos(params["incline_angle"])) * 0.5 * params["field_length"]));
          Vector boxsize = (params["field_length"], params["field_width"], 1.0);
          Vectpr boxorientation = (0, degToRad(params["incline_angle"]), 0);
          oldNodeIDs.push_back(control->nodes->createPrimitiveNode(name, NodeType::NODE_TYPE_BOX,
                                         false, boxposition, boxsize, 1.0, eulerToQuaternion(boxorientation), false));
        }
        for (int i = 0; i < n; i++) {
          std::string name = "obstacle_";
          std::string numstring = ""
          istringstream(numstring) >> i
          name.append(numstring)

          //init random generators and variables
          std::default_random_engine generator;
          std::uniform_real_distribution<double> uni_dis(0.0,1.0);
          std::normal_distribution<double> height_norm_dis(params["mean_obstacle_height"], params["std_obstacle_height"]);
          std::normal_distribution<double> radius_norm_dis(params["mean_obstacle_radius"], params["std_obstacle_radius"]);
          double pos_x=0, pos_y=0, pos_z=0, radius=0, height=0;

          //create position
          pos_x = params["field_distance"] + uni_dis(generator) * params["field_length"] - 0.5 * params["field_length"];
          pos_y = uni_dis(generator) * params["field_width"] - 0.5 * params["field_width"];
          //create size
          height = height_norm_dis(generator);
          radius = radius_norm_dis(generator);
          int n = 0;
          while ((height < params["min_obstacle_height"]) || (height > params["max_obstacle_height"])) {
              height = norm_dis(generator);
              ++n;
              if (n > 100) { //if there were too many unsuccessful trials; this prevents indefinite loops
                height = params["mean_obstacle_height"];                
                break;
              }
          }
          Vector position = (pos_x, pos_y, pos_z);
          Vector size = (radius, radius, height);
          Quaternion orientation = (1.0, 0.0, 0.0, 0.0);
          oldNodeIDs.push_back(control->nodes->createPrimitiveNode(name, NodeType::NODE_TYPE_CAPSULE,
                                         false, position, size, 1.0, orientation, false));
        }
      }

      ObstacleGenerator::~ObstacleGenerator() {
      }


      void ObstacleGenerator::update(sReal time_ms) {

        // control->motors->setMotorValue(id, value);
      }

      void ObstacleGenerator::receiveData(const data_broker::DataInfo& info,
                                    const data_broker::DataPackage& package,
                                    int id) {
        // package.get("force1/x", force);
      }
  
      void ObstacleGenerator::cfgUpdateProperty(cfg_manager::cfgPropertyStruct _property) {
        params[paramIds[_property.paramId]] = _property.dValue;
        clearObstacleField();
        createObstacleField();
      }

    } // end of namespace obstacle_generator
  } // end of namespace plugins
} // end of namespace mars

DESTROY_LIB(mars::plugins::obstacle_generator::ObstacleGenerator);
CREATE_LIB(mars::plugins::obstacle_generator::ObstacleGenerator);
