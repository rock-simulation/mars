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

#include "Color.h"

using namespace configmaps;

namespace mars {
  namespace utils {

    bool Color::fromConfigItem(ConfigItem *item) {
      a = (*item)["a"];
      r = (*item)["r"];
      g = (*item)["g"];
      b = (*item)["b"];
      return true;
    }

    void Color::toConfigItem(ConfigItem *item) {
      (*item)["a"] = a;
      (*item)["r"] = r;
      (*item)["g"] = g;
      (*item)["b"] = b;
    }
    
  } // end of namespace base

} // end of namespace mars

