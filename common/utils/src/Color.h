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

#ifndef MARS_UTILS_COLOR_H
#define MARS_UTILS_COLOR_H

#include "ConfigData.h"

namespace mars {
  namespace utils {

    class Color {
    public:
      void setZero(){
        a = 1;
        r = 0;
        g = 0;
        b = 0;
      }
      Color(){
        setZero();
      }
      Color(double r, double g, double b, double a):
        r(r),g(g),b(b),a(a)
      {
      }

      double  r;
      double  g;
      double  b;
      double  a;

      /** Compare with other color (usually compared with default color) */
      bool operator==(const Color &other) const
      {
        /* warning: comparing (double) float values */
        return (a == other.a) && (r == other.r) && (g == other.g) && (b == other.b);
      }

      bool operator!=(const Color& other) const
      {
        /* warning: comparing (double) float values */
        return !(*this == other);
      }

      bool fromConfigItem(ConfigItem *item);
      void toConfigItem(ConfigItem *item);

    }; // end of class Color

  } // end of namespace utils
} // end of namespace mars

#endif /* MARS_UTILS_COLOR_H */
