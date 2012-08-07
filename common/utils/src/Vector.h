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


#ifndef MARS_UTILS_VECTOR_H
#define MARS_UTILS_VECTOR_H


#ifdef WIN32
#  ifndef EIGEN_DONT_ALIGN
#    define EIGEN_DONT_ALIGN
#  endif
#endif
#include <Eigen/Core>

/* define M_PI if it is not defined (M_PI is *not* part of any standard) */
#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif


namespace mars {
  namespace utils {

    typedef Eigen::Vector3d Vector;

  } // end of namespace utils
} // end of namespace mars

#endif /* MARS_UTILS_VECTOR_H */

