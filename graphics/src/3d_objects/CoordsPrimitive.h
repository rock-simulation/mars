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
 * CoordsPrimitive.h
 *
 *  Created on: 19.04.2011
 *      Author: daniel
 */

#ifndef MARS_GRAPHICS_COORDSPRIMITIVE_H
#define MARS_GRAPHICS_COORDSPRIMITIVE_H

#include "GraphicsWidget.h"

#include <mars_utils/Vector.h>

#include <string>
#include <osg/Group>

namespace mars {
  namespace graphics {

    class CoordsPrimitive : public osg::Group {
    public:
      /**
       * creates a coordination reference node showing axis x,y and z directions with
       * appropriate labels
       *
       * @param size The stretching factor of the coordination frame
       * @param transformFlag If set false the main coordination frame will be build
       *
       */
      CoordsPrimitive(GraphicsWidget *gw, const mars::utils::Vector &size,
                      std::string object_path, bool transformFlag=true);

      CoordsPrimitive(GraphicsWidget *gw, std::string object_path);
    }; // end of class CoordsPrimitive

  } // end of namespace graphics
} // end of namespace mars

#endif /* MARS_GRAPHICS_COORDSPRIMITIVE_H */
