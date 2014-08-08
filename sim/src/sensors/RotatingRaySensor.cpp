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
 *  Created by Stefan Haase, Kai von Szadkowski, Malte Langosz
 *
 */

#include "RotatingRaySensor.h"

#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/sim/SimulatorInterface.h>
#include <mars/interfaces/sim/LoadCenter.h>

#include <mars/data_broker/DataBrokerInterface.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>
#include <mars/cfg_manager/CFGManagerInterface.h>
#include <mars/utils/MutexLocker.h>

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
        BasePolarIntersectionSensor(config.id, 
                                    config.name, 
                                    config.bands*config.lasers,
                                    1, 
                                    config.opening_width,
                                    config.opening_height),
        SensorInterface(control), 
        config(config) {

      updateRate = config.updateRate;
      orientation.setIdentity();
      maxDistance = config.maxDistance;
      turning_offset = 0.0;
      full_scan = false;
      current_pose.setIdentity();
      
      num_points = 0;
      time_start = base::Time::now();
      
        /**
      double calc_ms = 0.0;
      control->cfg->getPropertyValue("Simulator", "calc_ms", "value", &calc_ms);
      std::cout << "calc ms " << calc_ms <<  std::endl;
    
      nsamples = (1000/fmax(updateRate, calc_ms));
      turning_step = (config.turning_speed*2*M_PI)/(nsamples*config.bands);
      */

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

      bool erg = control->nodes->getDataBrokerNames(attached_node, &groupName, &dataName);
      assert(erg);
      if(control->dataBroker->registerTimedReceiver(this, groupName, dataName,"mars_sim/simTimer",updateRate)) {
      }

      position = control->nodes->getPosition(attached_node);
      orientation = control->nodes->getRotation(attached_node);
      orientation_offset.setIdentity();
      
      // Fills the direction array.
      if(config.bands < 1) {
        std::cerr << "Number of bands too low("<< config.bands <<"),will be set to 1" << std::endl;
        config.bands = 1;
      }
      if(config.lasers < 1) {
        std::cerr << "Number of lasers too low("<< config.lasers <<"),will be set to 1" << std::endl;
        config.lasers = 1;
      }
      
      turning_start_fullscan = -config.opening_width / 2.0;
      turning_end_fullscan = -config.opening_width / 2.0 + config.opening_width / config.bands;
      turning_offset = turning_start_fullscan;
      
      std::cout << "Horizontal resolution: " << config.horizontal_resolution << std::endl;
      turning_step = config.horizontal_resolution; 
      double vAngle = 0.0;
      double hAngle = 0.0;

      vAngle = config.lasers <= 1 ? config.opening_height/2.0 : config.opening_height/(config.lasers-1);
      hAngle = config.bands <= 1 ? 0 : config.opening_width/config.bands;
      
      for(int b=0; b<config.bands; ++b) {
        for(int l=0; l<config.lasers; ++l){

            tmp = Eigen::AngleAxisd(b*hAngle - turning_start_fullscan, Eigen::Vector3d::UnitZ()) * 
            Eigen::AngleAxisd(l*vAngle + config.downtilt - config.opening_height/2.0, Eigen::Vector3d::UnitY()) *
                
                Vector(1,0,0);
            directions.push_back(tmp);
          
            // Add a drawing item for each ray regarding the initial sensor orientation.
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
              
              // Initial vector length is set to 1.0
              item.start = position;
              item.end = orientation * tmp;
              draw.drawItems.push_back(item);
            }
        }
      }
      
      // Add sensor after everything has been initialized.
      control->nodes->addNodeSensor(this);
      
      // GraphicsManager crashes if default constructor drawStruct is passed.
      if(config.draw_rays) {
        if(control->graphics) {
          control->graphics->addDrawItems(&draw);
        }
      }
      assert(N == data.size());
      
    }

    RotatingRaySensor::~RotatingRaySensor(void) {
      control->graphics->removeDrawItems((DrawInterface*)this);
      control->dataBroker->unregisterTimedReceiver(this, "*", "*", "mars_sim/simTimer");
    }

    std::vector<utils::Vector> RotatingRaySensor::getPointcloud() {
      mars::utils::MutexLocker lock(&mutex_pointcloud);
      full_scan = false;
      base::Time time_now = base::Time::now();
      std::cout << "Points/sec " << num_points / (time_now - time_start).toSeconds() << std::endl;
      return pointcloud_full;
    }

    int RotatingRaySensor::getSensorData(double** data_) const {
      mars::utils::MutexLocker lock(&mutex_pointcloud);
      *data_ = (double*)malloc(pointcloud_full.size()*3*sizeof(double));
      for(unsigned int i=0; i<pointcloud_full.size(); i++) {
        if((pointcloud_full[i]).norm() <= config.maxDistance) {
            int array_pos = i*3;
            (*data_)[array_pos] = (pointcloud_full[i])[0];
            (*data_)[array_pos+1] = (pointcloud_full[i])[1];
            (*data_)[array_pos+2] = (pointcloud_full[i])[2];
        }
      }
      return pointcloud.size();
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
      
      Eigen::Vector3d position;
      package.get(positionIndices[0], &position.x());
      package.get(positionIndices[1], &position.y());
      package.get(positionIndices[2], &position.z());
      package.get(rotationIndices[0], &orientation.x());
      package.get(rotationIndices[1], &orientation.y());
      package.get(rotationIndices[2], &orientation.z());
      package.get(rotationIndices[3], &orientation.w());
      
      current_pose.setIdentity();
      current_pose.rotate(orientation);
      current_pose.translation() = position;

      // Counts all rays which have been added while full_scan == true.
      int overhead = 0;
      // Fills the pointcloud vector with (dist_m, x, y, z).
      // data[] contains all the measured distances.
      for(unsigned int i=0; i<data.size(); i++) {
        if (data[i] < config.maxDistance) {
          // Calculates the ray/vector within the sensor frame.
          utils::Vector local_ray = orientation_offset * directions[i] * data[i];
          // Gathers pointcloud in the world frame to prevent/reduce movement distortion.
          // This necessitates a back-transformation (world2node) in getPointcloud().
          utils::Vector tmpvec = current_pose * local_ray;
          pointcloud.push_back(tmpvec); // Scale normalized vector.
        }
      }
     num_points += data.size();
      
      have_update = true;
    }

    void RotatingRaySensor::update(std::vector<draw_item>* drawItems) {
      
      unsigned int i;
   
        std::cout << "In update draw rays" << std::endl;
        if(have_update) {
          control->nodes->updateRay(attached_node);
          have_update = false;
        }
       if(config.draw_rays) {
        if(!(*drawItems)[0].draw_state) {
          for(i=0; i<data.size(); i++) {
              (*drawItems)[i].draw_state = DRAW_STATE_UPDATE;
              // Updates the rays using the current sensor pose. 
              (*drawItems)[i].start = position;
              (*drawItems)[i].end = (orientation * orientation_offset * directions[i]);
              (*drawItems)[i].end *= data[i];
              (*drawItems)[i].end += (*drawItems)[i].start;
          }
        }
      }
    }

    utils::Quaternion RotatingRaySensor::turn() {  
      
      // If the scan is full the pointcloud will be copied.
      mutex_pointcloud.lock();
      turning_offset += turning_step;
      if(turning_offset >= turning_end_fullscan) {
        // Copies current full pointcloud to pointcloud_full.
        std::list<utils::Vector>::iterator it = pointcloud.begin();
        pointcloud_full.resize(pointcloud.size());
        for(int i=0; it != pointcloud.end(); it++, i++) {
          // Transforms the pointcloud back from world to current node (see receiveDate()).
          pointcloud_full[i]= current_pose.inverse() * (*it);
        }
        pointcloud.clear();
        turning_offset = turning_start_fullscan;
        full_scan = true;
      }
      orientation_offset = utils::angleAxisToQuaternion(turning_offset, utils::Vector(0.0, 0.0, 1.0));
      mutex_pointcloud.unlock();
      
      return orientation_offset;
    }

    int RotatingRaySensor::getNumberRays() {
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
      if((it = config->find("horizontal_resolution")) != config->end())
        cfg->horizontal_resolution = it->second[0].getDouble();
      cfg->attached_node = attachedNodeID;

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
      cfg["horizontal_resolution"][0] = ConfigItem(config.horizontal_resolution);
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
