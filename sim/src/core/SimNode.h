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

#ifndef SIMNODE_H
#define SIMNODE_H

#ifdef _PRINT_HEADER_
  #warning "SimNode.h"
#endif

#include <mars/utils/Mutex.h>
#include <mars/utils/Vector.h>
#include <mars/data_broker/ProducerInterface.h>
#include <mars/data_broker/DataPackageMapping.h>
#include <mars/interfaces/sensor_bases.h>
#include <mars/interfaces/nodeState.h>
#include <mars/interfaces/sim/NodeInterface.h>

namespace mars {

  namespace interfaces {
    class ControlCenter;
  }

  namespace sim {

#define BACK_VEL 25

    /*!
     * Two typedefs to keep mesh structure nearly similar to ODE.
     * original ODE structure is not being used here to keep the possibility to change physic engine
     *
     * Each SimNode object publishes its state on the dataBroker.
     * The name under which the data is published can be obtained from the 
     * nodeId via NodeManager::getDataBrokerNames.
     * The data_broker::DataPackage will contain the following items:
     *  - "id" (int)
     *  - "position/x" (double)
     *  - "position/y" (double)
     *  - "position/z" (double)
     *  - "rotation/x" (double)
     *  - "rotation/y" (double)
     *  - "rotation/z" (double)
     *  - "rotation/w" (double)
     *  - "linearVelocity/x" (double)
     *  - "linearVelocity/y" (double)
     *  - "linearVelocity/z" (double)
     *  - "angularVelocity/x" (double)
     *  - "angularVelocity/y" (double)
     *  - "angularVelocity/z" (double)
     *  - "linearAcceleration/x" (double)
     *  - "linearAcceleration/y" (double)
     *  - "linearAcceleration/z" (double)
     *  - "angularAcceleration/x" (double)
     *  - "angularAcceleration/y" (double)
     *  - "angularAcceleration/z" (double)
     *  - "force/x" (double)
     *  - "force/y" (double)
     *  - "force/z" (double)
     *  - "torque/x" (double)
     *  - "torque/y" (double)
     *  - "torque/z" (double)
     *  - "groundContact" (bool)
     *  - "groundContactForce" (double)
     */

    class SimNode : public data_broker::ProducerInterface {

    public:
      SimNode(interfaces::ControlCenter *c, const interfaces::NodeData &sNode);
      //SimNode(ControlCenter *c, unsigned long index);
      ~SimNode(void);

      /**
       * returns the density of the node
       * @return density of node
       */
      interfaces::sReal getDensity(void) const;

      /**
       * returns the mass of the node
       * @return mass of node
       */
      interfaces::sReal getMass(void) const;

      /**
       * returns the mesh of the node
       * @return mesh of node
       */
      const interfaces::snmesh getMesh(void) const;

      /**
       * returns the name of the node
       * @return name of node
       */
      const std::string getName(void) const;

      /**
       * returns the physic mode of the node as an integer
       * @return physic mode of node
       */
      interfaces::NodeType getPhysicMode(void) const;

      /**
       * returns the position of the node
       * @return position of node
       */
      const utils::Vector getPosition(void) const;
      const utils::Vector getVisualPosition(void) const;

      /**
       * returns the rotation of the node
       * @return rotation of node
       */
      const utils::Quaternion getRotation(void) const;
      const utils::Quaternion getVisualRotation(void) const;

      const utils::Vector getLinearVelocity(void) const;
      const utils::Vector getAngularVelocity(void) const;
      const utils::Vector getLinearAcceleration(void) const;
      const utils::Vector getAngularAcceleration(void) const;
      const utils::Vector getForce(void) const;
      const utils::Vector getTorque(void) const;

      /**
       * returns the collision space of the node
       * @return collision space of node
       */
      //int getSpace(void);

      /**
       * returns the name of the nodes texture
       * @return name of texture of of node
       */
      const std::string getTexture(void) const;

      /**
       * returns the bounding extent of the node
       * @return extent of node
       */
      const utils::Vector getExtent(void) const;

      /**
       * returns if node is a movable node
       * @return if node is a movable node
       */
      bool isMovable(void) const;


      /**
       * sets the density of the node
       * @param objectdensity density of the node
       */
      void setDensity(interfaces::sReal objectdensity);

      /**
       * sets the extent of the node
       * @param ext extent of the node
       */
      void setExtent(const utils::Vector &ext, bool update = false);

      /**
       * sets the mass of the node
       * @param objectmass mass of the node
       */
      void setMass(interfaces::sReal objectmass);

      /**
       * sets the mesh of the node
       * @param objectmesh mesh of the node
       */
      void setMesh(const interfaces::snmesh &objectmesh);

      /**
       * sets the name of the node
       * @param objectname name of the node
       */
      void setName(const std::string &objectname);
      void setGraphicsID(unsigned long g_id);
      unsigned long getGraphicsID(void) const;
      void setGraphicsID2(unsigned long g_id);
      unsigned long getGraphicsID2(void) const;

      /**
       * sets the physic mode of the node. See getPhysicMode for list of modes
       * @param mode physic mode of the node
       */
      void setPhysicMode(interfaces::NodeType mode);

      inline void setVisQOffset(utils::Quaternion q) {
        sNode.visual_offset_rot = q;
      }

      /**
       * sets the position of the node
       * @param newPosition position of the node
       * @param move_group group of the node
       */
      const utils::Vector setPosition(const utils::Vector &newPosition, 
                                     bool move_group);
  

      void setPositionOffset(const utils::Vector &offset);
      /**
       * sets the rotation of the node
       * @param rotation the quaternion for the node-rotation
       */
      const utils::Quaternion setRotation(const utils::Quaternion &rotation,
                                         bool move_all);

      /**
       * sets the collision space of the node
       * @param collision space of the node
       */
      //void setSpace(int mySpace);

      /**
       * sets if node is an movable node
       * @param movable true if a node is an movable
       */
      void setMovable(bool movable);

      void setColor(utils::Vector v);

      /**
       * sets the nodes texture name
       * @param tname name of the texture
       */
      void setTexture(const std::string &tname);

      void setMaterial(const interfaces::MaterialData &material);
      const interfaces::MaterialData getMaterial(void) const;

      /**
       * returns the nodes id
       * @return id of the node
       */
      unsigned long getID(void) const;

      int getGroupID(void) const;

      void setFromSNode(const interfaces::NodeData &sNode);
      /**
       * returns a pointer to the sNode
       * @return NodeData pointer
       */
      const interfaces::NodeData getSNode(void) const;

      /**
       * sets the node interface object
       * @param _interface new node interface
       */
      void setInterface(interfaces::NodeInterface *_interface);

      /**
       * gets the node interface object
       *
       */
      interfaces::NodeInterface* getInterface(void) const;

      /**
       * updates the values of the node from the physical layer
       *
       */
      void update(interfaces::sReal calc_ms, bool physics_thread = true);

      void getCoreExchange(interfaces::core_objects_exchange *obj) const;
      void rotateAtPoint(const utils::Vector &rotation_point, const utils::Quaternion &rotation,
                         bool move_group);
      void changeNode(interfaces::NodeData *node);
      void setPhysicalState(const interfaces::nodeState &state);
      void getPhysicalState(interfaces::nodeState *state) const;
      bool getGroundContact(void) const;
      interfaces::sReal getGroundContactForce(void) const;
      void clearRelativePosition(void);
      void setRelativePosition(const interfaces::NodeData &node);
      void applyForce(const utils::Vector &force, const utils::Vector &pos);
      void applyForce(const utils::Vector &force);
      void applyTorque(const utils::Vector &torque);
      void setContactMotion1(interfaces::sReal motion);
      void addSensor(interfaces::BaseSensor *sensor);
      void reloadSensor(interfaces::BaseSensor *s_cfg);
      void removeSensor(interfaces::BaseSensor &s_cfg);
      void setContactParams(const interfaces::contact_params &cp);
      const interfaces::contact_params getContactParams() const;
      void setLinearVelocity(const utils::Vector &vel);
      void setAngularVelocity(const utils::Vector &vel);
      void getMass(interfaces::sReal *mass, interfaces::sReal *inertia) const;
      void setAngularDamping(interfaces::sReal damping);
      void addRotation(const utils::Quaternion &q);
      void checkNodeState(void);
      void getContactPoints(std::vector<utils::Vector> *contact_points) const;
      void updateRay(void);
      void setVisualRep(int val);
      int getVisualRep(void) const;
      const utils::Vector getContactForce(void) const;
      double getCollisionDepth(void) const;
      void getDataBrokerNames(std::string *groupName, std::string *dataName) const;

      virtual void produceData(const data_broker::DataInfo &info,
                               data_broker::DataPackage *package,
                               int callbackParam);

      void updatePR(const utils::Vector &pos,
                    const utils::Quaternion &rot,
                    const utils::Vector &visOffsetPos,
                    const utils::Quaternion &visOffsetRot);

      interfaces::NodeId getParentID() {return sNode.relative_id;}

    private:
      interfaces::ControlCenter *control;
      interfaces::NodeData sNode;
      utils::Vector f;
      utils::Vector t;
      utils::Vector l_vel;
      utils::Vector last_l_vel;
      utils::Vector a_vel;
      utils::Vector last_a_vel;
      utils::Vector l_acc;
      utils::Vector a_acc;
      bool ground_contact;
      interfaces::sReal ground_contact_force;
      interfaces::NodeInterface *my_interface;
      bool has_sensor;
      interfaces::sReal i_velocity_sum;
      interfaces::sReal i_velocity[BACK_VEL];
      int vel_ptr;
      unsigned long graphics_id, graphics_id2;
      bool update_ray;
      int visual_rep;
      mutable utils::Mutex iMutex;
      // stuff for dataBroker communication
      data_broker::DataPackageMapping dbPackageMapping;
    };

  } // end of namespace sim
} // end of namespace mars

#endif  // SIMNODE_H
