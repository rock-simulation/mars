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
 *  RAYGRIDSENSOR.h
 *  QTVersion
 *
 *  Created by Daniel Bessler
 *
 */

#ifndef RAYGRIDSENSOR_H
#define RAYGRIDSENSOR_H

#ifdef _PRINT_HEADER_
#warning "RayGridSensor.h"
#endif

#include <mars/interfaces/sim/SensorInterface.h>
#include <mars/data_broker/ReceiverInterface.h>
#include <mars/interfaces/sensor_bases.h>
#include <mars/interfaces/graphics/draw_structs.h>

namespace mars {
  namespace sim {

    class RayGridSensor : public interfaces::SensorInterface,
                          public interfaces::BaseGridIntersectionSensor,
                          public data_broker::ReceiverInterface,
                          public interfaces::DrawInterface {
  
    public:
      //RayGridSensor(sensorStruct *sSensor, ControlCenter *control);
      RayGridSensor(interfaces::ControlCenter *control, const unsigned long id,
                    const std::string &name);
      ~RayGridSensor(void);
      virtual int getMonsterData(char* data) const;
      virtual int getSensorData(interfaces::sReal** data) const;
      virtual void receiveData(const data_broker::DataInfo &info,
                               const data_broker::DataPackage &package,
                               int callbackParam);

      virtual void update(std::vector<interfaces::draw_item>* drawItems);
  
      virtual void setMatrixSpacing(interfaces::sReal rowSpacing,
                                    interfaces::sReal columnSpacing);
      virtual void setMatrixDimensions(int numRows, int numCols);
      virtual void setLaserDistanceRange(interfaces::sReal min, interfaces::sReal max);
      virtual unsigned long getNodeID() const;
      virtual const utils::Quaternion getRotation() const;
      virtual const utils::Vector getPosition() const;
      //virtual const sensor_config getSensorConfig(void) const;
      //  sensor_config s_cfg;
      //virtual interfaces::GridSensorInterface* getGridSensor() const;

    private:
      unsigned long node_id;
      utils::Vector pos;
      utils::Quaternion rot;
      interfaces::drawStruct draw;
      long positionIndices[3];
      long rotationIndices[4];

      void updateSensorPositions();
      void updateDrawItems();
    };

  } // end of namespace sim
} // end of namespace mars

#endif
