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

 /**
 * \file NodePhysics.h
 * \author Malte Roemmermann
 * \brief "NodePhysics" implements the physical ode stuff for the nodes.
 *
 */

#ifndef NODE_PHYSICS_H
#define NODE_PHYSICS_H

#ifdef _PRINT_HEADER_
  #warning "NodePhysics.h"
#endif

#include "WorldPhysics.h"

#include <mars/interfaces/sim/NodeInterface.h>

#ifndef ODE11
  #define dTriIndex int
#endif

namespace mars {
  namespace sim {

    /*
     * we need a data structure to handle different collision parameter
     * and we need to save the collision_data somewhere
     * in this first version, it is a hack to get the ground collisions 
     */
    struct geom_data {
      void setZero(){
        num_ground_collisions = 0;
        ray_sensor = 0;
        sense_contact_force = 1;
        value = 0;
        c_params.setZero();
      }

      geom_data(){
        setZero();
      }
      unsigned long id;
      int num_ground_collisions;
      std::vector<utils::Vector> contact_points;
      std::list<unsigned long> contact_ids;
      std::vector<dJointFeedback*> ground_feedbacks;
      bool node1;
      interfaces::contact_params c_params;
      bool ray_sensor;
      bool sense_contact_force;
      interfaces::sReal value;
      dGeomID parent_geom;
      dBodyID parent_body;
    };

    struct sensor_list_element {
      interfaces::BaseSensor *sensor;
      geom_data *gd;
      dGeomID geom;
      utils::Vector ray_direction;
      utils::Vector ray_pos_offset;
      unsigned int index;
      dReal updateTime;
    };

    /**
     * The class that implements the NodeInterface interface.
     *
     */
    class NodePhysics : public interfaces::NodeInterface {
    public:
      NodePhysics(interfaces::PhysicsInterface *world);
      virtual ~NodePhysics(void);
      virtual bool createNode(interfaces::NodeData *node);
      virtual void getPosition(utils::Vector *pos) const;
      virtual const utils::Vector setPosition(const utils::Vector &pos, bool move_group);
      virtual void getRotation(utils::Quaternion *q) const;
      virtual const utils::Quaternion setRotation(const utils::Quaternion &q, bool move_group);
      virtual void setWorldObject(interfaces::PhysicsInterface *world);
      virtual void getLinearVelocity(utils::Vector *vel) const;
      virtual void getAngularVelocity(utils::Vector *vel) const;
      virtual void getForce(utils::Vector *f) const;
      virtual void getTorque(utils::Vector *t) const;
      virtual const utils::Vector rotateAtPoint(const utils::Vector &rotation_point,
                                                const utils::Quaternion &rotation, 
                                                bool move_group);
      virtual bool changeNode(interfaces::NodeData *node);
      virtual void setLinearVelocity(const utils::Vector &velocity);
      virtual void setAngularVelocity(const utils::Vector &velocity);
      virtual void setForce(const utils::Vector &f);
      virtual void setTorque(const utils::Vector &t);
      virtual void addForce(const utils::Vector &f, const utils::Vector &p);
      virtual void addForce(const utils::Vector &f);
      virtual void addTorque(const utils::Vector &t);
      virtual bool getGroundContact(void) const;
      virtual void getContactPoints(std::vector<utils::Vector> *contact_points) const;
      virtual void getContactIDs(std::list<interfaces::NodeId> *ids) const;
      virtual interfaces::sReal getGroundContactForce(void) const;
      virtual void setContactParams(interfaces::contact_params &c_params);
      virtual void addSensor(interfaces::BaseSensor *sensor);
      virtual void removeSensor(interfaces::BaseSensor *sensor);
      virtual void handleSensorData(bool physics_thread = true);
      virtual void destroyNode(void);
      virtual void getMass(interfaces::sReal *mass, interfaces::sReal *inertia=0) const;
      virtual const utils::Vector getContactForce(void) const;
      virtual interfaces::sReal getCollisionDepth(void) const;
      void addCompositeOffset(dReal x, dReal y, dReal z);
      ///return the body; this function is created to make it possible to get the 
      ///body from joint physics s
      dBodyID getBody() const;
      dMass getODEMass(void) const;
      void addMassToCompositeBody(dBodyID theBody, dMass *bodyMass);
      void getAbsMass(dMass *pMass) const;
      dReal heightCallback(int x, int y);

    protected:
      WorldPhysics *theWorld;
      dBodyID nBody;
      dGeomID nGeom;
      dMass nMass;
      dVector3 *myVertices;
      dTriIndex *myIndices;
      dTriMeshDataID myTriMeshData;
      bool composite;
      geom_data node_data;
      interfaces::terrainStruct *terrain;
      dReal *height_data;
      std::vector<sensor_list_element> sensor_list;
      bool createMesh(interfaces::NodeData *node);
      bool createBox(interfaces::NodeData *node);
      bool createSphere(interfaces::NodeData *node);
      bool createCapsule(interfaces::NodeData *node);
      bool createCylinder(interfaces::NodeData *node);
      bool createPlane(interfaces::NodeData *node);
      bool createHeightfield(interfaces::NodeData *node);
      void setProperties(interfaces::NodeData *node);
      void setInertiaMass(interfaces::NodeData *node);
    };

  } // end of namespace sim
} // end of namespace mars

#endif  // NODE_PHYSICS_H
