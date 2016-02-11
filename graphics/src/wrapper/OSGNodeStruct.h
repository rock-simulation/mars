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
 * OSGNodeStruct.h
 *
 *  Created on: 19.04.2011
 *      Author: daniel
 */

#ifndef MARS_GRAPHICS_OSGNODESTRUCT_H
#define MARS_GRAPHICS_OSGNODESTRUCT_H

#include "../3d_objects/DrawObject.h"

#include <mars/interfaces/NodeData.h>
#include <mars/interfaces/LightData.h>

#include <osg/Group>

namespace mars {
  namespace graphics {

    /**
     * Wraps a NodeData in a osg::Group.
     * Handles previews and editing of NodeData groups.
     */
    class GraphcisManager;
    class OSGNodeStruct : public osg::Group
    {
    public:
      /**
       * Constructor creates a node in PREVIEW or CREATED state.
       * In PREVIEW state some ressources are not allocated (material,..)
       */
      OSGNodeStruct(GraphicsManager *g,
                    const mars::interfaces::NodeData &node,
                    bool isPreview, unsigned long id);
      /**
       * Edit this node, works only in the PREVIEW state.
       */
      void edit(const mars::interfaces::NodeData &node, bool resize);

      inline DrawObject* object() {return drawObject_;}

      inline unsigned int id() const {return id_;}
    private:
      DrawObject *drawObject_;
      unsigned long id_;
      bool isPreview_;
    }; // end of class OSGNodeStruct

  } // end of namespace graphics
} // end of namespace mars

#endif /* MARS_GRAPHICS_OSGNODESTRUCT_H */
