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
 * GridPrimitive.cpp
 *
 *  Created on: 19.04.2011
 *      Author: daniel
 */

#include "GridPrimitive.h"
#include "../wrapper/OSGDrawItem.h"

#include <mars_interfaces/MARSDefs.h>
#include <mars/utils/Vector.h>
#include <mars_interfaces/graphics/draw_structs.h>

namespace mars {
  namespace graphics {

    using mars::utils::Vector;
    using mars::utils::Color;

    GridPrimitive::GridPrimitive(GraphicsWidget *gw)
    {
      double x, y;
      Vector start, end;
      Color myColor;

      myColor.r = 1.0;
      myColor.g = 1.0;
      myColor.b = 1.0;
      myColor.a = 0.8;

      for(x=-2; x<3; x++) {
        for(y=-2; y<3; y++) {
          start.x() = end.x() = x;
          start.y() = end.y() = y;
          start.z() = 0;
          end.z() = 2;
          osg::ref_ptr<OSGDrawItem> osgNode = new OSGDrawItem(gw);
          OSGDrawItem::createLine(osgNode.get(), start, end, myColor);
          addChild(osgNode.get());
        }
      }
      myColor.r = 0.0;
      myColor.g = 1.0;
      myColor.b = 0.0;
      for (x=-2; x<3; x++) {//=0.1) {
        for (y=1; y<3; y++) {
          start.x() = end.x() = x;
          start.z() = end.z() = y;
          start.y() = 2;
          end.y() = -2;
          osg::ref_ptr<OSGDrawItem> osgNode = new OSGDrawItem(gw);
          OSGDrawItem::createLine(osgNode, start, end, myColor);
          addChild(osgNode.get());
        }
      }
      myColor.r = 0.0;
      myColor.g = 0.0;
      myColor.b = 1.0;
      for (x=-2; x<3; x++) {
        for (y=1; y<3; y++) {
          start.y() = end.y() = x;
          start.z() = end.z() = y;
          start.x() = 2;
          end.x() = -2;
          osg::ref_ptr<OSGDrawItem> osgNode = new OSGDrawItem(gw);
          OSGDrawItem::createLine(osgNode, start, end, myColor);
          addChild(osgNode.get());
        }
      }

      myColor.r = 0.0;
      myColor.g = 1.0;
      myColor.b = 0.0;
      for (x=-1.9; x<2.1; x+=0.1) {
        for (y=0; y<1; y++) {
          start.x() = end.x() = x;
          start.z() = end.z() = y+0.01;
          start.y() = 2;
          end.y() = -2;
          osg::ref_ptr<OSGDrawItem> osgNode = new OSGDrawItem(gw);
          OSGDrawItem::createLine(osgNode, start, end, myColor);
          addChild(osgNode.get());
        }
      }
      myColor.r = 0.0;
      myColor.g = 0.0;
      myColor.b = 1.0;
      for (x=-1.9; x<2.1; x+=0.1) {
        for (y=0; y<1; y++) {
          start.y() = end.y() = x;
          start.z() = end.z() = y+0.01;
          start.x() = 2;
          end.x() = -2;
          osg::ref_ptr<OSGDrawItem> osgNode = new OSGDrawItem(gw);
          OSGDrawItem::createLine(osgNode, start, end, myColor);
          addChild(osgNode.get());
        }
      }

      osg::StateSet *stateset = getOrCreateStateSet();
      stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    }

  } // end of namespace graphics
} // end of namespace mars
