/*
 *  Copyright 2012, DFKI GmbH Robotics Innovation Center
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
 *  VertexBufferTerrain.h
 *  General to inherit from.
 *
 *  Created by Roemmermann on 15.05.12.
 */

#ifndef MARS_GRAPHICS_VERTEX_BUFFER_TERRAIN_H
#define MARS_GRAPHICS_VERTEX_BUFFER_TERRAIN_H

// On MS Windows MultiResHeightMapRenderer.h needs to be included before
// any OpenGL stuff!
#include "MultiResHeightMapRenderer.h"
#include "LoadDrawObject.h"
#include <osg/Drawable>
#include <mars/interfaces/terrainStruct.h>

namespace mars {
  namespace graphics {

    class VertexBufferTerrain : public osg::Drawable {

    public:
      VertexBufferTerrain();  
      VertexBufferTerrain(const interfaces::terrainStruct *ts);

      VertexBufferTerrain(const VertexBufferTerrain &pg,
                          const osg::CopyOp &copyop=osg::CopyOp::SHALLOW_COPY) {
        fprintf(stderr, "error: not implemented yet!!");
      }

      virtual ~VertexBufferTerrain();

      virtual osg::Object* cloneType() const {
        fprintf(stderr, "error: not implemented yet!!");
        return new VertexBufferTerrain();
      }

      virtual osg::Object* clone(const osg::CopyOp& copyop) const {
        fprintf(stderr, "error: not implemented yet!!");
        return new VertexBufferTerrain (*this, copyop);
      }

      virtual void drawImplementation(osg::RenderInfo& renderInfo) const;
      void collideSphere(double xPos, double yPos, double zPos, double radius);
      virtual osg::BoundingBox computeBound() const;
      void setSelected(bool val);

    private:
      MultiResHeightMapRenderer *mrhmr;
      double width, height, scale;

    }; // end of class VertexBufferTerrain

  } // end of namespace graphics
} // end of namespace mars

#endif /* MARS_GRAPHICS_VERTEX_BUFFER_TERRAIN_H */
