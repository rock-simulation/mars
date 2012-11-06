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

/*
 *  RayGridSensor.cpp
 *
 *  Created by Daniel Bessler
 *
 */

#include <cstdlib>
#include <cstdio>

#include "RayGridSensor.h"

#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>

#include <mars/data_broker/DataBrokerInterface.h>

#include <cmath>

namespace mars {
  namespace sim {

    using namespace utils;
    using namespace interfaces;

    RayGridSensor::RayGridSensor(ControlCenter *control,
                                 const unsigned long id,
                                 const std::string &name):
      SensorInterface(control),
      BaseGridIntersectionSensor(id,name,640,480,1,1)
    {
#warning SET size of sensor
#warning also other work needed
#if 0
      std::vector<unsigned long>::iterator iter;
      std::string groupName, dataName;
      draw_item item;
    
      for(int i = 0; i < 3; ++i)
        positionIndices[i] = -1;
      for(int i = 0; i < 4; ++i)
        rotationIndices[i] = -1;

      this->sSensor = *sSensor;
      this->control = control;
      // create a sensor_config struct
      this->node_id = sSensor->indices[0];
      this->s_cfg = *(sensor_config*)(sSensor->data);
    
      this->s_cfg.resolution = this->s_cfg.resx * this->s_cfg.resy;
    
      /* set sensor start postions relative to the sensor center */
      s_cfg.sensorPositions = NULL;
      updateSensorPositions();
    
      /* add sensors to physics engine */
      control->nodes->addNodeSensor(this->node_id, this->s_cfg);
      control->nodes->getDataBrokerNames(this->node_id, &groupName, &dataName);
      control->dataBroker->registerTimedReceiver(this, groupName, dataName,"mars_sim/simTimer",sSensor->rate);
    
      /* add draw items */
      draw.ptr_draw = (garphics::DrawInterface*)this;

      updateDrawItems();
    
      this->rot = Quaternion();
#endif
    }

    RayGridSensor::~RayGridSensor(void)
    {
#if 0
      control->dataBroker->unregisterTimedReceiver(this, "*", "*", 
                                                   "mars_sim/simTimer");
      if (s_cfg.sensorPositions)
        free(s_cfg.sensorPositions);
      control->graphics->removeDrawItems(this);
      if(s_cfg.data) free(s_cfg.data);
      //     sim->removeNodeSensor(this->node_id);
#endif
    }

    int RayGridSensor::getMonsterData(char* data) const
    {
#if 0
      char *p;
      int num_char = 0, i;
    
      p = data;
      for(i = 0; i < s_cfg.resolution; i++) {
        sprintf(p, " %6.2f", s_cfg.data[i]);
        p += 7;
        num_char += 7;
      }
      return num_char;
#endif
    }

    int RayGridSensor::getSensorData(sReal** data) const
    {
#if 0
      int i;
      *data = (sReal*)calloc(s_cfg.resolution, sizeof(sReal));
      for (i = 0; i < s_cfg.resolution; i++) {
        (*data)[i] = s_cfg.data[i];
      }
      return i;
#endif
    }

    void RayGridSensor::receiveData(const data_broker::DataInfo &info,
                                    const data_broker::DataPackage &package,
                                    int callbackParam) {
      CPP_UNUSED(info);
      CPP_UNUSED(callbackParam);
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
        package.get(positionIndices[i], &pos[i]);
      package.get(rotationIndices[0], &rot.x());
      package.get(rotationIndices[1], &rot.y());
      package.get(rotationIndices[2], &rot.z());
      package.get(rotationIndices[3], &rot.w());
    }

    void RayGridSensor::updateSensorPositions()
    {
#if 0
      double offsetX, offsetY;
    
      /* center node on the grid */
      offsetX = s_cfg.rowSpacing*(s_cfg.resx/2.0) -
        (s_cfg.resx%2 == 0 ? s_cfg.rowSpacing/2.0 : 0.0);
      offsetY = s_cfg.colSpacing*(s_cfg.resy/2.0) -
        (s_cfg.resy%2 == 0 ? s_cfg.colSpacing/2.0 : 0.0);
    
      if (s_cfg.sensorPositions)
        free(s_cfg.sensorPositions);
      s_cfg.sensorPositions = (Vector*) calloc(s_cfg.resolution, sizeof(Vector));
    
      for (int i = 0; i < s_cfg.resx; i++) {
        for (int j = 0; j < s_cfg.resy; j++) {
          int index = i + s_cfg.resx*j;
          s_cfg.sensorPositions[index].x() = s_cfg.rowSpacing*i - offsetX;
          s_cfg.sensorPositions[index].y() = s_cfg.colSpacing*j - offsetY;
          s_cfg.sensorPositions[index].z() = 0.0;
        }
      }
#endif
    }

    void RayGridSensor::updateDrawItems()
    {
#if 0
      draw_item item;
    
      control->graphics->removeDrawItems(this);
      item.id = 0;
      item.type = DRAW_LINE;
      item.draw_state = DRAW_STATE_CREATE;
      item.point_size = 1;
      item.myColor.r = 1;
      item.myColor.g = 0;
      item.myColor.b = 0;
      item.myColor.a = 1;
      item.label = "";
      draw.drawItems.clear();
      for (int i = 0; i < s_cfg.resx; i++) {
        for (int j = 0; j < s_cfg.resy; j++) {
          item.start = Vector(0,0,0);
          item.end = Vector(0,0,0);
          draw.drawItems.push_back(item);
        }
      }
      control->graphics->addDrawItems(&draw);
#endif
    }

    void RayGridSensor::update(std::vector<draw_item>* drawItems)
    {
#if 0
      Vector tmp, lpos;
    
      for (int i = 0; i < s_cfg.resolution; i++) {
        lpos = s_cfg.sensorPositions[i];
        //tmp = QVRotate(this->rot, lpos);
        tmp = (this->rot * lpos);
        (*drawItems)[i].draw_state = DRAW_STATE_UPDATE;
        (*drawItems)[i].start = Vector(
                                       tmp.x() + pos.x(),
                                       tmp.y() + pos.y(),
                                       tmp.z() + pos.z());
        lpos.z() = -s_cfg.data[i];
        //tmp = QVRotate(this->rot, lpos);
        tmp = (this->rot * lpos);
        (*drawItems)[i].end = Vector(
                                     tmp.x() + pos.x(),
                                     tmp.y() + pos.y(),
                                     tmp.z() + pos.z());
      }

#endif
    }

    void RayGridSensor::setMatrixSpacing(sReal rowSpacing, sReal columnSpacing)
    {
#if 0
      this->s_cfg.rowSpacing = rowSpacing;
      this->s_cfg.colSpacing = columnSpacing;
    
      /* reload sensor positions */
      updateSensorPositions();
      /* reload sensor elements */
      control->nodes->reloadNodeSensor(this->node_id, &(this->s_cfg));
#endif
    }

    void RayGridSensor::setLaserDistanceRange(sReal min, sReal max)
    {
#if 0
      //TODO: If max ==-1, there is no maximum limit; if min==-1, there is no minimum limit
      this->s_cfg.min_distance = min;
      this->s_cfg.max_distance = max;
    
      /* reload sensor elements */
      control->nodes->reloadNodeSensor(this->node_id, &(this->s_cfg));
#endif
    }

    void RayGridSensor::setMatrixDimensions(int numRows, int numCols)
    {
#if 0
      this->s_cfg.resx = numRows;
      this->s_cfg.resy = numCols;
      this->s_cfg.resolution = numRows*numCols;
    
      updateDrawItems();
      /* reload sensor positions */
      updateSensorPositions();
      /* reload sensor elements */
      control->nodes->reloadNodeSensor(this->node_id, &(this->s_cfg));
#endif
    }


    unsigned long RayGridSensor::getNodeID(void) const
    {
#if 0
      return node_id;
#endif
    }

    const Quaternion RayGridSensor::getRotation() const
    {
#if 0
      return rot;
#endif
    }

    const Vector RayGridSensor::getPosition() const
    {
#if 0
      return pos;
#endif
    }

#if 0
    const sensor_config RayGridSensor::getSensorConfig(void) const {
      return s_cfg;
    }

    GridSensorInterface* RayGridSensor::getGridSensor() const {
      return (GridSensorInterface*) this;
    }
#endif


  } // end of namespace sim
} // end of namespace mars
