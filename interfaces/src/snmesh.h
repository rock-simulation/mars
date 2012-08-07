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

#ifndef MARS_INTERFACES_SNMESH_H
#define MARS_INTERFACES_SNMESH_H

#include "MARSDefs.h"
#include <mars/utils/Color.h>

namespace mars {

  namespace interfaces {

    //mesh structure
    struct snmesh {
      void setZero(){
        vertices = 0;
        normals = 0;
        color= 0;
        tCoords = 0;
        indices = 0;
        indexcount = 0;
        vertexcount = 0;
      }

      snmesh(){
        setZero();
      }

      mydVector3 *vertices;
      mydVector3 *normals;
      utils::Color *color;
      mydVector2 *tCoords;

      int *indices;
      int indexcount;
      int vertexcount;

    }; // end of struct snmesh

  } // end of namespace interfaces

} // end of namespace mars

#endif /* MARS_INTERFACES_SNMESH_H */
