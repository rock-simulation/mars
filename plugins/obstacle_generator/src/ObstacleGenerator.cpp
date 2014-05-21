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
 * \author Kai (kavo01@dfki.de)
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
        params["field_height"] = 1.0;
        params["field_distance"] = 0.0;
        params["mean_obstacle_height"] = 0.1;
        params["std_obstacle_height"] = 0.1;
        params["min_obstacle_height"] = 0.01;
        params["max_obstacle_height"] = 1.0;
        params["obstacle_number"] = 20.0;
      }
  
      void ObstacleGenerator::init() {
        //create properties
        for (iter = params.begin(); iter != params.end(); ++iter) {
          control->cfg->getOrCreateProperty("obstacle_generator", params->first, params->second, this);
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

      void createObstacleField() {
        int n = static_cast<int>(params["obstacle_number"]);
        for (int i = 0; i < n; i++) {
          std::string name = "obstacle_";
          std::string numstring = ""
          istringstream(numstring) >> i
          name.append(numstring)
          std::normal_distribution<double> distribution(params["mean_obstacle_height"],2.0);
          mars::interfaces::utils::Vector position;
          mars::interfaces::utils::Vector size;
          mars::interfaces::utils::Quaternion orientation;
          control->nodes->createPrimitiveNode(name, mars::interfaces::NodeType::NODE_TYPE_CAPSULE,
                                         false, position, size, 1.0, orientation, false);
        }
      }

      void ObstacleGenerator::reset() {
        updateParameters();
        createObstacleField();
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

        if(_property.paramId == example.paramId) {
          example.dValue = _property.dValue;
        }
      }

    } // end of namespace obstacle_generator
  } // end of namespace plugins
} // end of namespace mars

DESTROY_LIB(mars::plugins::obstacle_generator::ObstacleGenerator);
CREATE_LIB(mars::plugins::obstacle_generator::ObstacleGenerator);
