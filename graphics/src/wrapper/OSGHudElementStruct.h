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
 * OSGHudElementStruct.h
 *
 *  Created on: 19.04.2011
 *      Author: daniel
 */

#ifndef MARS_GRAPHICS_OSGHUDELEMENTSTRUCT_H
#define MARS_GRAPHICS_OSGHUDELEMENTSTRUCT_H

#include "../2d_objects/HUDElement.h"

#include <mars_interfaces/graphics/draw_structs.h>

#include <string>

#include <osg/Group>

namespace mars {
  namespace graphics {

    /**
     * Wraps hudElementStruct in osg::Group.
     */
    class OSGHudElementStruct : public osg::Group
    {
    public:
      /**
       * Constructor creates HUDElement from hudElementStruct.
       */
      OSGHudElementStruct(
                          const interfaces::hudElementStruct &he,
                          const std::string &config_path,
                          unsigned int id,
                          osg::Node* node = NULL);
      ~OSGHudElementStruct();

      HUDElement *getHUDElement();
    private:
      HUDElement *elem_;
    }; // end of class OSGHudElementStruct

  } // end of namespace graphics
} // end of namespace mars

#endif /* MARS_GRAPHICS_OSGHUDELEMENTSTRUCT_H */
