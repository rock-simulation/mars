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
#include <mars/utils/mathUtils.h>
#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/NodeData.h>
#include <mars/interfaces/MaterialData.h>
#include <mars/utils/Color.h>

namespace mars {
  namespace plugins {
    namespace obstacle_generator {

      using namespace mars::utils;
      using namespace mars::interfaces;

      ObstacleGenerator::ObstacleGenerator(lib_manager::LibManager *theManager)
        : MarsPluginTemplate(theManager, "ObstacleGenerator") {
        sigma = 0.001;
      }
  
      void ObstacleGenerator::init() {
        //create properties
        params["field_width"] = 2.0;
        params["field_length"] = 5.0;
        params["field_distance"] = 1.0;
        params["mean_obstacle_aspect_ratio"] = 3.0;
        params["std_obstacle_aspect_ratio"] = 1.0;
        params["min_obstacle_aspect_ratio"] = 1.3;
        params["max_obstacle_aspect_ratio"] = 5.0;
        params["mean_obstacle_height"] = 0.1;
        params["std_obstacle_height"] = 0.1;
        params["min_obstacle_height"] = 0.01;
        params["max_obstacle_height"] = 0.3;
        params["obstacle_number"] = 40.0;
        params["incline_angle"] = 0.0;
        params["ground_level"] = 0.0;
        textures["ground"] = "ground.jpg";
        textures["ground_bump"] = "";
        textures["ground_norm"] = "";
        textures["obstacle"] = "rocks.jpg";
        textures["obstacle_bump"] = "";
        textures["obstacle_norm"] = "";
        cfg_manager::cfgParamId id = 0;
        for (std::map<std::string, double>::iterator it = params.begin(); it != params.end(); ++it) {
          id = control->cfg->getOrCreateProperty("obstacle_generator", it->first, it->second, this).paramId;
          paramIds[id] = it->first;
        }
        for (std::map<std::string, std::string>::iterator it = textures.begin(); it != textures.end(); ++it) {
          id = control->cfg->getOrCreateProperty("obstacle_generator", it->first, it->second, this).paramId;
          paramIds[id] = it->first;
        }
        createObstacleField();
      }

      void ObstacleGenerator::reset() {
        clearObstacleField();
        paramIds.clear();
        params.clear();
        init();
      }

      void ObstacleGenerator::clearObstacleField() {
        for(std::vector<NodeId>::iterator it = oldNodeIDs.begin(); it != oldNodeIDs.end(); ++it) {
          control->nodes->removeNode(*it);
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

      double ObstacleGenerator::random_normal_number(double mean, double std, double min, double max) {
        //std::default_random_engine generator;
        //std::normal_real_distribution<double> normal_dis(mean, std);
        //return normal_dis(generator);
        return random_number(min, max);
      }

      double ObstacleGenerator::random_number(double min, double max) {
        // std::default_random_engine generator;
        // std::uni_distribution<double> uni_dis(min, max);
        // uni_dis(generator);
        // int n = 0;
        //   while ((height < params["min_obstacle_height"]) || (height > params["max_obstacle_height"])) {
        //       height = random_normal_number(params["mean_obstacle_height"], params["std_obstacle_height"],
        //         params["min_obstacle_height"], params["max_obstacle_height"]);
        //       ++n;
        //       if (n > 100) { //if there were too many unsuccessful trials; this prevents indefinite loops
        //         height = params["mean_obstacle_height"];                
        //         break;
        //       }
        //   }

        int r = rand() % 1000;
        return r / 1000.0 * (max-min) + min;        
      }

      void ObstacleGenerator::createObstacleField() {
        // create inclined box as slope if an angle is given
        if ((params["incline_angle"] > sigma) or (params["incline_angle"] < -sigma)) {
          Vector boxposition(params["field_distance"] + 0.5 * params["field_length"],
                0,
                params["ground_level"] - 0.5 + sin(degToRad(params["incline_angle"])) * 0.5 * params["field_length"]);
          fprintf(stderr, "%f / %f", sin(params["incline_angle"]), sin(degToRad(params["incline_angle"])));
          Vector boxsize(params["field_length"], params["field_width"], 1.0);
          Vector boxorientation(0, -params["incline_angle"], 0);
          //fprint(stderr, "f");
          NodeData box("incline", boxposition, eulerToQuaternion(boxorientation));
          box.initPrimitive(NODE_TYPE_BOX, boxsize, 1.0);
          box.material.texturename = textures["ground"];
          if (textures["ground_bump"] != "") {
            box.material.bumpmap = textures["ground_bump"];
          }
          if (textures["ground_norm"] != "") {
            box.material.normalmap = textures["ground_norm"];
          }
          box.material.diffuseFront = Color(1.0, 1.0, 1.0, 1.0);
          control->nodes->addNode(&box);
          oldNodeIDs.push_back(box.index);
          // control->nodes->createPrimitiveNode("incline", NODE_TYPE_BOX,
                                         // false, boxposition, boxsize, 1.0, eulerToQuaternion(boxorientation), false));
        }
        // create obstacles
        int n = static_cast<int>(params["obstacle_number"]);
        for (int i = 0; i < n; i++) {
          std::string name = "obstacle_";
          std::string numstring = "";
          std::istringstream(numstring) >> i;
          name.append(numstring);

          //init variables
          double pos_x=0, pos_y=0, pos_z=params["ground_level"], radius=0, height=0;

          //create position
          pos_x = params["field_distance"] + random_number(0, params["field_length"]);
          pos_y = random_number(-0.5 * params["field_width"], 0.5 * params["field_width"]);
          if ((params["incline_angle"] > sigma) or (params["incline_angle"] < -sigma)) {
            pos_z += sin(degToRad(params["incline_angle"])) * (pos_x - params["field_distance"]);
          }
          //create size
          height = random_normal_number(params["mean_obstacle_height"], params["std_obstacle_height"],
                params["min_obstacle_height"], params["max_obstacle_height"]);
          radius = height / random_normal_number(params["mean_obstacle_aspect_ratio"], params["std_obstacle_aspect_ratio"],
                  params["min_obstacle_aspect_ratio"], params["max_obstacle_aspect_ratio"]);
          if (height<radius) {height=radius+sigma;}
          Vector position(pos_x, pos_y, pos_z);
          Vector size(radius, height-radius, 0);
          Quaternion orientation(1.0, 0.0, 0.0, 0.0);
          NodeData obstacle(name, position, orientation);
          obstacle.initPrimitive(NODE_TYPE_CAPSULE, size, 1.0);
          obstacle.material.texturename = textures["obstacle"];
          obstacle.material.tex_scale = 5.0;
          obstacle.material.diffuseFront = Color(1.0, 1.0, 1.0, 1.0);
          if (textures["obstacle_bump"] != "") {
            obstacle.material.bumpmap = textures["obstacle_bump"];
          }
          if (textures["obstacle_bump"] != "") {
             obstacle.material.normalmap = textures["obstacle_norm"];
          }
          control->nodes->addNode(&obstacle);
          oldNodeIDs.push_back(obstacle.index);

        }
      }

      ObstacleGenerator::~ObstacleGenerator() {
        // params.clear();
        // paramIDs.clear();
        // oldNodeIDs.clear();
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
        if (_property.propertyType == cfg_manager::doubleProperty) {
          params[paramIds[_property.paramId]] = _property.dValue;
        }
        else {
         textures[paramIds[_property.paramId]] = _property.sValue; 
        }
        clearObstacleField();
        createObstacleField();
      }

    } // end of namespace obstacle_generator
  } // end of namespace plugins
} // end of namespace mars

DESTROY_LIB(mars::plugins::obstacle_generator::ObstacleGenerator);
CREATE_LIB(mars::plugins::obstacle_generator::ObstacleGenerator);
