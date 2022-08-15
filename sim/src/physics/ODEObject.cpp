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

//TODO cleanup includes
#include "ODEObject.h"
#include "../sensors/RotatingRaySensor.h"

#include <mars/interfaces/Logging.hpp>
#include <mars/utils/MutexLocker.h>
#include <mars/utils/mathUtils.h>
#include <mars/interfaces/sensor_bases.h>
#include <cmath>
#include <set>

#include <mars/interfaces/Logging.hpp>

#include <ode/odemath.h>

#include <iostream>


namespace mars {
namespace sim {

    using namespace utils;
    using namespace interfaces;

    /**
     * \brief Creates a empty node objekt.
     *
     * \pre
     *     - the pointer to the physics Interface should be correct.
     *       This implementation can be a bad trap. The class that implements the
     *       physics interface, have to inherit from the interface at first,
     *       otherwise this implementation will cause bad error while pointing
     *       an incorrect adresses.
     *
     * \post
     *     - the class should have saved the pointer to the physics implementation
     *     - the body and geom should be initialized to 0
     */
    ODEObject::ODEObject(std::shared_ptr<PhysicsInterface> world, NodeData * nodeData) {
      theWorld = std::dynamic_pointer_cast<WorldPhysics>(world);
      nBody = 0;
      nGeom = 0;
      composite = false;
      //node_data.num_ground_collisions = 0;
      node_data.setZero();      
      dMassSetZero(&nMass);
    }

    /**
     * \brief Destroys the node in the physical world.
     *
     * pre:
     *     - theWorld is the correct world object
     *
     * post:
     *     - all physical representation of the node should be cleared
     *
     * are the geom and the body realy all thing to take care of?
     */
    ODEObject::~ODEObject(void) {
      std::vector<sensor_list_element>::iterator iter;
      MutexLocker locker(&(theWorld->iMutex));

      if(nBody) theWorld->destroyBody(nBody, this);

      if(nGeom) dGeomDestroy(nGeom);

      // TODO: how does this loop work? why doesn't it run forever?
      for(iter = sensor_list.begin(); iter != sensor_list.end();) {
        if((*iter).gd){
          delete ((*iter).gd);
          (*iter).gd = 0;
        }
        dGeomDestroy((*iter).geom);
        sensor_list.erase(iter);
      }
    }

    /**
     * \brief The method copies the position of the node at the adress
     * of the pointer pos.
     *
     * pre:
     *     - the physical representation of the node should be availbe
     *     - the node should have an body (should be movable)
     *     - the position pointer param should point to a correct position struct
     *
     * post:
     *     - if the node is physically availbe and is set to be movable
     *       the struct of the position pointer should be filled with
     *       the physical position of the node
     *     - otherwise the position should be set to zero
     */

    bool ODEObject::createODEGeometry(interfaces::NodeData *node){
      std::cout << "ODEObject: using default createODEGeometry func. Did you forget to override it?." << std::endl;
      LOG_WARN("ODEObject: using default createODEGeometry func. Did you forget to override it?.");
      return true;
    }

    bool ODEObject::changeNode(interfaces::NodeData* node) {
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
        success = createODEGeometry(node);
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
    
    bool ODEObject::createNode(interfaces::NodeData* node) {
#ifdef _VERIFY_WORLD_
      sRotation euler = utils::quaternionTosRotation(node->rot);
      fprintf(stderr, "node %d  ;  %.4f, %.4f, %.4f  ;  %.4f, %.4f, %.4f  ;  %.4f  ;  %.4f\n",
              node->index, node->pos.x(), node->pos.y(),
              node->pos.z(), euler.alpha, euler.beta, euler.gamma,
              node->mass, node->density);
#endif
      MutexLocker locker(&(theWorld->iMutex));
      if(theWorld && theWorld->existsWorld()) {
        bool ret;
        ret = createODEGeometry(node); 
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


    void ODEObject::getPosition(Vector* pos) const {
      MutexLocker locker(&(theWorld->iMutex));
      if(nBody) {
        const dReal* tmp = dGeomGetPosition(nGeom);
        pos->x() = (sReal)tmp[0];
        pos->y() = (sReal)tmp[1];
        pos->z() = (sReal)tmp[2];
        /*
          if(composite) {
          tmp = dGeomGetOffsetPosition(nGeom);
          pos->x += (sReal)tmp[0];
          pos->y += (sReal)tmp[1];
          pos->z += (sReal)tmp[2];
          }*/
      }
      else if(nGeom) {
        const dReal* tmp = dGeomGetPosition(nGeom);
        pos->x() = (sReal)tmp[0];
        pos->y() = (sReal)tmp[1];
        pos->z() = (sReal)tmp[2];
      }
      else {
        pos->x() = 0;
        pos->y() = 0;
        pos->z() = 0;
      }
    }

    /**
     * \brief The method sets the position of the physical node model to the
     * position of the param.  If move_group is set, all nodes of a composite
     * group will be moved, otherwise only this node will be moved.  A vector
     * from the old position to the new will be returned.
     *
     * I don't know if we should use this function in a way like it is
     * implemented now. The pre and post conditions could loke like this:
     *
     * pre:
     *     - there should be a physical representation of the node
     *     - the pos param should point to a correct position struct
     *
     * post:
     *     - if there is a physically representation and the node is movable
     *       set the position of the corresponding body to the given parameter
     *     - otherwise, we have to do nothing
     */
    const Vector ODEObject::setPosition(const Vector &pos, bool move_group) {
      const dReal *tpos;
      const dReal *tpos2;
      dReal npos[3];
      Vector offset;
      MutexLocker locker(&(theWorld->iMutex));

      if(composite) {
        if(move_group) {
          /*
            brot = dBodyGetQuaternion(nBody);
            tpos2 = dGeomGetOffsetPosition(nGeom);
            tpos = dBodyGetPosition(nBody);
            dQtoR(brot, R);
            dMULTIPLY0_331(npos, R, tpos2);
            offset.x = pos->x - (sReal)(tpos[0]+npos[0]);
            offset.y = pos->y - (sReal)(tpos[1]+npos[1]);
            offset.z = pos->z - (sReal)(tpos[2]+npos[2]);
          */
          tpos2 = dGeomGetPosition(nGeom);
          offset.x() = pos.x() - (sReal)(tpos2[0]);
          offset.y() = pos.y() - (sReal)(tpos2[1]);
          offset.z() = pos.z() - (sReal)(tpos2[2]);
          tpos = dBodyGetPosition(nBody);
          dBodySetPosition(nBody, tpos[0] + (dReal)offset.x(),
                           tpos[1] + (dReal)offset.y(),
                           tpos[2] + (dReal)offset.z());
          return offset;
        }
        else {
          tpos = dBodyGetPosition(nBody);
          tpos2 = dGeomGetOffsetPosition(nGeom);
          npos[0] = tpos[0] + tpos2[0];
          npos[1] = tpos[1] + tpos2[1];
          npos[2] = tpos[2] + tpos2[2];
          offset.x() = pos.x() - (sReal)(npos[0]);
          offset.y() = pos.y() - (sReal)(npos[1]);
          offset.z() = pos.z() - (sReal)(npos[2]);
          //std::cout << " " << pos.z();
          dGeomSetOffsetWorldPosition(nGeom, (dReal)pos.x(), (dReal)pos.y(),
                                      (dReal)pos.z());
          // here we have to recalculate the mass
          theWorld->resetCompositeMass(nBody);
          return offset;
        }
      }
      else {
        if(nBody) {
          tpos = dBodyGetPosition(nBody);
          offset.x() = pos.x() - (sReal)(tpos[0]);
          offset.y() = pos.y() - (sReal)(tpos[1]);
          offset.z() = pos.z() - (sReal)(tpos[2]);
          dBodySetPosition(nBody, (dReal)pos.x(), (dReal)pos.y(), (dReal)pos.z());
          return offset;
        }
        else if(nGeom) {
          //tpos = dGeomGetPosition(nGeom);
          //offset.x() = pos->x - (sReal)(tpos[0]);
          //offset.y() = pos->y - (sReal)(tpos[1]);
          //offset.z() = pos->z - (sReal)(tpos[2]);
          dGeomSetPosition(nGeom, (dReal)pos.x(), (dReal)pos.y(), (dReal)pos.z());
          return offset;
        }
      }
      return offset;
    }

    /**
     * \brief The method copies the Quaternion of the physically node at the
     * adress of the Quaternion pointer q.
     *
     * pre:
     *     - there should be a physical representation of the node
     *     - the node should be movable
     *     - the q param should point to a correct Quaternion struct
     *
     * post:
     *     - if there is a physical representation and the node is movable
     *       the Quaternion struct should be filled with the physical rotation
     *       of the node
     *     - otherwise a standard return of zero rotation should be set
     */
    void ODEObject::getRotation(Quaternion* q) const {
      dQuaternion tmp;
      MutexLocker locker(&(theWorld->iMutex));

      if(nBody || nGeom) {
        dGeomGetQuaternion(nGeom, tmp);
        q->x() = (sReal)tmp[1];
        q->y() = (sReal)tmp[2];
        q->z() = (sReal)tmp[3];
        q->w() = (sReal)tmp[0];
      }
      else {
        q->x() = (sReal)0;
        q->y() = (sReal)0;
        q->z() = (sReal)0;
        q->w() = (sReal)1;    
      }
    }

    /**
     * \brief The method copies the linear velocity of the physically node at the
     * adress of the linear_vel pointer vel.
     *
     * pre:
     *     - there should be a physical representation of the node
     *     - the node should be movable
     *     - the vel param should point to a correct linear_vel struct
     *
     * post:
     *     - if there is a physical representation and the node is movable
     *       the linear_vel struct should be filled with the physical linar
     *       velocity of the node
     *     - otherwise a standard return of zero velocity should be set
     */
    void ODEObject::getLinearVelocity(Vector* vel) const {
      const dReal *tmp;
      MutexLocker locker(&(theWorld->iMutex));

      if(nBody) {
        tmp = dBodyGetLinearVel(nBody);
        vel->x() = (sReal)tmp[0];
        vel->y() = (sReal)tmp[1];
        vel->z() = (sReal)tmp[2];
      }
      else {
        vel->x() = (sReal)0;
        vel->y() = (sReal)0;
        vel->z() = (sReal)0;
      }  
    }

    /**
     * \brief The method copies the angular velocity of the physically node at the
     * adress of the angular_vel pointer vel.
     *
     * pre:
     *     - there should be a physical representation of the node
     *     - the node should be movable
     *     - the vel param should point to a correct angular_vel struct
     *
     * post:
     *     - if there is a physical representation and the node is movable
     *       the angular_vel struct should be filled with the physical angular
     *       velocity of the node
     *     - otherwise a standard return of zero velocity should be set
     */
    void ODEObject::getAngularVelocity(Vector* vel) const {
      const dReal *tmp;
      MutexLocker locker(&(theWorld->iMutex));

      if(nBody) {
        tmp = dBodyGetAngularVel(nBody);
        vel->x() = (sReal)tmp[0];
        vel->y() = (sReal)tmp[1];
        vel->z() = (sReal)tmp[2];
      }
      else {
        vel->x() = (sReal)0;
        vel->y() = (sReal)0;
        vel->z() = (sReal)0;
      }  
    }

    /**
     * \brief The method copies the force of the physically node at the
     * adress of the force pointer force.
     *
     * pre:
     *     - there should be a physical representation of the node
     *     - the node should be movable
     *     - the f param should point to a correct force struct
     *
     * post:
     *     - if there is a physical representation and the node is movable
     *       the force struct should be filled with the physical
     *       force of the node
     *     - otherwise a standard return of zero force should be set
     */
    void ODEObject::getForce(Vector* f) const {
      const dReal *tmp;
      MutexLocker locker(&(theWorld->iMutex));

      if(nBody) {
        tmp = dBodyGetForce(nBody);
        f->x() = (sReal)tmp[0];
        f->y() = (sReal)tmp[1];
        f->z() = (sReal)tmp[2];
      }
      else {
        f->x() = (sReal)0;
        f->y() = (sReal)0;
        f->z() = (sReal)0;
      }  
    }

    /**
     * \brief The method copies the torque of the physically node at the
     * adress of the torque pointer force.
     *
     * pre:
     *     - there should be a physical representation of the node
     *     - the node should be movable
     *     - the t param should point to a correct torque struct
     *
     * post:
     *     - if there is a physical representation and the node is movable
     *       the torque struct should be filled with the physical torque
     *       of the node
     *     - otherwise a standard return of zero torque should be set
     */
    void ODEObject::getTorque(Vector *t) const {
      const dReal *tmp;
      MutexLocker locker(&(theWorld->iMutex));

      if(nBody) {
        tmp = dBodyGetTorque(nBody);
        t->x() = (sReal)tmp[0];
        t->y() = (sReal)tmp[1];
        t->z() = (sReal)tmp[2];
      }
      else {
        t->x() = (sReal)0;
        t->y() = (sReal)0;
        t->z() = (sReal)0;
      }  
    }


    /**
     * \brief This method sets the rotation of the physically node.
     *
     * I don't if and how to use this function yet. ^-^
     * If we need it, the pre and post conditions are like them in the set
     * position method.
     */
    const Quaternion ODEObject::setRotation(const Quaternion &q, bool move_group) {
      dQuaternion tmp, tmp2, tmp3, tmp4, tmp5;
      const dReal *brot, *bpos, *gpos;
      Quaternion q2;
      dMatrix3 R;
      dVector3 pos, new_pos, new2_pos;
      MutexLocker locker(&(theWorld->iMutex));

      pos[0] = pos[1] = pos[2] = 0;
      tmp[1] = (dReal)q.x();
      tmp[2] = (dReal)q.y();
      tmp[3] = (dReal)q.z();
      tmp[0] = (dReal)q.w();
      if(nBody) {
        bpos = dBodyGetPosition(nBody);
        pos[0] = bpos[0];
        pos[1] = bpos[1];
        pos[2] = bpos[2];
        brot = dBodyGetQuaternion(nBody);
        tmp4[0] = brot[0];
        tmp4[1] = brot[1];
        tmp4[2] = brot[2];
        tmp4[3] = brot[3];
        if(composite && !move_group) {
          dGeomGetQuaternion(nGeom, tmp2);
          dGeomSetOffsetWorldQuaternion(nGeom, tmp);
        }
        else if (composite) {
          dGeomGetQuaternion(nGeom, tmp2);
          dGeomGetOffsetQuaternion(nGeom, tmp3);
          tmp3[1] *= -1;
          tmp3[2] *= -1;
          tmp3[3] *= -1;
          dQMultiply0(tmp5, tmp, tmp3);
          dBodySetQuaternion(nBody, tmp5);
        }
        else {
          tmp2[0] = brot[0];
          tmp2[1] = brot[1];
          tmp2[2] = brot[2];
          tmp2[3] = brot[3];
          dBodySetQuaternion(nBody, tmp);
        }
      }
      else if(nGeom) {
        dGeomGetQuaternion(nGeom, tmp2);
        dGeomSetQuaternion(nGeom, tmp);
      }
      dQMultiply2(tmp3, tmp, tmp2);
      q2.x() = (sReal)tmp3[1];
      q2.y() = (sReal)tmp3[2];
      q2.z() = (sReal)tmp3[3];
      q2.w() = (sReal)tmp3[0];
      if(nBody && composite && move_group) {
        gpos = dGeomGetOffsetPosition(nGeom);
        dQtoR(tmp4, R);
        dMULTIPLY0_331(new_pos, R, gpos);
        pos[0] += new_pos[0];
        pos[1] += new_pos[1];
        pos[2] += new_pos[2];
        new_pos[0] *= -1;
        new_pos[1] *= -1;
        new_pos[2] *= -1;
        dQtoR(tmp3, R);
        dMULTIPLY0_331(new2_pos, R, new_pos);
        new_pos[0] = pos[0]+new2_pos[0];
        new_pos[1] = pos[1]+new2_pos[1];
        new_pos[2] = pos[2]+new2_pos[2];
        dBodySetPosition(nBody, new_pos[0], new_pos[1], new_pos[2]);
      }
      return q2;
    }

    /**
     * \brief This function sets the pointer to the physical world object.
     *
     * I don't think that we need this function.
     */
    void ODEObject::setWorldObject(std::shared_ptr<PhysicsInterface>  world) {
      theWorld = std::dynamic_pointer_cast<WorldPhysics>(world);
    }

    /**
     *\brief return the body;
     * this function is created to make it possible to get the
     * body from joint physics
     *TO DO : test if the Node has a body
     */
    dBodyID ODEObject::getBody() const {
      return nBody;
    }

    /**
     * This method sets some properties for the node. The properties includes
     * the posistion, the rotation, the movability and the coposite group number
     *
     */
    void ODEObject::setProperties(NodeData* node) {
      bool body_created = 1;
      dQuaternion tmp;

      tmp[1] = (dReal)node->rot.x();
      tmp[2] = (dReal)node->rot.y();
      tmp[3] = (dReal)node->rot.z();
      tmp[0] = (dReal)node->rot.w();

      // first we must find or create a physically body for the node
      // and connect the geometry to the body
      if(node->groupID) {
        body_created = theWorld->getCompositeBody(node->groupID, &nBody, this);
        composite = true;
      }
      else {
        if(!nBody) nBody = dBodyCreate(theWorld->getWorld());
      }
      //dBodySetLinearDamping(nBody, 0.5);
      //dBodySetAngularDamping(nBody, 0.5);
      dGeomSetBody(nGeom, nBody);

      // if a new body was created, we have to set the initial position
      // and rotation of the body, otherwise have to set the position
      // and rotation of the geom
      if(body_created) {
        dBodySetMass(nBody, &nMass);
        dBodySetPosition(nBody, (dReal)(node->pos.x()),
                         (dReal)(node->pos.y()),
                         (dReal)(node->pos.z()));
        // ## the rotation has to be defined as quanternion or matrix? ##
        dBodySetQuaternion(nBody, tmp);
      }
      else if(node->density > 0 || node->mass > 0) {
        dMass bodyMass;

        dBodyGetMass(nBody, &bodyMass);
        dGeomSetOffsetWorldPosition(nGeom, (dReal)(node->pos.x()),
                                    (dReal)(node->pos.y()),
                                    (dReal)(node->pos.z()));
        // ## the same for the rotation here ##
        dGeomSetOffsetWorldQuaternion(nGeom, tmp);
        addMassToCompositeBody(nBody, &bodyMass);
        dBodySetMass(nBody, &bodyMass);
      }
#ifdef _DEBUG_MASS_
      fprintf(stderr, "%mass id: %d %g\n", node->index, nMass.mass);
      fprintf(stderr, "\t%g\t%g\t%g\n", nMass.I[0], nMass.I[1], nMass.I[2]);
      fprintf(stderr, "\t%g\t%g\t%g\n", nMass.I[4], nMass.I[5], nMass.I[6]);
      fprintf(stderr, "\t%g\t%g\t%g\n", nMass.I[8], nMass.I[9], nMass.I[10]);
#endif
    }

    /**
     * \brief executes an rotation at a given point and returns the
     * new position of the node
     *
     * pre:
     *
     * post:
     */
    const Vector ODEObject::rotateAtPoint(const Vector &rotation_point,
                                            const Quaternion &rotation,
                                            bool move_group) {
      dQuaternion tmp, tmp2, tmp3;
      const dReal *bpos, *gpos, *brot;
      dVector3 pos, new_pos;
      Vector npos;
      dMatrix3 R;
      MutexLocker locker(&(theWorld->iMutex));
  
      tmp[1] = (dReal)rotation.x();
      tmp[2] = (dReal)rotation.y();
      tmp[3] = (dReal)rotation.z();
      tmp[0] = (dReal)rotation.w();
      if(composite) {
        brot = dBodyGetQuaternion(nBody);
        dQMultiply0(tmp2, tmp, brot);
        if(move_group) {
          // we have to rotate the body and return the new position of the geom
          dBodySetQuaternion(nBody, tmp2);
          dQtoR(tmp, R);
          bpos = dBodyGetPosition(nBody);
          pos[0] = bpos[0] - (dReal)rotation_point.x();
          pos[1] = bpos[1] - (dReal)rotation_point.y();
          pos[2] = bpos[2] - (dReal)rotation_point.z();
          dMULTIPLY0_331(new_pos, R, pos);
          npos.x() = new_pos[0] + (dReal)rotation_point.x();
          npos.y() = new_pos[1] + (dReal)rotation_point.y();
          npos.z() = new_pos[2] + (dReal)rotation_point.z();
          dBodySetPosition(nBody, (dReal)npos.x(), (dReal)npos.y(), (dReal)npos.z());
      
          gpos = dGeomGetOffsetPosition(nGeom);
          npos.x() += (sReal)(gpos[0]);
          npos.y() += (sReal)(gpos[1]);
          npos.z() += (sReal)(gpos[2]);
          return npos;
        }
        else {
          dGeomGetQuaternion(nGeom, tmp3);
          dQMultiply0(tmp2, tmp, tmp3);
          dNormalize4(tmp2);
          dGeomSetOffsetWorldQuaternion(nGeom, tmp2);

          gpos = dGeomGetPosition(nGeom);
          pos[0] = gpos[0] - (dReal)rotation_point.x();
          pos[1] = gpos[1] - (dReal)rotation_point.y();
          pos[2] = gpos[2] - (dReal)rotation_point.z();
          dQtoR(tmp, R);
          dMULTIPLY0_331(new_pos, R, pos);
          pos[0] = new_pos[0] + (dReal)rotation_point.x();
          pos[1] = new_pos[1] + (dReal)rotation_point.y();
          pos[2] = new_pos[2] + (dReal)rotation_point.z();
          dGeomSetOffsetWorldPosition(nGeom, pos[0], pos[1], pos[2]);
          npos.x() = (sReal)(pos[0]);
          npos.y() = (sReal)(pos[1]);
          npos.z() = (sReal)(pos[2]);
         return npos;
        }
      }
      // the last two cases do in principle the same
      // we have to rotate the geom and calculate the translation
      /*
        else if(nBody) {

        }*/
      else if(nGeom) {
        dGeomGetQuaternion(nGeom, tmp2);
        dQMultiply0(tmp3, tmp, tmp2);
        dGeomSetQuaternion(nGeom, tmp3);
        dQtoR(tmp, R);
        gpos = dGeomGetPosition(nGeom);
        pos[0] = gpos[0] - (dReal)rotation_point.x();
        pos[1] = gpos[1] - (dReal)rotation_point.y();
        pos[2] = gpos[2] - (dReal)rotation_point.z();
        dMULTIPLY0_331(new_pos, R, pos);
        npos.x() = new_pos[0] + (dReal)rotation_point.x();
        npos.y() = new_pos[1] + (dReal)rotation_point.y();
        npos.z() = new_pos[2] + (dReal)rotation_point.z();
        dGeomSetPosition(nGeom, (dReal)npos.x(), (dReal)npos.y(), (dReal)npos.z());
        return npos;
      }
      return npos;
    }

    /**
     * \brief returns the ode mass object
     *
     *
     * pre:
     *
     * post:
     */
    dMass ODEObject::getODEMass(void) const {
      return nMass;
    }

    /**
     * \brief Sets the linear velocity of a node
     *
     * pre:
     *      - the node should have a body
     *
     * post:
     *      - the linear velocity of the body should be set
     */
    void ODEObject::setLinearVelocity(const Vector &velocity) {
      MutexLocker locker(&(theWorld->iMutex));
      if(nBody) dBodySetLinearVel(nBody, (dReal)velocity.x(),
                                  (dReal)velocity.y(), (dReal)velocity.z());
    }

    /**
     * \brief Sets the angular velocity of a node
     *
     * pre:
     *      - the node should have a body
     *
     * post:
     *      - the angular velocity of the body should be set
     */
    void ODEObject::setAngularVelocity(const Vector &velocity) {
      MutexLocker locker(&(theWorld->iMutex));
      if(nBody) dBodySetAngularVel(nBody, (dReal)velocity.x(),
                                   (dReal)velocity.y(), (dReal)velocity.z());
    }

    /**
     * \brief Sets the force of a node
     *
     * pre:
     *      - the node should have a body
     *
     * post:
     *      - the force of the body should be set
     */
    void ODEObject::setForce(const Vector &f) {
      MutexLocker locker(&(theWorld->iMutex));
      if(nBody) dBodySetForce(nBody, (dReal)f.x(),
                              (dReal)f.y(), (dReal)f.z());
    }

    /**
     * \brief Sets the torque of a node
     *
     * pre:
     *      - the node should have a body
     *
     * post:
     *      - the torque of the body should be set
     */
    void ODEObject::setTorque(const Vector &t) {
      MutexLocker locker(&(theWorld->iMutex));
      if(nBody) dBodySetTorque(nBody, (dReal)t.x(),
                               (dReal)t.y(), (dReal)t.z());
    }

    /**
     * \brief Adds a off-center force to a node
     *
     * pre:
     *      - the node should have a body
     *
     * post:
     *      - the force should be added to the body
     */
    void ODEObject::addForce(const Vector &f, const Vector &p) {
      MutexLocker locker(&(theWorld->iMutex));
      if(nBody) {
        dBodyAddForceAtPos(nBody, 
                           (dReal)f.x(), (dReal)f.y(), (dReal)f.z(),
                           (dReal)p.x(), (dReal)p.y(), (dReal)p.z());
      }
    }
    /**
     * \brief Adds a force to a node
     *
     * pre:
     *      - the node should have a body
     *
     * post:
     *      - the force should be added to the body
     */
    void ODEObject::addForce(const Vector &f) {
      MutexLocker locker(&(theWorld->iMutex));
      if(nBody) {
        dBodyAddForce(nBody, (dReal)f.x(), (dReal)f.y(), (dReal)f.z());
      }
    }

    /**
     * \brief Adds a torque to a node
     *
     * pre:
     *      - the node should have a body
     *
     * post:
     *      - the torque should be added to the body
     */
    void ODEObject::addTorque(const Vector &t) {
      MutexLocker locker(&(theWorld->iMutex));
      if(nBody) dBodyAddTorque(nBody, (dReal)t.x(), (dReal)t.y(), (dReal)t.z());
    }

    bool ODEObject::getGroundContact(void) const {
      if(nGeom) {
        return node_data.num_ground_collisions;
      }
      return false;
    }

    void ODEObject::getContactPoints(std::vector<Vector> *contact_points) const {
      contact_points->clear();
      if(nGeom) {
        std::vector<Vector>::const_iterator iter;

        for(iter=node_data.contact_points.begin();
            iter!=node_data.contact_points.end(); ++iter)

          contact_points->push_back((*iter));
      }
    }

    void ODEObject::getContactIDs(std::list<interfaces::NodeId> *ids) const {
      ids->clear();
      if(nGeom) {
        *ids = node_data.contact_ids;
      }
    }


    sReal ODEObject::getGroundContactForce(void) const {
      std::vector<dJointFeedback*>::const_iterator iter;
      dReal force[3] = {0,0,0};

      if(nGeom) {
        for(iter = node_data.ground_feedbacks.begin();
            iter != node_data.ground_feedbacks.end(); iter++) {
          if(node_data.node1) {
            force[0] += (*iter)->f1[0];
            force[1] += (*iter)->f1[1];
            force[2] += (*iter)->f1[2];
          }
          else {
            force[0] += (*iter)->f2[0];
            force[1] += (*iter)->f2[1];
            force[2] += (*iter)->f2[2];
          }
        }
      }
      return dLENGTH(force);
    }

    const Vector ODEObject::getContactForce(void) const {
      std::vector<dJointFeedback*>::const_iterator iter;
      dReal force[3] = {0,0,0};

      if(nGeom) {
        for(iter = node_data.ground_feedbacks.begin();
            iter != node_data.ground_feedbacks.end(); iter++) {
          if(node_data.node1) {
            force[0] += (*iter)->f1[0];
            force[1] += (*iter)->f1[1];
            force[2] += (*iter)->f1[2];
          }
          else {
            force[0] += (*iter)->f2[0];
            force[1] += (*iter)->f2[1];
            force[2] += (*iter)->f2[2];
          }
        }
      }
      return Vector(force[0], force[1], force[2]);
    }

    void ODEObject::addCompositeOffset(dReal x, dReal y, dReal z) {
      // no lock because physics internal functions get locked elsewhere
      const dReal *gpos;

      gpos = dGeomGetPosition(nGeom);
      dGeomSetOffsetWorldPosition(nGeom, gpos[0]+x, gpos[1]+y, gpos[2]+z);
    }

    void ODEObject::addMassToCompositeBody(dBodyID theBody, dMass *bodyMass) {
      // no lock because physics internal functions get locked elsewhere
      const dReal *rotation, *pos, *brot;
      dReal tpos[3];
      dMatrix3 R;
      dMass myMass = nMass;

      // first rotate and translate the mass and add it to the body
      // we get the rotation and translation relative to the body-frame
      // from the geom
      rotation = dGeomGetOffsetRotation(nGeom);
      pos = dGeomGetOffsetPosition(nGeom);
      dMassRotate(&myMass, rotation);
      dMassTranslate(&myMass, pos[0], pos[1], pos[2]);
      dMassAdd(bodyMass, &myMass);
      // the mass displacement is in bodyframe
      // to get the displacement in world koordinates
      // we have to rotate the vector
      brot = dBodyGetQuaternion(theBody);
      dRfromQ(R, brot);
      dMULTIPLY0_331(tpos, R, bodyMass->c);
      // then we can add the vector to the geoms and body
      theWorld->moveCompositeMassCenter(theBody, tpos[0], tpos[1], tpos[2]);
      dMassTranslate(bodyMass, -bodyMass->c[0], -bodyMass->c[1], -bodyMass->c[2]);
    }

    void ODEObject::getAbsMass(dMass *tMass) const {
      // no lock because physics internal functions get locked elsewhere
      const dReal *pos = dGeomGetPosition(nGeom);
      const dReal *rot = dGeomGetRotation(nGeom);

      *tMass = nMass;
      dMassRotate(tMass, rot);
      dMassTranslate(tMass, pos[0], pos[1], pos[2]);
    }

    void ODEObject::setContactParams(contact_params& c_params) {
      MutexLocker locker(&(theWorld->iMutex));
      node_data.c_params = c_params;
      if(nGeom) {
        dGeomSetCollideBits(nGeom, c_params.coll_bitmask);
        dGeomSetCategoryBits(nGeom, c_params.coll_bitmask);
      }
    }

    /**
     * \brief This function adds a new sensor to the physical node.
     *  Sensors that are implemented are a "ray sensor" and a "multi ray sensor"
     *
     *
     * pre:
     *     - the BaseSensor should contain a complete and correct configuration
     *     - the world have to exists because geoms have to be created
     *     - this Node should have an existing geom
     *
     * post:
     *     - every new sensor has to be added to a list
     *     - the memory for the sensor data has to be allocated
     *     - the physical elements for the sensor had to be created
     */
    void ODEObject::addSensor(BaseSensor* sensor) {
      MutexLocker locker(&(theWorld->iMutex));
      int i;
      geom_data* gd;
      sensor_list_element sle;
      const dReal *pos = dGeomGetPosition(nGeom);
      const dReal *rot = dGeomGetRotation(nGeom);
      Vector direction;
      dVector3 dest, tmp;
      //sReal rad_angle, rad_steps, rad_start;
      double rad_steps, rad_start;


      BasePolarIntersectionSensor *polarSensor;
      polarSensor = dynamic_cast<BasePolarIntersectionSensor*>(sensor);
  
      //case SENSOR_TYPE_RAY:
      if(polarSensor){
        sle.sensor = sensor;
        sle.updateTime = 0.0;
        //sensor.count_data = sensor.resolution;
        //sensor.data = (sReal*)malloc(sensor.resolution * sizeof(sReal));
   
        mars::sim::RotatingRaySensor* rotRaySensor = dynamic_cast<RotatingRaySensor*>(sensor);
        if(rotRaySensor){
            int N = rotRaySensor->getNumberRays();
            std::vector<utils::Vector>& directions = rotRaySensor->getDirections();
            assert(N == directions.size());
            
            // Requests and adds the single rays using the local sensor frame.
            for(i=0; i<N; i++){
                gd = new geom_data;
                (*gd).setZero();
                gd->sense_contact_force = 0;
                (*polarSensor)[i] = gd->value = polarSensor->maxDistance;//sensor.max_distance;
                gd->ray_sensor = 1;
                gd->parent_geom = nGeom;
                gd->parent_body = nBody;
                sle.geom = dCreateRay(NULL, polarSensor->maxDistance);
                dGeomRaySetClosestHit(sle.geom, 1);
                dGeomSetCollideBits(sle.geom, 32768);
                dGeomSetCategoryBits(sle.geom, 32768);
                
                // Use the precalculated ray directions of the sensor. 
                direction = /*polarSensor->getOrientation() **/ directions[i];
                sle.ray_direction = direction;
                tmp[0] = direction.x();
                tmp[1] = direction.y();
                tmp[2] = direction.z();
                dMULTIPLY0_331(dest, rot, tmp);
                dGeomRaySet(sle.geom, pos[0], pos[1], pos[2], dest[0], dest[1], dest[2]);
                sle.gd = gd;
                sle.index = i;
                sensor_list.push_back(sle);
                dGeomSetData(sle.geom, gd);
                //dGeomRaySetParams(sle.geom, 1, 1);
                dGeomSetCollideBits(sle.geom, COLLIDE_MASK_SENSOR);
                dGeomSetCategoryBits(sle.geom, COLLIDE_MASK_SENSOR);
                dGeomDisable(sle.geom);
            }
        } else {
            //rad_angle = polarSensor->widthX*; //M_PI*sensor.flare_angle/180;
            rad_steps = polarSensor->getCols(); //rad_angle/(sReal)(sensor.resolution-1);
            rad_start = -((rad_steps-1)/2.0)*polarSensor->stepX; //Starting to Left, because 0 is in front and rock convention posive CCW //(M_PI-rad_angle)/2;
            if(rad_steps == 1){
              rad_start = 0;
            }
            for(i=0; i<rad_steps; i++) {
              gd = new geom_data;
              (*gd).setZero();
              gd->sense_contact_force = 0;
              (*polarSensor)[i] = gd->value = polarSensor->maxDistance;//sensor.max_distance;

              gd->ray_sensor = 1;
              gd->parent_geom = nGeom;
              gd->parent_body = nBody;
              sle.geom = dCreateRay(NULL, polarSensor->maxDistance);
              dGeomRaySetClosestHit(sle.geom, 1);
              dGeomSetCollideBits(sle.geom, 32768);
              dGeomSetCategoryBits(sle.geom, 32768);
              direction = Vector(cos(rad_start+i*polarSensor->stepX),
                                 sin(rad_start+i*polarSensor->stepX), 0);
              //direction = QVRotate(sensor.rotation, direction);
              direction = (polarSensor->getOrientation() * direction);
              sle.ray_direction = direction;
              tmp[0] = direction.x();
              tmp[1] = direction.y();
              tmp[2] = direction.z();
              dMULTIPLY0_331(dest, rot, tmp);
              dGeomRaySet(sle.geom, pos[0], pos[1], pos[2], dest[0], dest[1], dest[2]);
              sle.gd = gd;
              sle.index = i;
              sensor_list.push_back(sle);
              dGeomSetData(sle.geom, gd);
              //dGeomRaySetParams(sle.geom, 1, 1);
              dGeomSetCollideBits(sle.geom, COLLIDE_MASK_SENSOR);
              dGeomSetCategoryBits(sle.geom, COLLIDE_MASK_SENSOR);
              dGeomDisable(sle.geom);
            }
        }
      }
  
      BaseGridIntersectionSensor *polarGridSensor;
      polarGridSensor = dynamic_cast<BaseGridIntersectionSensor*>(sensor);

      if(polarGridSensor){
        sle.sensor = sensor;
        sle.updateTime = 0.0;
        int cols, rows;
        dVector3 dir={0,0,0,0}, xStep={0,0,0,0}, 
            yStep={0,0,0,0}, xOffset={0,0,0,0}, yOffset={0,0,0,0};

        cols = polarGridSensor->getCols();
        rows = polarGridSensor->getRows();
    
        tmp[0] = 0;
        tmp[1] = 0;
        tmp[2] = -1;
        dMULTIPLY0_331(dir, rot, tmp);
        tmp[0] = polarGridSensor->stepX;
        tmp[1] = 0;
        tmp[2] = 0;
        //dMULTIPLY0_331(xStep, rot, tmp);
        tmp[0] = 0;
        tmp[1] = polarGridSensor->stepY;
        tmp[2] = 0;
        //dMULTIPLY0_331(yStep, rot, tmp);


        for(int x=0; x<cols; x++) {
          for(int y=0; y<rows; y++) {
            gd = new geom_data;
            (*gd).setZero();
            gd->sense_contact_force = 0;
            (*polarGridSensor)[y*cols+x] = gd->value = polarGridSensor->maxDistance;
      
            gd->ray_sensor = 1;
            gd->parent_geom = nGeom;
            sle.geom = dCreateRay(theWorld->getSpace(),
                                  polarGridSensor->maxDistance);
            dGeomRaySetClosestHit(sle.geom, 1);
            dGeomSetCollideBits(sle.geom, 32768);
            dGeomSetCategoryBits(sle.geom, 32768);
        
            direction = (polarGridSensor->getOrientation() *
                         Vector(0.0, 0.0, -1.0));
            sle.ray_direction = direction;
            tmp[0] = direction.x();
            tmp[1] = direction.y();
            tmp[2] = direction.z();
            dMULTIPLY0_331(dest, rot, tmp);
            xOffset[0] =  -cols*0.5*xStep[0] + x*xStep[0];
            xOffset[1] =  -cols*0.5*xStep[1] + x*xStep[1];
            xOffset[2] =  -cols*0.5*xStep[2] + x*xStep[2];

            yOffset[0] =  -rows*0.5*yStep[0] + y*yStep[0];
            yOffset[1] =  -rows*0.5*yStep[1] + y*yStep[1];
            yOffset[2] =  -rows*0.5*yStep[2] + y*yStep[2];
        
            sle.ray_pos_offset.x() = xOffset[0] + yOffset[0];
            sle.ray_pos_offset.y() = xOffset[1] + yOffset[1];
            sle.ray_pos_offset.z() = xOffset[2] + yOffset[2];

            dGeomRaySet(sle.geom, pos[0] + xOffset[0] + yOffset[0],
                        pos[1] + xOffset[1] + yOffset[1],
                        pos[2]  + xOffset[2] + yOffset[2],
                        dest[0], dest[1], dest[2]);
            sle.gd = gd;
            sle.index = y*cols+x;
            sensor_list.push_back(sle);      
            dGeomSetData(sle.geom, gd);
            //dGeomRaySetParams(sle.geom, 1, 1);      
            dGeomSetCollideBits(sle.geom, COLLIDE_MASK_SENSOR);
            dGeomSetCategoryBits(sle.geom, COLLIDE_MASK_SENSOR);
            dGeomDisable(sle.geom);
        
          }
        }
      }
    }

    void ODEObject::removeSensor(BaseSensor *sensor) {
      MutexLocker locker(&(theWorld->iMutex));
      std::vector<sensor_list_element>::iterator iter;
      for (iter = sensor_list.begin(); iter != sensor_list.end(); ) {
        if (iter->sensor == sensor) {
          free(iter->gd);
          dGeomDestroy(iter->geom);
          iter = sensor_list.erase(iter);
        } else
          ++iter;
      }
    }

    /**
     * \brief This function copies all sensor values to the specific allocated
     *  memory
     *
     * pre:
     *
     * post:
     */
    void ODEObject::handleSensorData(bool physics_thread) {
      if(!physics_thread) return;
      MutexLocker locker(&(theWorld->iMutex));
      std::vector<sensor_list_element>::iterator iter;
      const dReal* pos = dGeomGetPosition(nGeom);
      const dReal* rot = dGeomGetRotation(nGeom);
      dVector3 dest, tmp, posOffset;
      dReal steps_size = 1.0, length = 0.0;
      bool done = false;
      int steps = 0;
      dReal worldStep = theWorld->getWorldStep();
      // RotatingRaySensor
      utils::Vector tmpV;
      utils::Quaternion turnrotation;
      turnrotation.setIdentity();
      std::set<unsigned long> ids_rotating_ray_sensors;

      //New Code
      int i=0;
      for(iter = sensor_list.begin(); iter != sensor_list.end(); iter++) {
        i+=1;
        if((double)iter->sensor->updateRate * 0.001 > worldStep) {
          iter->updateTime += worldStep;
          if(iter->updateTime < 0.001*iter->sensor->updateRate) continue;
          iter->updateTime -= 0.001*iter->sensor->updateRate;
        }
        BasePolarIntersectionSensor *polarSensor = dynamic_cast<BasePolarIntersectionSensor*>((*iter).sensor);
        if(polarSensor){
          sensor_list_element elem = *iter;

          tmpV[0] = elem.ray_direction.x();
          tmpV[1] = elem.ray_direction.y();
          tmpV[2] = elem.ray_direction.z();
          
          // Applies orientation_offset (z-Rotation) to the laser rays.
          mars::sim::RotatingRaySensor *rotRaySensor = dynamic_cast<RotatingRaySensor*>((*iter).sensor);
          if(rotRaySensor){
              std::set<unsigned long>::iterator it = ids_rotating_ray_sensors.find(rotRaySensor->id);
              // Takes care that each rotating ray sensor is only turned once (sensor_list contains each ray independently).
              if(it == ids_rotating_ray_sensors.end()) {
                  //turnrotation.setIdentity(); // If we set it to identity first, receiveSensorData will not be called anymore.. wtf!?
                  turnrotation = rotRaySensor->turn();
                  ids_rotating_ray_sensors.insert(rotRaySensor->id);
              }
              //fprintf(stderr, "tmp[%i]: %f, %f, %f\n", i, tmpV.x(), tmpV.y(), tmpV.z());
              tmpV = turnrotation * tmpV;
              //fprintf(stderr, "tmp[%i]: %f, %f, %f\n", i, tmpV.x(), tmpV.y(), tmpV.z());
          }
          tmp[0] = tmpV.x();//elem.ray_direction.x();
          tmp[1] = tmpV.y();//elem.ray_direction.y();
          tmp[2] = tmpV.z();//elem.ray_direction.z();
          dMULTIPLY0_331(dest, rot, tmp);

          if(physics_thread) {
            steps = 0;
            length = 0.0;
            done = false;
            dGeomEnable(elem.geom);
            // make here the collision check
            while (!done) {
              dGeomRaySet(elem.geom,
                          pos[0] + dest[0]*steps_size*steps,
                          pos[1] + dest[1]*steps_size*steps,
                          pos[2] + dest[2]*steps_size*steps,
                          dest[0], dest[1], dest[2]);
              if(length + steps_size < polarSensor->maxDistance) {
                steps++;
                dGeomRaySetLength(elem.geom, steps_size);
              }
              else {
                dGeomRaySetLength(elem.geom, polarSensor->maxDistance- length);
                done = true;
              }
              if(theWorld->handleCollision(elem.geom)) {
                elem.gd->value += length;
                done = true;
              }
              if(!done) length = steps_size*steps;
            }
            dGeomDisable(elem.geom);
          }
          (*polarSensor)[elem.index] = elem.gd->value;
          elem.gd->value = polarSensor->maxDistance;
        }
    
        BaseGridIntersectionSensor *polarGridSensor;
        polarGridSensor = dynamic_cast<BaseGridIntersectionSensor*>(iter->sensor);

        if(polarGridSensor) {
          sensor_list_element elem = *iter;

          if(physics_thread) {
        
            tmp[0] = elem.ray_direction.x();
            tmp[1] = elem.ray_direction.y();
            tmp[2] = elem.ray_direction.z();
            dMULTIPLY0_331(dest, rot, tmp);

            tmp[0] = elem.ray_pos_offset.x();
            tmp[1] = elem.ray_pos_offset.y();
            tmp[2] = elem.ray_pos_offset.z();
            dMULTIPLY0_331(posOffset, rot, tmp);

            dGeomEnable(elem.geom);

            dGeomRaySet(elem.geom, pos[0] + posOffset[0],
                        pos[1] + posOffset[1],
                        pos[2] + posOffset[2],
                        dest[0], dest[1], dest[2]);

            dGeomRaySetLength(elem.geom, polarGridSensor->maxDistance);
            theWorld->handleCollision(elem.geom);
            dGeomDisable(elem.geom);        
          }
          (*polarGridSensor)[elem.index] = elem.gd->value;
          elem.gd->value = polarGridSensor->maxDistance;      
        }
      } // end for loop.
    }

    /**
     * \brief destroyes a node from the physics
     *
     * pre:
     *
     * post:
     */
    void ODEObject::destroyNode(void) {
      MutexLocker locker(&(theWorld->iMutex));
      if(nBody) theWorld->destroyBody(nBody, this);
      if(nGeom) dGeomDestroy(nGeom);
      nBody = 0;
      nGeom = 0;
      composite = false;
      //node_data.num_ground_collisions = 0;
      node_data.setZero();
      std::cout << "Destroyed Parent" << std::endl;
    }

    void ODEObject::setInertiaMass(NodeData* node) {

      dMassSetZero(&nMass);
      nMass.mass = (dReal)node->mass;
      nMass.I[0] = (dReal)node->inertia[0][0];
      nMass.I[1] = (dReal)node->inertia[0][1];
      nMass.I[2] = (dReal)node->inertia[0][2];
      nMass.I[3] = 0.0;

      nMass.I[4] = (dReal)node->inertia[1][0];
      nMass.I[5] = (dReal)node->inertia[1][1];
      nMass.I[6] = (dReal)node->inertia[1][2];
      nMass.I[7] = 0.0;

      nMass.I[8] = (dReal)node->inertia[2][0];
      nMass.I[9] = (dReal)node->inertia[2][1];
      nMass.I[10] = (dReal)node->inertia[2][2];
      nMass.I[11] = 0.0;
    }

    void ODEObject::getMass(sReal *mass, sReal *inertia) const {
      if(mass) *mass = nMass.mass;
      if(inertia) {
        inertia[0] = nMass.I[0];
        inertia[1] = nMass.I[1];
        inertia[2] = nMass.I[2];
        inertia[3] = nMass.I[4];
        inertia[4] = nMass.I[5];
        inertia[5] = nMass.I[6];
        inertia[6] = nMass.I[8];
        inertia[7] = nMass.I[9];
        inertia[8] = nMass.I[10];
      }
    }

    std::vector<dJointFeedback*> ODEObject::addContacts(ContactsPhysics contacts, dWorldID world, dJointGroupID contactgroup){
      dVector3 v;
      //dMatrix3 R;
      dReal dot;
      std::vector<dJointFeedback*> fbs; 
      std::shared_ptr<std::vector<dContact>> contactsPtr = contacts.contactsPtr;
      const smurf::ContactParams contactParams = contacts.collidable->getContactParams(); 
      std::string nodeName = contacts.collidable->getName();
      for(int i=0; i<contacts.numContacts; i++){
        /*
          TODO: check how friction direction1 has to be properly set. Currently
          the setting is hardcoded and it is needed for succesful interaction
        */
        //if(contactParams.friction_direction1) { // For now it won't go in here
        v[0] = contactsPtr->operator[](i).geom.normal[0];
        v[1] = contactsPtr->operator[](i).geom.normal[1];
        v[2] = contactsPtr->operator[](i).geom.normal[2];
        dot = dDOT(v, contactsPtr->operator[](i).fdir1);
        dOPEC(v, *=, dot);
        contactsPtr->operator[](i).fdir1[0] -= v[0];
        contactsPtr->operator[](i).fdir1[1] -= v[1];
        contactsPtr->operator[](i).fdir1[2] -= v[2];
        //dNormalize3(contactsPtr->operator[](0).fdir1); // Why 0 and not i?
        dNormalize3(contactsPtr->operator[](i).fdir1); 
        //}
        contactsPtr->operator[](i).geom.depth += (contactParams.depth_correction); 
        /*
        // TODO: If after further testing this is not needed, then remove.
        // Otherwise use a parameter for this
        if(contactsPtr->operator[](i).geom.depth > 0.001)
        {
          LOG_DEBUG("[ODEObject::createContacts]:The colision detected might have too large depth, reducing it to the maxmimum allowed");
          std::cout << "[ODEObject::createContacts]: Depth " << contactsPtr->operator[](i).geom.depth << std::endl;
          contactsPtr->operator[](i).geom.depth = 0.001; 
        }
        */
        if(contactsPtr->operator[](i).geom.depth < 0.0) contactsPtr->operator[](i).geom.depth = 0.0; 
        //contactsPtr->operator[](0).geom.depth += (contactParams.depth_correction); // Why 0 and not i?
        contactsPtr->operator[](i).geom.depth += (contactParams.depth_correction); 
        //if(contactsPtr->operator[](0).geom.depth < 0.0) contactsPtr->operator[](0).geom.depth = 0.0; // Why 0 and not i ?
        dJointID c=dJointCreateContact(world, contactgroup, &contactsPtr->operator[](i));
        dJointFeedback *fb;
        fb = (dJointFeedback*)malloc(sizeof(dJointFeedback));
        dJointSetFeedback(c, fb); // ODE
        #ifdef DEBUG_ADD_CONTACTS
          if (isnan(abs(fb->f1[0]))||isnan(abs(fb->f1[1]))||isnan(abs(fb->f1[2])) || isnan(abs(fb->f1[3])) ||
            isnan(abs(fb->t1[0]))||isnan(abs(fb->t1[1]))||isnan(abs(fb->t1[2])) || isnan(abs(fb->t1[3])) ||
            isnan(abs(fb->f2[0]))||isnan(abs(fb->f2[1]))||isnan(abs(fb->f2[2])) || isnan(abs(fb->f2[3])) ||
            isnan(abs(fb->t2[0]))||isnan(abs(fb->t2[1]))||isnan(abs(fb->t2[2])) || isnan(abs(fb->t2[3])) )
          {
            LOG_WARN("Catched a nan in the feedback forces, won't be used");
          }
          LOG_DEBUG("[ODEObject::addContacts] Contacts were added to the node %s", nodeName.c_str());
        #endif
        fbs.push_back(fb);
        addContact(c, contactsPtr->operator[](i), fb); 
      } // for numContacts
      return fbs;
    }

    void ODEObject::addContact(dJointID contactJointId, dContact contact, dJointFeedback* fb){
      Vector contact_point;
      dJointAttach(contactJointId, nBody, 0);
      node_data.num_ground_collisions ++; 
      contact_point.x() = contact.geom.pos[0];
      contact_point.y() = contact.geom.pos[1];
      contact_point.z() = contact.geom.pos[2];
      node_data.contact_ids.push_back(0);
      node_data.contact_points.push_back(contact_point);
      node_data.ground_feedbacks.push_back(fb);
      node_data.node1 = true;
    }


    sReal ODEObject::getCollisionDepth(void) const {
      if(nGeom && theWorld) {
        return theWorld->getCollisionDepth(nGeom);
      }
      return 0.0;
    }

} // end of namespace sim
} // end of namespace mars
