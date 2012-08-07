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


#ifndef MARS_GRAPHICS_HUDNODE_H
#define MARS_GRAPHICS_HUDNODE_H

#include <osg/Geometry>
#include <osg/Camera>

namespace mars {
  namespace graphics {

    class HUDDataType : public osg::Referenced {
    public:
      HUDDataType(osg::Geometry *g, osg::Camera *c);
      ~HUDDataType(void);
      void updateRect(void);
    protected:
      osg::Geometry *geometry;
      osg::Camera *camera;
    }; // end of class HUDDataType

    class HUDNodeCallback : public osg::NodeCallback {
    public:
      virtual void operator()(osg::Node *node, osg::NodeVisitor *nv);
    }; // end of class HUDNodeCallback

  } // end of namespace graphics
} // end of namespace mars

#endif /* MARS_GRAPHICS_HUDNODE_H */
