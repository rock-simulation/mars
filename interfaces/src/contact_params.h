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

#ifndef MARS_INTERFACES_CONTACT_PARAMS_H
#define MARS_INTERFACES_CONTACT_PARAMS_H

#include "MARSDefs.h"
#include <mars/utils/Vector.h>

namespace mars {
  namespace interfaces {

    struct contact_params {
      void setZero(){
        max_num_contacts = 4;
        erp = 0.1;
        cfm = 0.00000001;
        friction1 = 0.8;
        friction2 = 0.8;
        friction_direction1 = 0;
        motion1 = motion2 = 0;
        fds1 = fds2 = 0;
        bounce = bounce_vel = 0;
        approx_pyramid = 1;
        coll_bitmask = 65535;
        depth_correction = 0.0;
      }

      contact_params(){
        setZero();
      }

      int max_num_contacts;
      sReal erp, cfm;
      sReal friction1, friction2;
      utils::Vector *friction_direction1;
      sReal motion1, motion2;
      sReal fds1, fds2;
      sReal bounce, bounce_vel;
      bool approx_pyramid;
      int coll_bitmask;
      sReal depth_correction;
    }; // end of struct contact_params

  } // end of namespace interfaces
} // end of namespace mars

#endif /* MARS_INTERFACES_CONTACT_PARAMS_H */
