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

#ifndef MARS_CORE_TERRAIN_STRUCT_H
#define MARS_CORE_TERRAIN_STRUCT_H

#include "MaterialData.h"
#include <string>

namespace mars {

  namespace interfaces {

    /**
     * terrainStruct is a struct to exchange heightfields between the GUI and
     * the simulation
     */
    struct terrainStruct {
      terrainStruct()
        : width(0), height(0), 
          targetWidth(0), targetHeight(0), 
          scale(1.0), 
          texScale(0.1),
          pixelData(NULL),
          mesh(0) {}

      std::string name; //the joints name
      std::string srcname;
      MaterialData material;
      int width;
      int height;
      double targetWidth;
      double targetHeight;
      double scale;
      double texScale; // texture scaling - a value of 0 will fit the complete terrain
      double *pixelData;
      int mesh;

    }; // end of struct terrainStruct

  } // end of namespace interfaces

} // end of namespace mars

#endif
