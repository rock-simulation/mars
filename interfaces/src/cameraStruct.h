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

#ifndef MARS_INTERFACES_CAMERA_STRUCT_H
#define MARS_INTERFACES_CAMERA_STRUCT_H

#include <mars/utils/Vector.h>
#include <mars/utils/Quaternion.h>

namespace mars {

  namespace interfaces {

    /** 
     * this struct seems to be used to get information about a camera
     * which can be accessed through the sensor interface.
     */
    struct cameraStruct {
      utils::Vector pos;
      utils::Quaternion rot; //rotation

      /** 
       * camera intrinsic parameters
       * As defined in http://opencv.willowgarage.com/documentation/camera_calibration_and_3d_reconstruction.html
       *
       * u = scale_x * x + center_x
       * v = scale_y * y + center_y
       *
       * where u, v are pixel coordinates and x, y scene coordinates
       */ 
      double center_x, center_y;
      double scale_x, scale_y;
    }; // end of struct cameraStruct


  } // end of namespace interfaces

} // end of namespace mars

#endif /* MARS_INTERFACES_CAMERA_STRUCT_H */
