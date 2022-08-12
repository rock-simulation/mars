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

#include "ODEMesh.h"

namespace mars {
  namespace sim {

    using namespace utils;
    using namespace interfaces;

    ODEMesh::ODEMesh(std::shared_ptr<PhysicsInterface> world, NodeData * nodeData) : ODEObject(world, nodeData) {
      // At this moment we have not much things to do here. ^_^
      std::cout << "DEBUGGG: in ODEMesh Constructor " << __FILE__ << ":" << __LINE__ << std::endl;
      createNode(nodeData);
    }

    ODEMesh::~ODEMesh(void) {
      std::cout << "DEBUGGG: in ODEMesh Destructor " << __FILE__ << ":" << __LINE__ << std::endl;
      if(myVertices) free(myVertices);
      if(myIndices) free(myIndices);
      if(myTriMeshData) dGeomTriMeshDataDestroy(myTriMeshData);
    }

    void ODEMesh::destroyNode(void) {
      ODEObject::destroyNode();
      if(myVertices) free(myVertices);
      if(myIndices) free(myIndices);
      if(myTriMeshData) dGeomTriMeshDataDestroy(myTriMeshData);      
      myVertices = 0;
      myIndices = 0;
      myTriMeshData = 0;
    }    

    /**
     * The method creates an ode box representation of the given node.
     *
     */
    bool ODEMesh::createODEGeometry(NodeData* node) {
      int i;

      if (!node->inertia_set && 
          (node->ext.x() <= 0 || node->ext.y() <= 0 || node->ext.z() <= 0)) {
        LOG_ERROR("Cannot create Node \"%s\" (id=%lu):\n"
                  "  Mesh Nodes must have ext.x(), ext.y(), and ext.z() > 0.\n"
                  "  Current values are: x=%g; y=%g, z=%g",
                  node->name.c_str(), node->index,
                  node->ext.x(), node->ext.y(), node->ext.z());
        return false;
      }

      myVertices = (dVector3*)calloc(node->mesh.vertexcount, sizeof(dVector3));
      myIndices = (dTriIndex*)calloc(node->mesh.indexcount, sizeof(dTriIndex));
      //LOG_DEBUG("%d %d", node->mesh.vertexcount, node->mesh.indexcount);
      // first we have to copy the mesh data to prevent errors in case
      // of double to float conversion
      dReal minx, miny, minz, maxx, maxy, maxz;
      for(i=0; i<node->mesh.vertexcount; i++) {
        myVertices[i][0] = (dReal)node->mesh.vertices[i][0];
        myVertices[i][1] = (dReal)node->mesh.vertices[i][1];
        myVertices[i][2] = (dReal)node->mesh.vertices[i][2];
        if(i==0) {
          minx = myVertices[i][0];
          maxx = myVertices[i][0];
          miny = myVertices[i][1];
          maxy = myVertices[i][1];
          minz = myVertices[i][2];
          maxz = myVertices[i][2];
        }
        else {
          if(minx > myVertices[i][0]) minx = myVertices[i][0];
          if(maxx < myVertices[i][0]) maxx = myVertices[i][0];
          if(miny > myVertices[i][1]) miny = myVertices[i][1];
          if(maxy < myVertices[i][1]) maxy = myVertices[i][1];
          if(minz > myVertices[i][2]) minz = myVertices[i][2];
          if(maxz < myVertices[i][2]) maxz = myVertices[i][2];
        }
      }
      // rescale
      dReal sx = node->ext.x()/(maxx-minx);
      dReal sy = node->ext.y()/(maxy-miny);
      dReal sz = node->ext.z()/(maxz-minz);
      for(i=0; i<node->mesh.vertexcount; i++) {
        myVertices[i][0] *= sx;
        myVertices[i][1] *= sy;
        myVertices[i][2] *= sz;
      }
      for(i=0; i<node->mesh.indexcount; i++) {
        myIndices[i] = (dTriIndex)node->mesh.indices[i];
      }

      // then we can build the ode representation
      myTriMeshData = dGeomTriMeshDataCreate();
      dGeomTriMeshDataBuildSimple(myTriMeshData, (dReal*)myVertices,
                                  node->mesh.vertexcount,
                                  myIndices, node->mesh.indexcount);
      nGeom = dCreateTriMesh(theWorld->getSpace(), myTriMeshData, 0, 0, 0);

      // at this moment we set the mass properties as the mass of the
      // bounding box if no mass and inertia is set by the user
      if(node->inertia_set) {
        setInertiaMass(node);
      }
      else if(node->density > 0) {
        dMassSetBox(&nMass, (dReal)node->density, (dReal)node->ext.x(),
                    (dReal)node->ext.y(),(dReal)node->ext.z());
      }
      else if(node->mass > 0) {
        dMassSetBoxTotal(&nMass, (dReal)node->mass, (dReal)node->ext.x(),
                         (dReal)node->ext.y(),(dReal)node->ext.z());
      }
      std::cout << "Created ODEMesh" << std::endl;
      return true;
    }
  } // end of namespace sim
} // end of namespace mars
