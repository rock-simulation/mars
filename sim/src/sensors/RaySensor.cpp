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

/*
 *  RaySensor.cpp
 *  QTVersion
 *
 *  Created by Malte Römmerann
 *
 */

#include "RaySensor.h"

#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/sim/SimulatorInterface.h>
#include <mars/interfaces/sim/LoadCenter.h>

#include <mars/data_broker/DataBrokerInterface.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace mars {
  namespace sim {

    using namespace utils;
    using namespace configmaps;
    using namespace interfaces;

    BaseSensor* RaySensor::instanciate(ControlCenter *control, BaseConfig *config ){
      RayConfig *cfg = dynamic_cast<RayConfig*>(config);
      assert(cfg);
      return new RaySensor(control,*cfg);
    }

    RaySensor::RaySensor(ControlCenter *control, RayConfig config): 
      BasePolarIntersectionSensor(config.id, config.name, config.width,
                                  config.height, config.opening_width,
                                  config.opening_height),
      SensorInterface(control), config(config) {

      updateRate = config.updateRate;
      orientation.setIdentity();
      maxDistance = config.maxDistance;
      this->attached_node = config.attached_node;

      std::string groupName, dataName;
      drawStruct draw;
      draw_item item;
      int i;
      Vector tmp;
      have_update = false;
      for(int i = 0; i < 3; ++i)
        positionIndices[i] = -1;
      for(int i = 0; i < 4; ++i)
        rotationIndices[i] = -1;

      control->nodes->addNodeSensor(this);
      bool erg = control->nodes->getDataBrokerNames(attached_node, &groupName, &dataName);
      (void)erg;
      assert(erg);
      if(control->dataBroker->registerTimedReceiver(this, groupName, dataName,"mars_sim/simTimer",updateRate)) {
      }

      position = control->nodes->getPosition(attached_node);
      orientation = control->nodes->getRotation(attached_node);

      //Drawing Stuff
      if(config.draw_rays) {
        draw.ptr_draw = (DrawInterface*)this;
        item.id = 0;
        item.type = DRAW_LINE;
        item.draw_state = DRAW_STATE_CREATE;
        item.point_size = 1;
        item.myColor.r = 1;
        item.myColor.g = 0;
        item.myColor.b = 0;
        item.myColor.a = 1;
        item.texture = "";
        item.t_width = item.t_height = 0;
        item.get_light = 0.0;
          
    
        double rad_steps = getCols(); //rad_angle/(sReal)(sensor.resolution-1);
        double rad_start = -((rad_steps-1)/2.0)*stepX; //Starting to Left, because 0 is in front and rock convention posive CCW //(M_PI-rad_angle)/2;
        if(rad_steps == 1){
          rad_start = 0;
        }

        //printf("Rad Start: %f, rad_steps: %f, stepX: %f\n",rad_start,rad_steps,stepX);

        for(i=0; i<rad_steps; i++){
          tmp = Vector(cos(rad_start+i*stepX),
                       sin(rad_start+i*stepX), 0);
          directions.push_back(tmp);
          tmp = (orientation * tmp);
          item.start = position;
          item.end = (orientation * tmp);
          item.end *= data[i];
          draw.drawItems.push_back(item); // TEST
        }
    
        if(control->graphics)
          control->graphics->addDrawItems(&draw);
        
    
        assert(rad_steps == data.size());
      }
    }

    RaySensor::~RaySensor(void) {
      if(control->graphics)
        control->graphics->removeDrawItems((DrawInterface*)this);
      control->dataBroker->unregisterTimedReceiver(this, "*", "*", 
                                                   "mars_sim/simTimer");
    }

    std::vector<double> RaySensor::getSensorData() const {
      std::vector<double> result;
      result.resize(data.size());
      for(unsigned int i=0; i<data.size(); i++) {
        result[i] = data[i];
      }
      return result;
    }

    int RaySensor::getSensorData(double **data_) const {
      *data_ = (double*)malloc(data.size()*sizeof(double));
      for(unsigned int i=0; i<data.size(); i++) {
        (*data_)[i] = data[i];
      }
      return data.size();
    }

    void RaySensor::receiveData(const data_broker::DataInfo &info,
                                const data_broker::DataPackage &package,
                                int callbackParam) {
      CPP_UNUSED(info);
      CPP_UNUSED(callbackParam);
      long id;
      package.get(0, &id);

      if(positionIndices[0] == -1) {
        positionIndices[0] = package.getIndexByName("position/x");
        positionIndices[1] = package.getIndexByName("position/y");
        positionIndices[2] = package.getIndexByName("position/z");
        rotationIndices[0] = package.getIndexByName("rotation/x");
        rotationIndices[1] = package.getIndexByName("rotation/y");
        rotationIndices[2] = package.getIndexByName("rotation/z");
        rotationIndices[3] = package.getIndexByName("rotation/w");
      }
      for(int i = 0; i < 3; ++i)
      {
        package.get(positionIndices[i], &position[i]);
      }
      
      package.get(rotationIndices[0], &orientation.x());
      package.get(rotationIndices[1], &orientation.y());
      package.get(rotationIndices[2], &orientation.z());
      package.get(rotationIndices[3], &orientation.w());
  
      have_update = true;
    }

    void RaySensor::update(std::vector<draw_item>* drawItems) {
      unsigned int i;
      if(config.draw_rays) {
        if(have_update) {
          control->nodes->updateRay(attached_node);
          have_update = false;
        }
    
    
        if(!(*drawItems)[0].draw_state) {
          for(i=0; i<data.size(); i++) {
            (*drawItems)[i].draw_state = DRAW_STATE_UPDATE;
            (*drawItems)[i].start = position;
            (*drawItems)[i].end = (orientation * directions[i]);
            (*drawItems)[i].end *= data[i];
            
            (*drawItems)[i].end += (*drawItems)[i].start;
          }
        }
      }
    }

    BaseConfig* RaySensor::parseConfig(ControlCenter *control,
                                       ConfigMap *config) {
      RayConfig *cfg = new RayConfig;
      unsigned int mapIndex = (*config)["mapIndex"];
      unsigned long attachedNodeID = (*config)["attached_node"];
      if(mapIndex) {
        attachedNodeID = control->loadCenter->getMappedID(attachedNodeID,
                                                          interfaces::MAP_TYPE_NODE,
                                                          mapIndex);
      }

      ConfigMap::iterator it;
      if((it = config->find("width")) != config->end())
        cfg->width = it->second;
      if((it = config->find("opening_width")) != config->end())
        cfg->opening_width = it->second;
      if((it = config->find("max_distance")) != config->end())
        cfg->maxDistance = it->second;
      if((it = config->find("draw_rays")) != config->end())
        cfg->draw_rays = it->second;
      if((it = config->find("rate")) != config->end())
        cfg->updateRate = it->second;

      cfg->attached_node = attachedNodeID;
#warning Parse stepX stepY cols and rows
      /*
        ConfigMap::iterator it;

        if((it = config->find("stepX")) != config->end())
        cfg->stepX = it->second[0].getDouble();

        if((it = config->find("stepY")) != config->end())
        cfg->stepY = it->second[0].getDouble();

        if((it = config->find("cols")) != config->end())
        cfg->cols = it->second[0].getUInt();

        if((it = config->find("rows")) != config->end())
        cfg->rows = it->second[0].getUInt();
      */
      return cfg;
    }

    ConfigMap RaySensor::createConfig() const {
      ConfigMap cfg;
      cfg["name"] = config.name;
      cfg["id"] = config.id;
      cfg["type"] = "RaySensor";
      cfg["attached_node"] = config.attached_node;
      cfg["width"] = config.width;
      cfg["opening_width"] = config.opening_width;
      cfg["max_distance"] = config.maxDistance;
      cfg["draw_rays"] = config.draw_rays;
      cfg["rate"] = config.updateRate;
      /*
        cfg["stepX"] = config.stepX;
        cfg["stepY"] = config.stepY;
        cfg["cols"] = config.cols;
        cfg["rows"] = config.rows;
      */
      return cfg;
    }

    const RayConfig& RaySensor::getConfig() const {
      return config;
    }

  } // end of namespace sim
} // end of namespace mars
