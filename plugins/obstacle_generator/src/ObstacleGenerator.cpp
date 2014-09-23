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
        params["field_distance"] = 0.5;
        params["mean_obstacle_width"] = 0.12;
        params["mean_obstacle_length"] = 0.12;
        params["std_obstacle_width"] = 0.06;
        params["min_obstacle_width"] = 0.03;
        params["max_obstacle_width"] = 0.24;
        params["mean_obstacle_height"] = 0.12;
        params["std_obstacle_height"] = 0.06;
        params["min_obstacle_height"] = 0.05;
        params["max_obstacle_height"] = 0.24;
        params["obstacle_number"] = 100.0;
        params["incline_angle"] = 0.0;
        params["ground_level"] = 0.0;
        textures["ground"] = "moon_surface_small.jpg";
        textures["ground_bump"] = "";
        textures["ground_norm"] = "";
        textures["obstacle"] = "rocks_small.jpg";
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
        bool_params["support_platform"] = false;
        bool_params["use_boxes"] = false;
        bool_params["use_grid"] = false;
        bool_params["incline_obstacles"] = false;
        //bool_params["incline_obstacles"] = false;
        for (std::map<std::string, bool>::iterator it = bool_params.begin(); it != bool_params.end(); ++it) {
          id = control->cfg->getOrCreateProperty("obstacle_generator", it->first, it->second, this).paramId;
          bool_paramIds[id] = it->first;
        }
        createObstacleField();
      }

      void ObstacleGenerator::reset() {
        //clearObstacleField();
        //createObstacleField();

        for(std::vector<NodeId>::iterator it=oldNodeIDs.begin();
	    it!=oldNodeIDs.end(); ++it) {
          double pos_x=0, pos_y=0, pos_z=params["ground_level"];
          //create position
          pos_x = random_number(0, params["field_length"], 3);
          pos_y = random_number(-0.5 * params["field_width"], 3,
				0.5 * params["field_width"]);
          if ((params["incline_angle"] > sigma) or
	      (params["incline_angle"] < -sigma)) {
            pos_z += (params["ground_level"] +
		      sin(degToRad(params["incline_angle"])) * pos_x);
            pos_x *= cos(degToRad(params["incline_angle"]));
          }
          pos_x += params["field_distance"];
	  control->nodes->setPosition(*it, Vector(pos_x, pos_y, pos_z));
	}
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

      void ObstacleGenerator::createObstacleField() {
        // initial calculations
        double obstacle_length = params["mean_obstacle_length"];
        if (!bool_params["use_boxes"]) {obstacle_length = params["mean_obstacle_width"];}
        double field_length = params["field_length"];
        double field_width = params["field_width"];
        if (bool_params["use_grid"]) {
            field_length = obstacle_length * params["field_length"];
            field_width = params["mean_obstacle_width"] * params["field_width"];
        }

        // create inclined box as slope if an angle is given
        if (bool_params["support_platform"]) {
          double platform_length = 0, platform_x_position = 0;
          if ((params["incline_angle"] > sigma) or (params["incline_angle"] < -sigma)) {
            Vector boxposition(params["field_distance"] + 0.5 * cos(degToRad(params["incline_angle"])) * field_length + 0.5 * sin(degToRad(params["incline_angle"])),
                  0,
                  params["ground_level"] - 0.5 + sin(degToRad(params["incline_angle"])) * 0.5 * field_length + 0.5 * (1 - cos(degToRad(params["incline_angle"]))));
            Vector boxsize(field_length, field_width, 1.0);
            Vector boxorientation(0, -params["incline_angle"], 0);
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
            box.material.tex_scale = field_width/2.0;
            control->nodes->addNode(&box, false);
            oldNodeIDs.push_back(box.index);
            // control->nodes->createPrimitiveNode("incline", NODE_TYPE_BOX,
                                           // false, boxposition, boxsize, 1.0, eulerToQuaternion(boxorientation), false));
            platform_length = 2.0 + params["field_distance"];
            platform_x_position = -1.0 + 0.5 * params["field_distance"];
          }
          else { //if there is no inclined surface
            platform_length = 2.0 + params["field_distance"] + field_length;
            platform_x_position = -1.0 + 0.5 * (params["field_distance"] + field_length);
          }

          Vector platformsize(platform_length, field_width, 1.0);
          Vector platformposition(platform_x_position, 0.0, -0.5 + params["ground_level"]);
          Vector platformorientation(0.0, 0.0, 0.0);
          NodeData platform("platform", platformposition, eulerToQuaternion(platformorientation));
          platform.initPrimitive(NODE_TYPE_BOX, platformsize, 1.0);
          platform.material.texturename = textures["ground"];
          if (textures["ground_bump"] != "") {
            platform.material.bumpmap = textures["ground_bump"];
          }
          if (textures["ground_norm"] != "") {
            platform.material.normalmap = textures["ground_norm"];
          }
          platform.material.diffuseFront = Color(1.0, 1.0, 1.0, 1.0);
          //platform.material.tex_scale = field_width/2.0;
          control->nodes->addNode(&platform, false);
          oldNodeIDs.push_back(platform.index);
        }
        // create obstacles
        if (bool_params["use_grid"]) {
            params["obstacle_number"] = params["field_width"]*params["field_length"];
            for (int w = 0; w < params["field_width"]; w++) {
                for (int l = 0; l < params["field_length"]; l++) {
                    std::string name = "obstacle_";
                    std::string numstring = "";
                    std::istringstream(numstring) >> l >>  w;
                    name = name.append(numstring);

                    //create position
                    double pos_x = (0.5+l) * obstacle_length;
                    double pos_y = -0.5 * field_width + (0.5+w) * params["mean_obstacle_width"];

                    //create size
                    double height = random_normal_number(params["mean_obstacle_height"], params["std_obstacle_height"],
                          params["min_obstacle_height"], params["max_obstacle_height"]);

                    //create obstacle
                    createObstacle(name, pos_x, pos_y, params["mean_obstacle_width"], params["mean_obstacle_length"], height);
                    }
                }
        }
        else {
            int n = static_cast<int>(params["obstacle_number"]);
            for (int i = 0; i < n; i++) {
              std::string name = "obstacle_";
              std::string numstring = "";
              std::istringstream(numstring) >> i;
              name = name.append(numstring);

              //create position
              double pos_x = random_number(0, params["field_length"], 3);
              double pos_y = random_number(-0.5 * params["field_width"], 0.5 * params["field_width"], 3);

              //create size
              double height = random_normal_number(params["mean_obstacle_height"], params["std_obstacle_height"],
                    params["min_obstacle_height"], params["max_obstacle_height"]);
              double radius = random_normal_number(params["mean_obstacle_width"], params["std_obstacle_width"],
                      params["min_obstacle_width"], params["max_obstacle_width"]);

              //create obstacle
              createObstacle(name, pos_x, pos_y, radius, radius, height);
            }
        }
      }

      void ObstacleGenerator::createObstacle(std::string name, double pos_x, double pos_y, double width, double length, double height) {
          //TODO: if we have an inclination and the obstacles are inclined as well, they might not show the correct height in inclination space
          double pos_z=params["ground_level"];
          if (bool_params["use_boxes"]) {
            pos_z = 0.5 * height;
          }
          if ((params["incline_angle"] > sigma) or (params["incline_angle"] < -sigma)) {
              if (bool_params["use_grid"] && !bool_params["support_platform"]) {
                if (params["incline_angle"] > 0) {
                  height += tan(degToRad(params["incline_angle"])) * pos_x;
                } else {
                  double field_length = length * params["field_length"];
                  height -= tan(degToRad(params["incline_angle"])) * (field_length-pos_x);
                  pos_z += tan(degToRad(params["incline_angle"])) * field_length;
                }
              } else {
                pos_z += sin(degToRad(params["incline_angle"])) * pos_x;
                pos_x *= cos(degToRad(params["incline_angle"]));
              }
              pos_z -= tan(degToRad(params["incline_angle"])) * 0.5 * length;
          }
           pos_x += params["field_distance"];
           Vector position(pos_x, pos_y, pos_z);
           Vector size(0, 0, 0);
           if (bool_params["use_boxes"]) {
               size[0] = length;
               size[1] = width;
               size[2] = height;
           } else {
               size[0] = width / 2.0;
               if (height<width) {
                   size[1] = sigma;
               } else {
                   size[1] = height-width;
               }
           }
           Quaternion orientation(1.0, 0.0, 0.0, 0.0);
           if (bool_params["incline_obstacles"]) {
               Vector incline(0, -params["incline_angle"], 0);
               orientation = eulerToQuaternion(incline);
           }
           NodeData obstacle(name, position, orientation);
           if (bool_params["use_boxes"]) {
               obstacle.initPrimitive(NODE_TYPE_BOX, size, 1.0);
           }
           else {
               obstacle.initPrimitive(NODE_TYPE_CAPSULE, size, 1.0);
           }
           obstacle.material.texturename = textures["obstacle"];
           obstacle.material.diffuseFront = Color(1.0, 1.0, 1.0, 1.0);
           if (textures["obstacle_bump"] != "") {
             obstacle.material.bumpmap = textures["obstacle_bump"];
           }
           if (textures["obstacle_bump"] != "") {
              obstacle.material.normalmap = textures["obstacle_norm"];
           }
           control->nodes->addNode(&obstacle, false);
           oldNodeIDs.push_back(obstacle.index);
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
        switch (_property.propertyType) {
          case cfg_manager::doubleProperty:
            params[paramIds[_property.paramId]] = _property.dValue;
            break;
          case cfg_manager::stringProperty:
            textures[paramIds[_property.paramId]] = _property.sValue;
            break;
          case cfg_manager::boolProperty:
            bool_params[bool_paramIds[_property.paramId]] = _property.bValue;
            break;
        }
        clearObstacleField();
        createObstacleField();
      }

    } // end of namespace obstacle_generator
  } // end of namespace plugins
} // end of namespace mars

DESTROY_LIB(mars::plugins::obstacle_generator::ObstacleGenerator);
CREATE_LIB(mars::plugins::obstacle_generator::ObstacleGenerator);
