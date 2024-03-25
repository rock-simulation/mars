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
 * \file ODEObject.h
 * \author Malte Roemmermann, Leon Danter, Muhammad Haider Khan Lodhi
 * \brief "ODEObject" implements an ODEObject as parent of ode objects.
 *
 */

#ifndef ODE_OBJECT_H
#define ODE_OBJECT_H

#ifdef _PRINT_HEADER_
  #warning "ODEObject.h"
#endif

#include "WorldPhysics.h"

#include <mars/interfaces/sim/NodeInterface.h>

#ifndef ODE11
  #define dTriIndex int
#endif

//TODO move struct descriptions to seperate file!
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
        filter_depth = -1.;
        filter_angle = -1.;
        filter_radius = -1.0;
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
      dReal filter_depth, filter_angle, filter_radius;
      utils::Vector filter_sphere;
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

    class ODEObject : public interfaces::NodeInterface {
    public:
      ODEObject(std::shared_ptr<interfaces::PhysicsInterface> world, interfaces::NodeData * nodeData);
      virtual ~ODEObject(void);

      virtual bool createNode(interfaces::NodeData *node) override;
      virtual bool changeNode(interfaces::NodeData *node) override;
      virtual bool createODEGeometry(interfaces::NodeData *node);       
      virtual void getPosition(utils::Vector *pos) const;
      virtual const utils::Vector setPosition(const utils::Vector &pos, bool move_group);
      virtual void getRotation(utils::Quaternion *q) const;
      virtual const utils::Quaternion setRotation(const utils::Quaternion &q, bool move_group);
      virtual void setWorldObject(std::shared_ptr<interfaces::PhysicsInterface> world);
      virtual void getLinearVelocity(utils::Vector *vel) const;
      virtual void getAngularVelocity(utils::Vector *vel) const;
      virtual void getForce(utils::Vector *f) const;
      virtual void getTorque(utils::Vector *t) const;
      virtual const utils::Vector rotateAtPoint(const utils::Vector &rotation_point,
                                                const utils::Quaternion &rotation, 
                                                bool move_group);
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
      virtual void addContact(dJointID contactJointId, dContact contact, dJointFeedback* fb);
      virtual std::vector<dJointFeedback*> addContacts(ContactsPhysics contacts, dWorldID world, dJointGroupID contactgroup);
      virtual interfaces::sReal getCollisionDepth(void) const;
      void addCompositeOffset(dReal x, dReal y, dReal z);
      ///return the body; this function is created to make it possible to get the 
      ///body from joint physics s
      dBodyID getBody() const;
      dMass getODEMass(void) const;
      void addMassToCompositeBody(dBodyID theBody, dMass *bodyMass);
      void getAbsMass(dMass *pMass) const;
      bool isObjectCreated();

    protected:
      std::shared_ptr<WorldPhysics> theWorld;
      dBodyID nBody;
      dGeomID nGeom;
      dMass nMass;
      bool composite;
      geom_data node_data;
      std::vector<sensor_list_element> sensor_list;
      void setProperties(interfaces::NodeData *node);
      void setInertiaMass(interfaces::NodeData *node);
      
    private:  
      bool object_created;
    };

} // end of namespace sim
} // end of namespace mars

#endif  // NODE_PHYSICS_H
