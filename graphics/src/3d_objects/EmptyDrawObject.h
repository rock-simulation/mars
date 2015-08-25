/*
 *  Copyright 2015, DFKI GmbH Robotics Innovation Center
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
 *  EmptyDrawObject.h
 *  General EmptyDrawObject to inherit from.
 *
 *  Created by Malte Langosz on 10.07.15.
 */

#ifndef MARS_GRAPHICS_EMPTY_DRAW_OBJECT_H
#define MARS_GRAPHICS_EMPTY_DRAW_OBJECT_H

#include "DrawObject.h"

namespace mars {
  namespace graphics {

    class EmptyDrawObject : public DrawObject {
    public:
    EmptyDrawObject(GraphicsManager *g) : DrawObject(g) {}

    protected:
      std::list< osg::ref_ptr< osg::Geode > > createGeometry() {
        return std::list< osg::ref_ptr< osg::Geode > >();
      }
    };

  } // end of namespace graphics
} // end of namespace mars

#endif /* MARS_GRAPHICS_EMPTY_DRAW_OBJECT_H */
