/*
 *  Copyright 2022, DFKI GmbH Robotics Innovation Center
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

 /**
 * \file ODEMesh.h
 * \author Malte Roemmermann, Muhammad Haider Khan Lodhi
 *
 */

#ifndef ODE_MESH_H
#define ODE_MESH_H

#ifdef _PRINT_HEADER_
  #warning "ODEMesh.h"
#endif

#include <mars/utils/MutexLocker.h>
#include "ODEObject.h"
#include <string>

//TODO remove?
#ifndef ODE11
  #define dTriIndex int
#endif

namespace mars {
namespace sim {

    class ODEMesh : public ODEObject {
    public:
      ODEMesh(std::shared_ptr<interfaces::PhysicsInterface> world, interfaces::NodeData * nodeData);
      virtual ~ODEMesh(void);
      virtual bool createODEGeometry(interfaces::NodeData *node) override;
      virtual void destroyNode(void) override;
    protected:
      dVector3 *myVertices;
      dTriIndex *myIndices;
      dTriMeshDataID myTriMeshData;
    };

} // end of namespace sim
} // end of namespace mars

#endif  // ODE_MESH_H
