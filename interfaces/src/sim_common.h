/*
 *  Copyright 2011, 2012 DFKI GmbH Robotics Innovation Center
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

/**
 *  some defines and structures that are used in the whole simulation
 */

#ifndef MARS_INTERFACES_COMMON_H
#define MARS_INTERFACES_COMMON_H

//#define DEBUG_TIME
#ifdef _PRINT_HEADER_
  #warning "sim_common.h"
#endif

#include "MARSDefs.h"
#include <mars/utils/Vector.h>
#include <mars/utils/Quaternion.h>
#include <mars/utils/Color.h>
#include "nodeState.h"
#include "contact_params.h"
#include "snmesh.h"


#endif  // MARS_INTERFACES_COMMON_H
