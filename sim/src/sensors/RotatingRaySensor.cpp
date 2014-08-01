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
 *  RotatingRaySensor.cpp
 *
 *  Created by Malte Langosz, Kai von Szadkowski
 *
 */

#include "RotatingRaySensor.h"

#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/sim/SimulatorInterface.h>
#include <mars/interfaces/sim/LoadCenter.h>

#include <mars/data_broker/DataBrokerInterface.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>
#include <mars/cfg_manager/CFGManagerInterface.h>

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
      BasePolarIntersectionSensor(config.id, config.name, config.bands*config.lasers,
                                  1, config.opening_width,
                                  config.opening_height),
      SensorInterface(control), config(config) {

      updateRate = config.updateRate;
      orientation.setIdentity();
      maxDistance = config.maxDistance;
      turning_offset = 0.0;
      double calc_ms = 0.0;
      control->cfg->getPropertyValue("Simulator", "calc_ms", "value", &calc_ms);
      nsamples = (1000/fmax(updateRate, calc_ms));
      if (config.turning_speed <= 0) {
          config.turning_speed = 1.0/config.bands;
      }
      turning_step = (config.turning_speed*2*M_PI)/nsamples;

      this->attached_node = config.attached_node;

      std::string groupName, dataName;
      drawStruct draw;
      draw_item item;
      Vector tmp;
      have_update = false;
      full_scan = false;
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

        /* The following places the lasers around the scanner as follows:
         * There are no two lasers on the same horizontal angle. Instead, each successive laser is
         * placed one horizontal angle step further, with the *increment* determining how many
         * vertical 'slots' are skipped. Increasing the *increment* up to about half the number of
         * vertical slots will lead to the laser being more spread out, further increase will
         * mirror the same patterns as reached before.
         */
        unsigned int N = getNRays();
        double hAngle = 2*M_PI/N;
        double vAngle = config.opening_height/(config.lasers-1);
        double maxheight = config.opening_height/2-config.downtilt;
        int vpos = 0;
        int inc = config.increment;
        for(unsigned int i=0; i<N; i++){
            tmp = Vector(cos(i*hAngle), sin(i*hAngle), sin(maxheight)-sin(vpos*vAngle));
            directions.push_back(tmp);
            vpos += inc;
            if (vpos > (config.lasers-1)) {
                vpos %= config.lasers-1;
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
      for(unsigned int i=0; i<data.size(); i++) {
        if(data[i] <= config.maxDistance) {
            utils::Vector tmpvec = orientation_offset * orientation * directions[i];
            result.push_back(data[i]);
            result.push_back(tmpvec.x());
            result.push_back(tmpvec.y());
            result.push_back(tmpvec.z());
        }
      }
      return result;
    }

    std::vector<double> RotatingRaySensor::getPointCloud() {
      std::vector<double> result;
      if (full_scan) {
        result.reserve(pointcloud.size());
        result.insert(result.end(),pointcloud.begin(),pointcloud.end());
        pointcloud.clear();
        full_scan = false;
      }
      return result;
    }

    int RotatingRaySensor::getSensorData(double** data_) const {
      *data_ = (double*)malloc(data.size()*4*sizeof(double));
      int counter = 0;
      for(unsigned int i=0; i<data.size(); i+=4) {
        if(data[i] <= config.maxDistance) {
            utils::Vector tmpvec = orientation_offset * orientation * directions[i];
            (*data_)[i] = data[i];
            (*data_)[i+1] = tmpvec.x();
            (*data_)[i+2] = tmpvec.y();
            (*data_)[i+3] = tmpvec.z();
            counter++;
        }
      }
      return counter;
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

      // Fills the pointcloud vector with (dist_m, x, y, z).
      for(unsigned int i=0; i<data.size(); i+=4) {
        if (data[i] < config.maxDistance) {
          fprintf(stderr, "data[i]: %f, maxDistance: %f", data[i], config.maxDistance);
          utils::Vector tmpvec = orientation_offset * orientation * directions[i];
          pointcloud.push_back(data[i]);
          pointcloud.push_back(tmpvec.x());
          pointcloud.push_back(tmpvec.y());
          pointcloud.push_back(tmpvec.z());
        }
      }
      int overhead = pointcloud.size() - (nsamples*getNRays());
      if (overhead > 0) {
          std::list<double>::iterator it1,it2;
          it1 = it2 = pointcloud.begin();
          advance(it2,overhead-1);
          pointcloud.erase(it1, it2);
      }
      fprintf(stderr, "nsamples: %i, getNRays: %i\n", nsamples, getNRays());
      fprintf(stderr, "overhead: %i, nsamples*getNRays: %i\n", overhead, nsamples*getNRays());
      fprintf(stderr, "pointcloud.size: %i\n", (int)pointcloud.size());
  
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
        turning_offset += turning_step;
        if (turning_offset > (2*M_PI/config.bands)) {
            full_scan = true;
            turning_offset -= (2*M_PI/config.bands) - (turning_step/config.subresolution);
        }
        //fprintf(stderr, "turning_offset: %f, %f, %i\n",turning_offset,turning_step,config.subresolution);
        orientation_offset = utils::angleAxisToQuaternion(turning_offset, utils::Vector(0.0, 0.0, 1.0));
        return turning_offset;
    }

    int RotatingRaySensor::getNRays() {
        return config.bands * config.lasers;
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
      if((it = config->find("bands")) != config->end())
        cfg->bands = it->second[0].getInt();
      if((it = config->find("lasers")) != config->end())
        cfg->lasers = it->second[0].getInt();
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
      if((it = config->find("turning_speed")) != config->end())
        cfg->turning_speed = it->second[0].getDouble();
      if((it = config->find("increment")) != config->end())
              cfg->increment = it->second[0].getULong();
      if((it = config->find("subresolution")) != config->end())
              cfg->subresolution = it->second[0].getULong();

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
      cfg["bands"][0] = ConfigItem(config.bands);
      cfg["lasers"][0] = ConfigItem(config.lasers);
      cfg["opening_width"][0] = ConfigItem(config.opening_width);
      cfg["opening_height"][0] = ConfigItem(config.opening_height);
      cfg["max_distance"][0] = ConfigItem(config.maxDistance);
      cfg["draw_rays"][0] = ConfigItem(config.draw_rays);
      cfg["downtilt"][0] = ConfigItem(config.downtilt);
      cfg["rate"][0] = ConfigItem(config.updateRate);
      cfg["turning_speed"][0] = ConfigItem(config.turning_speed);
      cfg["increment"][0] = ConfigItem(config.increment);
      cfg["subresolution"][0] = ConfigItem(config.subresolution);
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
