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

#include "RotatingRaySensor.h"

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
    using namespace interfaces;

    BaseSensor* RotatingRaySensor::instanciate(ControlCenter *control, BaseConfig *config ){
      RotatingRayConfig *cfg = dynamic_cast<RotatingRayConfig*>(config);
      assert(cfg);
      return new RotatingRaySensor(control,*cfg);
    }

    RotatingRaySensor::RotatingRaySensor(ControlCenter *control, RotatingRayConfig config):
      BasePolarIntersectionSensor(config.id, config.name, config.width*config.height,
                                  1, config.opening_width,
                                  config.opening_height),
      SensorInterface(control), config(config) {

      updateRate = config.updateRate;
      orientation.setIdentity();
      maxDistance = config.maxDistance;
      turning_offset = 0.0;

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
      assert(erg);
      if(control->dataBroker->registerTimedReceiver(this, groupName, dataName,"mars_sim/simTimer",updateRate)) {
      }

      position = control->nodes->getPosition(attached_node);
      orientation = control->nodes->getRotation(attached_node);
      orientation_offset = orientation;

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

        // now calculate the directions in which the rays point
        int N = config.width * config.height;
        double hAngle = 2*M_PI/N;
        double vAngle = config.opening_height/(config.height-1);
        double maxheight = config.opening_height/2-config.downtilt;
        int vpos = 0;
        int inc = config.increment;
        //fprintf(stderr, "Placing rays...\n");
        for(i=0; i<N; i++){
            //fprintf(stderr, "%i: %f/%f/%f  vpos:%i, vAngle: %f, z = %f\n", i, cos(i*hAngle), sin(i*hAngle), sin(maxheight-(vpos*vAngle)), vpos, vAngle, maxheight-(vpos*vAngle));
            tmp = Vector(cos(i*hAngle), sin(i*hAngle), sin(maxheight)-sin(vpos*vAngle));
            directions.push_back(tmp);
            vpos += inc;
            if (vpos > config.height) {
                vpos %= config.height-1;
            }
            tmp = (orientation * tmp);
            item.start = position;
            item.end = (orientation * tmp);
            item.end *= data[i];
            draw.drawItems.push_back(item);
        }

    
        if(control->graphics)
          control->graphics->addDrawItems(&draw);
        
    
        assert(N == data.size());
      }
    }

    RotatingRaySensor::~RotatingRaySensor(void) {
      control->graphics->removeDrawItems((DrawInterface*)this);
      control->dataBroker->unregisterTimedReceiver(this, "*", "*", "mars_sim/simTimer");
    }

    std::vector<double> RotatingRaySensor::getSensorData() const {
      std::vector<double> result;
      result.resize(data.size());
      for(unsigned int i=0; i<data.size(); i++) {
        result[i] = data[i];
      }
      return result;
    }

    int RotatingRaySensor::getSensorData(double *data_) const {
      data_ = (double*)malloc(data.size()*4*sizeof(double));
      for(unsigned int i=0; i<data.size(); i+=4) {
        utils::Vector tmpvec = orientation_offset * orientation * directions[i];
        data_[i] = data[i];
        data_[i+1] = tmpvec.x();
        data_[i+2] = tmpvec.y();
        data_[i+3] = tmpvec.z();
      }
      return data.size();
    }

    void RotatingRaySensor::receiveData(const data_broker::DataInfo &info,
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

    void RotatingRaySensor::update(std::vector<draw_item>* drawItems) {
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
            (*drawItems)[i].end = (orientation_offset * orientation * directions[i]);
            (*drawItems)[i].end *= data[i];
            
            (*drawItems)[i].end += (*drawItems)[i].start;
          }
        }
      }
    }

    double RotatingRaySensor::turn() {
        turning_offset += config.turning_step;
        if (turning_offset > 360) {
            turning_offset -= 360;
        }
        //fprintf(stderr, "turning_offset: %f\n",turning_offset);
        double radoffset = turning_offset/180.0*M_PI;
        orientation_offset = utils::angleAxisToQuaternion(radoffset, utils::Vector(0.0, 0.0, 1.0));
        return radoffset;
    }

    int RotatingRaySensor::getNRays() {
        return config.width * config.height;
    }

    BaseConfig* RotatingRaySensor::parseConfig(ControlCenter *control,
                                       ConfigMap *config) {
      RotatingRayConfig *cfg = new RotatingRayConfig;
      unsigned int mapIndex = (*config)["mapIndex"][0].getUInt();
      unsigned long attachedNodeID = (*config)["attached_node"][0].getULong();
      if(mapIndex) {
        attachedNodeID = control->loadCenter->getMappedID(attachedNodeID,
                                                          interfaces::MAP_TYPE_NODE,
                                                          mapIndex);
      }

      ConfigMap::iterator it;
      if((it = config->find("width")) != config->end())
        cfg->width = it->second[0].getInt();
      if((it = config->find("height")) != config->end())
        cfg->height = it->second[0].getInt();
      if((it = config->find("opening_width")) != config->end())
        cfg->opening_width = it->second[0].getDouble();
      if((it = config->find("opening_height")) != config->end())
        cfg->opening_height = it->second[0].getDouble();
      if((it = config->find("max_distance")) != config->end())
        cfg->maxDistance = it->second[0].getDouble();
      if((it = config->find("draw_rays")) != config->end())
        cfg->draw_rays = it->second[0].getBool();
      if((it = config->find("downtilt")) != config->end())
        cfg->downtilt = it->second[0].getDouble();
      if((it = config->find("rate")) != config->end())
        cfg->updateRate = it->second[0].getULong();
      if((it = config->find("turning_step")) != config->end())
        cfg->turning_step = it->second[0].getDouble();

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

    ConfigMap RotatingRaySensor::createConfig() const {
      ConfigMap cfg;
      cfg["name"][0] = ConfigItem(config.name);
      cfg["id"][0] = ConfigItem(config.id);
      cfg["type"][0] = ConfigItem("RaySensor");
      cfg["attached_node"][0] = ConfigItem(config.attached_node);
      cfg["width"][0] = ConfigItem(config.width);
      cfg["height"][0] = ConfigItem(config.height);
      cfg["opening_width"][0] = ConfigItem(config.opening_width);
      cfg["opening_height"][0] = ConfigItem(config.opening_height);
      cfg["max_distance"][0] = ConfigItem(config.maxDistance);
      cfg["draw_rays"][0] = ConfigItem(config.draw_rays);
      cfg["downtilt"][0] = ConfigItem(config.downtilt);
      cfg["rate"][0] = ConfigItem(config.updateRate);
      cfg["turning_step"][0] = ConfigItem(config.turning_step);
      /*
        cfg["stepX"][0] = ConfigItem(config.stepX);
        cfg["stepY"][0] = ConfigItem(config.stepY);
        cfg["cols"][0] = ConfigItem(config.cols);
        cfg["rows"][0] = ConfigItem(config.rows);
      */
      return cfg;
    }

    const RotatingRayConfig& RotatingRaySensor::getConfig() const {
      return config;
    }

  } // end of namespace sim
} // end of namespace mars
