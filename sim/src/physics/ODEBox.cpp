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

#include "ODEBox.h"

namespace mars {
  namespace sim {

    using namespace utils;
    using namespace interfaces;

    ODEBox::ODEBox(std::shared_ptr<PhysicsInterface> world, NodeData * nodeData) : ODEObject(world, nodeData) {
      // At this moment we have not much things to do here. ^_^
      std::cout << "DEBUGGG: in ODEBox Constructor " << __FILE__ << ":" << __LINE__ << std::endl;
      createNode(nodeData);
    }

    ODEBox::~ODEBox(void) {
      std::cout << "DEBUGGG: in ODEBox Destructor " << __FILE__ << ":" << __LINE__ << std::endl;
    }

    /**
     * \brief The method creates an ode node, which properties are given by
     * the NodeData param node.
     *
     * pre:
     *     - Node sturct should point to a correct object
     *     - we should have a pointer to the physics implementation
     *     - a physically world should have been created to insert a node
     *
     * post:
     *     - a physical node should be created in the world
     *     - the node properties should be set
     */
    bool ODEBox::createNode(NodeData* node) {
#ifdef _VERIFY_WORLD_
      std::cout << "DEBUGGG: in ODEBox create Node " << __FILE__ << ":" << __LINE__ << std::endl;
      sRotation euler = utils::quaternionTosRotation(node->rot);
      fprintf(stderr, "node %d  ;  %.4f, %.4f, %.4f  ;  %.4f, %.4f, %.4f  ;  %.4f  ;  %.4f\n",
              node->index, node->pos.x(), node->pos.y(),
              node->pos.z(), euler.alpha, euler.beta, euler.gamma,
              node->mass, node->density);
#endif
      MutexLocker locker(&(theWorld->iMutex));
      if(theWorld && theWorld->existsWorld()) {
        bool ret;
        ret = createBox(node);
        if(ret == 0) {
          // Error createing the physical Node
          return 0;
        }

        // then, if the geometry was sucsessfully build, we can create a
        // body for the node or add the node to an existing body
        if(node->movable) setProperties(node);
        else if(node->physicMode != NODE_TYPE_PLANE) {
          dQuaternion tmp, t1, t2;
          tmp[1] = (dReal)node->rot.x();
          tmp[2] = (dReal)node->rot.y();
          tmp[3] = (dReal)node->rot.z();
          tmp[0] = (dReal)node->rot.w();
          dGeomSetPosition(nGeom, (dReal)node->pos.x(),
                           (dReal)node->pos.y(), (dReal)node->pos.z());
          //TODO is this irrelevant as we are in ODEBox now? or is this still possible due to wrong parameter definition/ class selection
          if(node->physicMode == NODE_TYPE_TERRAIN) {
            dGeomGetQuaternion(nGeom, t1);
            dQMultiply0(t2, tmp, t1);
            dGeomSetQuaternion(nGeom, t2);
          }
          else
            dGeomSetQuaternion(nGeom, tmp);
        }
        node_data.id = node->index;
        dGeomSetData(nGeom, &node_data);
        locker.unlock();
        setContactParams(node->c_params);
        return 1;
      }
      LOG_WARN("ODEBox: tried to create a Box without there being a world.");
      return 0;
    }

    /**
     * \brief This function rebuilds the geom (type, size and mass) of a node
     *
     *
     * pre:
     *     - the world should be exist
     *     - a geom should be exist
     *
     * post:
     *     - the geom should be rebuild with the new properties
     */
    //TODO Where is this called and how should it be handled?!
    //     Use switch for now and handle it via Registry later
    bool ODEBox::changeNode(NodeData* node) {
      dReal pos[3] = {node->pos.x(), node->pos.y(), node->pos.z()};
      dQuaternion rotation;
      rotation[1] = node->rot.x();
      rotation[2] = node->rot.y();
      rotation[3] = node->rot.z();
      rotation[0] = node->rot.w();
      const dReal *tpos;
#ifdef _VERIFY_WORLD_
      sRotation euler = utils::quaternionTosRotation(node->rot);
      fprintf(stderr, "node %d  ;  %.4f, %.4f, %.4f  ;  %.4f, %.4f, %.4f  ;  %.4f  ;  %.4f\n",
              node->index, node->pos.x(), node->pos.y(),
              node->pos.z(), euler.alpha, euler.beta, euler.gamma,
              node->mass, node->density);
#endif
      MutexLocker locker(&(theWorld->iMutex));

      if(nGeom && theWorld && theWorld->existsWorld()) {
        if(composite) {
          dGeomGetQuaternion(nGeom, rotation);
          tpos = dGeomGetPosition(nGeom);
          pos[0] = tpos[0];
          pos[1] = tpos[1];
          pos[2] = tpos[2];
        }
        // deferre destruction of geom until after the successful creation of 
        // a new geom
        dGeomID tmpGeomId = nGeom;
        // first we create a ode geometry for the node
        bool success = false;
        success = createBox(node);
        if(!success) {
          fprintf(stderr, "creation of body geometry failed.\n");
          return 0;
        }
        if(nBody) {
          theWorld->destroyBody(nBody, this);
          nBody = NULL;
        }
        dGeomDestroy(tmpGeomId);
        // now the geom is rebuild and we have to reconnect it to the body
        // and reset the mass of the body
        if(!node->movable) {
          dGeomSetBody(nGeom, nBody);
          dGeomSetQuaternion(nGeom, rotation);
          dGeomSetPosition(nGeom, (dReal)node->pos.x(),
                           (dReal)node->pos.y(), (dReal)node->pos.z());
        }
        else {
          bool body_created = false;
          if(node->groupID) {
            body_created = theWorld->getCompositeBody(node->groupID, &nBody, this);
            composite = true;
          }
          else {
            composite = false;
            if(!nBody) nBody = dBodyCreate(theWorld->getWorld());
          }
          if(nBody) {
            dGeomSetBody(nGeom, nBody);
            if(!composite) {
              dBodySetMass(nBody, &nMass);
              dGeomSetQuaternion(nGeom, rotation);
              dGeomSetPosition(nGeom, pos[0], pos[1], pos[2]);
            }
            else {
              // if the geom is part of a composite object
              // we have to translate and rotate the geom mass
              if(body_created) {
                dBodySetMass(nBody, &nMass);
                dBodySetPosition(nBody, pos[0], pos[1], pos[2]);
                dBodySetQuaternion(nBody, rotation);
              }
              else {
                dGeomSetOffsetWorldQuaternion(nGeom, rotation);
                dGeomSetOffsetWorldPosition(nGeom, pos[0], pos[1], pos[2]);
                theWorld->resetCompositeMass(nBody);
              }
            }
          }
        }
        dGeomSetData(nGeom, &node_data);
        locker.unlock();
        setContactParams(node->c_params);
      }
      return 1;
    }


    /**
     * The method creates an ode box representation of the given node.
     *
     */
    bool ODEBox::createBox(NodeData* node) {
      if (!node->inertia_set && 
          (node->ext.x() <= 0 || node->ext.y() <= 0 || node->ext.z() <= 0)) {
        LOG_ERROR("Cannot create Node \"%s\" (id=%lu):\n"
                  "  Box Nodes must have ext.x(), ext.y(), and ext.z() > 0.\n"
                  "  Current values are: x=%g; y=%g, z=%g",
                  node->name.c_str(), node->index,
                  node->ext.x(), node->ext.y(), node->ext.z());
        return false;
      }

      // build the ode representation
      nGeom = dCreateBox(theWorld->getSpace(), (dReal)(node->ext.x()),
                         (dReal)(node->ext.y()), (dReal)(node->ext.z()));

      // create the mass object for the box
      if(node->inertia_set) {
        setInertiaMass(node);
      }
      else if(node->density > 0) {
        dMassSetBox(&nMass, (dReal)(node->density), (dReal)(node->ext.x()),
                    (dReal)(node->ext.y()),(dReal)(node->ext.z()));
      }
      else if(node->mass > 0) {
        dReal tempMass =(dReal)(node->mass);
        dMassSetBoxTotal(&nMass, tempMass, (dReal)(node->ext.x()),
                         (dReal)(node->ext.y()),(dReal)(node->ext.z()));
      }
      return true;
    }
      
//    /**
//     * \brief The method creates an ode mesh representation of the given node.
//     *
//     *
//     */
//    bool NodePhysics::createMesh(NodeData* node) {
//      int i;
//
//      if (!node->inertia_set && 
//          (node->ext.x() <= 0 || node->ext.y() <= 0 || node->ext.z() <= 0)) {
//        LOG_ERROR("Cannot create Node \"%s\" (id=%lu):\n"
//                  "  Mesh Nodes must have ext.x(), ext.y(), and ext.z() > 0.\n"
//                  "  Current values are: x=%g; y=%g, z=%g",
//                  node->name.c_str(), node->index,
//                  node->ext.x(), node->ext.y(), node->ext.z());
//        return false;
//      }
//
//      myVertices = (dVector3*)calloc(node->mesh.vertexcount, sizeof(dVector3));
//      myIndices = (dTriIndex*)calloc(node->mesh.indexcount, sizeof(dTriIndex));
//      //LOG_DEBUG("%d %d", node->mesh.vertexcount, node->mesh.indexcount);
//      // first we have to copy the mesh data to prevent errors in case
//      // of double to float conversion
//      dReal minx, miny, minz, maxx, maxy, maxz;
//      for(i=0; i<node->mesh.vertexcount; i++) {
//        myVertices[i][0] = (dReal)node->mesh.vertices[i][0];
//        myVertices[i][1] = (dReal)node->mesh.vertices[i][1];
//        myVertices[i][2] = (dReal)node->mesh.vertices[i][2];
//        if(i==0) {
//          minx = myVertices[i][0];
//          maxx = myVertices[i][0];
//          miny = myVertices[i][1];
//          maxy = myVertices[i][1];
//          minz = myVertices[i][2];
//          maxz = myVertices[i][2];
//        }
//        else {
//          if(minx > myVertices[i][0]) minx = myVertices[i][0];
//          if(maxx < myVertices[i][0]) maxx = myVertices[i][0];
//          if(miny > myVertices[i][1]) miny = myVertices[i][1];
//          if(maxy < myVertices[i][1]) maxy = myVertices[i][1];
//          if(minz > myVertices[i][2]) minz = myVertices[i][2];
//          if(maxz < myVertices[i][2]) maxz = myVertices[i][2];
//        }
//      }
//      // rescale
//      dReal sx = node->ext.x()/(maxx-minx);
//      dReal sy = node->ext.y()/(maxy-miny);
//      dReal sz = node->ext.z()/(maxz-minz);
//      for(i=0; i<node->mesh.vertexcount; i++) {
//        myVertices[i][0] *= sx;
//        myVertices[i][1] *= sy;
//        myVertices[i][2] *= sz;
//      }
//      for(i=0; i<node->mesh.indexcount; i++) {
//        myIndices[i] = (dTriIndex)node->mesh.indices[i];
//      }
//
//      // then we can build the ode representation
//      myTriMeshData = dGeomTriMeshDataCreate();
//      dGeomTriMeshDataBuildSimple(myTriMeshData, (dReal*)myVertices,
//                                  node->mesh.vertexcount,
//                                  myIndices, node->mesh.indexcount);
//      nGeom = dCreateTriMesh(theWorld->getSpace(), myTriMeshData, 0, 0, 0);
//
//      // at this moment we set the mass properties as the mass of the
//      // bounding box if no mass and inertia is set by the user
//      if(node->inertia_set) {
//        setInertiaMass(node);
//      }
//      else if(node->density > 0) {
//        dMassSetBox(&nMass, (dReal)node->density, (dReal)node->ext.x(),
//                    (dReal)node->ext.y(),(dReal)node->ext.z());
//      }
//      else if(node->mass > 0) {
//        dMassSetBoxTotal(&nMass, (dReal)node->mass, (dReal)node->ext.x(),
//                         (dReal)node->ext.y(),(dReal)node->ext.z());
//      }
//      return true;
//    }


//    /**
//     * The method creates an ode shpere representation of the given node.
//     *
//     */
//    bool NodePhysics::createSphere(NodeData* node) {
//      if (!node->inertia_set && node->ext.x() <= 0) {
//        LOG_ERROR("Cannot create Node \"%s\" (id=%lu):\n"
//                  "  Sphere Nodes must have ext.x() > 0.\n"
//                  "  Current value is: x=%g",
//                  node->name.c_str(), node->index, node->ext.x());
//        return false;
//      }
//
//      // build the ode representation
//      nGeom = dCreateSphere(theWorld->getSpace(), (dReal)node->ext.x());
//
//      // create the mass object for the sphere
//      if(node->inertia_set) {
//        setInertiaMass(node);
//      }
//      else if(node->density > 0) {
//        dMassSetSphere(&nMass, (dReal)node->density, (dReal)node->ext.x());
//      }
//      else if(node->mass > 0) {
//        dMassSetSphereTotal(&nMass, (dReal)node->mass, (dReal)node->ext.x());
//      }
//      return true;
//    }
//
//    /**
//     * The method creates an ode capsule representation of the given node.
//     *
//     */
//    bool NodePhysics::createCapsule(NodeData* node) {
//      if (!node->inertia_set && (node->ext.x() <= 0 || node->ext.y() <= 0)) {
//        LOG_ERROR("Cannot create Node \"%s\" (id=%lu):\n"
//                  "  Capsule Nodes must have ext.x() and ext.y() > 0.\n"
//                  "  Current values are: x=%g; y=%g",
//                  node->name.c_str(), node->index,
//                  node->ext.x(), node->ext.y());
//        return false;
//      }
//
//      // build the ode representation
//      nGeom = dCreateCapsule(theWorld->getSpace(), (dReal)node->ext.x(),
//                             (dReal)node->ext.y());
//
//      // create the mass object for the capsule
//      if(node->inertia_set) {
//        setInertiaMass(node);
//      }
//      else if(node->density > 0) {
//        dMassSetCapsule(&nMass, (dReal)node->density, 3, (dReal)node->ext.x(),
//                        (dReal)node->ext.y());
//      }
//      else if(node->mass > 0) {
//        dMassSetCapsuleTotal(&nMass, (dReal)node->mass, 3, (dReal)node->ext.x(),
//                             (dReal)node->ext.y());
//      }
//      return true;
//    }
//
//    /**
//     * The method creates an ode cylinder representation of the given node.
//     *
//     */
//    bool NodePhysics::createCylinder(NodeData* node) {
//      if (!node->inertia_set && (node->ext.x() <= 0 || node->ext.y() <= 0)) {
//        LOG_ERROR("Cannot create Node \"%s\" (id=%lu):\n"
//                  "  Cylinder Nodes must have ext.x() and ext.y() > 0.\n"
//                  "  Current values are: x=%g; y=%g",
//                  node->name.c_str(), node->index,
//                  node->ext.x(), node->ext.y());
//        return false;
//      }
//
//      // build the ode representation
//      nGeom = dCreateCylinder(theWorld->getSpace(), (dReal)node->ext.x(),
//                              (dReal)node->ext.y());
//
//      // create the mass object for the cylinder
//      if(node->inertia_set) {
//        setInertiaMass(node);
//      }
//      else if(node->density > 0) {
//        dMassSetCylinder(&nMass, (dReal)node->density, 3, (dReal)node->ext.x(),
//                         (dReal)node->ext.y());
//      }
//      else if(node->mass > 0) {
//        dMassSetCylinderTotal(&nMass, (dReal)node->mass, 3, (dReal)node->ext.x(),
//                              (dReal)node->ext.y());
//      }
//      return true;
//    }
//
//    /**
//     * The method creates an ode plane
//     *
//     */
//    bool NodePhysics::createPlane(NodeData* node) {
//
//      // build the ode representation
//      nGeom = dCreatePlane(theWorld->getSpace(), 0, 0, 1, (dReal)node->pos.z());
//      return true;
//    }
//
//    bool NodePhysics::createHeightfield(NodeData* node) {
//      dMatrix3 R;
//      unsigned long size;
//      int x, y;
//      terrain = node->terrain;
//      size = terrain->width*terrain->height;
//      if(!height_data) height_data = (dReal*)calloc(size, sizeof(dReal));
//      for(x=0; x<terrain->height; x++) {
//        for(y=0; y<terrain->width; y++) {
//          height_data[(terrain->height-(x+1))*terrain->width+y] = (dReal)terrain->pixelData[x*terrain->width+y];
//        }
//      }
//      // build the ode representation
//      dHeightfieldDataID heightid = dGeomHeightfieldDataCreate();
//
//      // Create an finite heightfield.
//      dGeomHeightfieldDataBuildCallback(heightid, this, heightfield_callback,
//                                        terrain->targetWidth,
//                                        terrain->targetHeight,
//                                        terrain->width, terrain->height,
//                                        REAL(1.0), REAL( 0.0 ),
//                                        REAL(1.0), 0);
//      // Give some very bounds which, while conservative,
//      // makes AABB computation more accurate than +/-INF.
//      dGeomHeightfieldDataSetBounds(heightid, REAL(-terrain->scale*2.0),
//                                    REAL(terrain->scale*2.0));
//      //dGeomHeightfieldDataSetBounds(heightid, -terrain->scale, terrain->scale);
//      nGeom = dCreateHeightfield(theWorld->getSpace(), heightid, 1);
//      dRSetIdentity(R);
//      dRFromAxisAndAngle(R, 1, 0, 0, M_PI/2);
//      dGeomSetRotation(nGeom, R);
//      return true;
//    }

  } // end of namespace sim
} // end of namespace mars
