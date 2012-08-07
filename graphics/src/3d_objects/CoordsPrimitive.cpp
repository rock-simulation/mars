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
 * CoordsPrimitive.cpp
 *
 *  Created on: 19.04.2011
 *      Author: daniel
 */

#include "CoordsPrimitive.h"
#include "gui_helper_functions.h"
#include "../wrapper/OSGDrawItem.h"

namespace mars {
  namespace graphics {

    using namespace std;
    using mars::utils::Vector;

    CoordsPrimitive::CoordsPrimitive(GraphicsWidget *gw, const Vector &size,
                                     std::string resPath,
                                     bool transformFlag) {

      string objPath = resPath;
      objPath.append("/Objects/coords.3ds");
      string fontPath = resPath;
      fontPath.append("/Fonts");

      osg::ref_ptr<osg::Node> coordsCompleteNode;
      osg::ref_ptr<OSGDrawItem> group;

      coordsCompleteNode = GuiHelper::readNodeFromFile(objPath);

      //add child to parent osg groups
      if (transformFlag) {
        // FIXME: scaling of coordsCompleteNode wrong!
        addChild(coordsCompleteNode.get());

        group = new OSGDrawItem(gw);
        OSGDrawItem::createLabel(group, Vector(size.x()/2+0.2, 0.0,0.0), 0.1,"X",
                                 fontPath);
        addChild(group.get());

        group = new OSGDrawItem(gw);
        OSGDrawItem::createLabel(group, Vector(0.0, size.y()/2+0.2, 0.0), 0.1,"Y",
                                 fontPath);
        addChild(group.get());

        group = new OSGDrawItem(gw);
        OSGDrawItem::createLabel(group, Vector(0.0,0.0, size.z()/2+0.2), 0.1,"Z",
                                 fontPath);
        addChild(group.get());
      } else {
        addChild(coordsCompleteNode.get());

        group = new OSGDrawItem(gw);
        OSGDrawItem::createLabel(group, Vector(1.1,0.0,0.0),0.1,"X", fontPath);
        addChild(group.get());

        group = new OSGDrawItem(gw);
        OSGDrawItem::createLabel(group, Vector(0.0,1.1,0.0),0.1,"Y", fontPath);
        addChild(group.get());

        group = new OSGDrawItem(gw);
        OSGDrawItem::createLabel(group, Vector(0.0,0.0,1.1),0.1,"Z", fontPath);
        addChild(group.get());
      }
    }

    CoordsPrimitive::CoordsPrimitive(GraphicsWidget *gw,
                                     std::string resPath) {

      string objPath = resPath;
      objPath.append("/Objects/coords.3ds");

      osg::ref_ptr<osg::Node> coordsCompleteNode;
      osg::ref_ptr<OSGDrawItem> group;

      coordsCompleteNode = GuiHelper::readNodeFromFile(objPath);

      addChild(coordsCompleteNode.get());

      group = new OSGDrawItem(gw);
      string fontPath = resPath;
      fontPath.append("/Fonts");
      OSGDrawItem::createLabel(group, Vector(1.1,0.0,0.0),0.1,"X", fontPath);
      addChild(group.get());

      group = new OSGDrawItem(gw);
      OSGDrawItem::createLabel(group, Vector(0.0,1.1,0.0),0.1,"Y", fontPath);
      addChild(group.get());

      group = new OSGDrawItem(gw);
      OSGDrawItem::createLabel(group, Vector(0.0,0.0,1.1),0.1,"Z", fontPath);
      addChild(group.get());
    }

  } // end of namespace graphics
} // end of namespace mars
