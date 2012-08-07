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
 * AxisPrimitive.h
 *
 *  Created on: 19.04.2011
 *      Author: daniel
 */

#ifndef MARS_GRAPHICS_AXISPRIMITIVE_H
#define MARS_GRAPHICS_AXISPRIMITIVE_H

#include <mars/utils/Vector.h>

#include <osg/Group>

namespace mars {
  namespace graphics {

    class AxisPrimitive : public osg::Group {
    public:
      AxisPrimitive(const mars::utils::Vector &first,
                    const mars::utils::Vector &second,
                    const mars::utils::Vector &third,
                    const mars::utils::Vector &axis1,
                    const mars::utils::Vector &axis2);
    }; // end of class AxisPrimitive

  } // end of namespace graphics
} // end of namespace mars

#endif /* MARS_GRAPHICS_AXISPRIMITIVE_H */
