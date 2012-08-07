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

namespace mars {
  namespace utils {

    bool Color::fromConfigItem(ConfigItem *item) {
      a = item->children["a"][0].getDouble();
      r = item->children["r"][0].getDouble();
      g = item->children["g"][0].getDouble();
      b = item->children["b"][0].getDouble();
      return true;
    }

    void Color::toConfigItem(ConfigItem *item) {
      item->children["a"][0] = ConfigItem(a);
      item->children["r"][0] = ConfigItem(r);
      item->children["g"][0] = ConfigItem(g);
      item->children["b"][0] = ConfigItem(b);
    }
    
  } // end of namespace base

} // end of namespace mars

