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
 *  Created by Malte Roemmermann
 */

#ifdef USE_VERTEX_BUFFER

/* The prototypes are only declared in GL/glext.h if this is defined */
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1 //for glGenBuffers, glBindBuffer,
                              //glDeleteBuffers
#endif

#include "VertexBufferTerrain.h"

#ifdef WIN32
#include <windows.h>
#endif

namespace mars {
  namespace graphics {

    VertexBufferTerrain::VertexBufferTerrain() {

      setSupportsDisplayList(false);

      mrhmr = new MultiResHeightMapRenderer(1, 1, 1, 1, 1.0, 1.0, 1, 1.0, 1.0);
      width = height = scale = 1.0;
    }

    VertexBufferTerrain::VertexBufferTerrain(const interfaces::terrainStruct *ts) {

      setSupportsDisplayList(false);

      mrhmr = new MultiResHeightMapRenderer(ts->width, ts->height,
                                            ts->targetWidth, ts->targetHeight,
                                            1.0, 1.0, 1.0, ts->texScale,
                                            ts->texScale);
      double maxHeight = 0.0;

      for(int i=0; i<ts->height; ++i)
        for(int j=0; j<ts->width; ++j) {
          mrhmr->setHeight(i, j, ts->scale*ts->pixelData[i*ts->width+j]);
          if(ts->pixelData[i*ts->width+j]*ts->scale > maxHeight) {
            maxHeight = ts->pixelData[i*ts->width+j]*ts->scale;
          }
        }

      width = ts->targetWidth;
      height = ts->targetHeight;
      scale = maxHeight;
    }

    VertexBufferTerrain::~VertexBufferTerrain() {
      delete mrhmr;
    }

    void VertexBufferTerrain::drawImplementation(osg::RenderInfo& renderInfo) const{
      mrhmr->render();
    }

    void VertexBufferTerrain::collideSphere(double xPos, double yPos,
                                            double zPos, double radius) {

      mrhmr->collideSphere(xPos, yPos, zPos, radius);
    }

    osg::BoundingBox VertexBufferTerrain::computeBound() const {
      fprintf(stderr, "error: not implemented yet!!");

      return osg::BoundingBox(0.0, 0.0, 0.0, width, height, scale);
    }

  } // end of namespace graphics
} // end of namespace mars

#endif /* USE_VERTEX_BUFFER */
