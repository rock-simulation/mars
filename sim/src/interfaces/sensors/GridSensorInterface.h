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
 *  GridSensorInterface.h
 *  QTVersion
 *
 *  Created by Malte Römmermann
 *
 */

#ifndef GRIDSENSOR_INTERFACE_H
#define GRIDSENSOR_INTERFACE_H

#ifdef _PRINT_HEADER_
  #warning "GridSensorInterface.h"
#endif

#include <mars/base/sensor_bases.h>

namespace mars {
  namespace sim {

    class GridSensorInterface {
    public:
      virtual ~GridSensorInterface(void) {}
      virtual void setMatrixSpacing(base::sReal rowSpacing, base::sReal columnSpacing) = 0;
      virtual void setMatrixDimensions(int numRows, int numCols) = 0;
      virtual void setLaserDistanceRange(base::sReal min, base::sReal max) = 0;
      virtual unsigned long getNodeID() const = 0;
      virtual const base::Quaternion getRotation() const = 0;
      virtual const base::Vector getPosition() const = 0;
      //virtual const sensor_config getSensorConfig(void) const = 0;
    };

  } // end of namespace sim
} // end of namespace mars

#endif // GRIDSENSOR_INTERFACE_H
