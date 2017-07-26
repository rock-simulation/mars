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

    /**
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

      
      // getter
      interfaces::sReal getDensity(void) const; ///< Returns the density of the node.
      interfaces::sReal getMass(void) const; ///< Returns the mass of the node.
      const interfaces::snmesh getMesh(void) const; ///< Returns the mesh of the node.
      const std::string getName(void) const; ///<
      interfaces::NodeType getPhysicMode(void) const;
      const utils::Vector getPosition(void) const;
      const utils::Vector getVisualPosition(void) const;
      const utils::Quaternion getRotation(void) const; ///< Returns the rotation of the node.
      const utils::Quaternion getVisualRotation(void) const;
      const utils::Vector getLinearVelocity(void) const;
      const utils::Vector getAngularVelocity(void) const;
      const utils::Vector getLinearAcceleration(void) const;
      const utils::Vector getAngularAcceleration(void) const;
      const utils::Vector getForce(void) const;
      const utils::Vector getTorque(void) const;
      //int getSpace(void); ///< Returns the collision space of the node.
      const std::string getTexture(void) const; ///< Returns the name of the nodes texture.
      const utils::Vector getExtent(void) const; ///< returns the bounding extent of the node
      bool isMovable(void) const; ///< returns if node is a movable node
      unsigned long getGraphicsID2(void) const;
      int getGroupID(void) const;
      const interfaces::NodeData getSNode(void) const; ///< Returns a pointer to the sNode.
      interfaces::NodeInterface* getInterface(void) const; ///< Gets the node interface object.
      const interfaces::MaterialData getMaterial(void) const;
      unsigned long getID(void) const; ///< Returns the node ID.
      void getCoreExchange(interfaces::core_objects_exchange *obj) const;
      void getPhysicalState(interfaces::nodeState *state) const;
      bool getGroundContact(void) const;      
      void getMass(interfaces::sReal *mass, interfaces::sReal *inertia) const;
      void getContactPoints(std::vector<utils::Vector> *contact_points) const;
      void getContactIDs(std::list<interfaces::NodeId> *ids) const;
      int getVisualRep(void) const;
      void getDataBrokerNames(std::string *groupName, std::string *dataName) const;
      double getCollisionDepth(void) const;
      const interfaces::contact_params getContactParams() const;
      const utils::Vector getContactForce(void) const;
      interfaces::sReal getGroundContactForce(void) const;
      
      // setter
      void setDensity(interfaces::sReal objectdensity); ///< Sets the density of the node.
      void setExtent(const utils::Vector &ext, bool update = false); ///< sets the extend of the node
      void setMass(interfaces::sReal objectmass); ///< Sets the mass of the node.
      void setMesh(const interfaces::snmesh &objectmesh); ///< Sets the mesh of the node.
      void setName(const std::string &objectname); ///< Sets the name of the node.
      void setGraphicsID(unsigned long g_id);
      unsigned long getGraphicsID(void) const;
      void setGraphicsID2(unsigned long g_id);
      void setPhysicMode(interfaces::NodeType mode); ///< Sets the physic mode of the node. See getPhysicMode for list of modes.
      const utils::Vector setPosition(const utils::Vector &newPosition, bool move_group); ///< Sets the position of the node.
      void setPositionOffset(const utils::Vector &offset);
      const utils::Quaternion setRotation(const utils::Quaternion &rotation, bool move_all); ///< Sets the rotation of the node.
      //void setSpace(int mySpace); ///< Sets the collision space of the node.
      void setMovable(bool movable); ///< Set the node movable/non-movable
      void setColor(utils::Vector v);
      void setTexture(const std::string &tname); ///< Sets the node's texture name.
      void setMaterial(const interfaces::MaterialData &material);
      void setPhysicalState(const interfaces::nodeState &state);
      void setFromSNode(const interfaces::NodeData &sNode);
      void setInterface(interfaces::NodeInterface *_interface); ///< Sets the node interface object.
      void setRelativePosition(const interfaces::NodeData &node);
      void setContactParams(const interfaces::contact_params &cp);
      void setLinearVelocity(const utils::Vector &vel);
      void setAngularVelocity(const utils::Vector &vel);
      void setAngularDamping(interfaces::sReal damping);
      void setVisualRep(int val);
      void setContactMotion1(interfaces::sReal motion);
      void setMaterialName(const std::string &name) {sNode.material.name = name;}
      void setRelativeID(interfaces::NodeId id) {sNode.relative_id = id;}
      inline void setVisQOffset(utils::Quaternion q) {
        sNode.visual_offset_rot = q;
      }

      
      // manipulation
      void update(interfaces::sReal calc_ms, bool physics_thread = true); ///< Updates the values of the node from the physical layer.
      void rotateAtPoint(const utils::Vector &rotation_point, const utils::Quaternion &rotation, bool move_group);
      void changeNode(interfaces::NodeData *node);
      void clearRelativePosition(void);
      void applyForce(const utils::Vector &force, const utils::Vector &pos);
      void applyForce(const utils::Vector &force);
      void applyTorque(const utils::Vector &torque);
      void addSensor(interfaces::BaseSensor *sensor);
      void reloadSensor(interfaces::BaseSensor *s_cfg);
      void removeSensor(interfaces::BaseSensor &s_cfg);
      void addRotation(const utils::Quaternion &q);
      void checkNodeState(void);
      void updateRay(void);
      virtual void produceData(const data_broker::DataInfo &info, data_broker::DataPackage *package, int callbackParam);
      void updatePR(const utils::Vector &pos,
                    const utils::Quaternion &rot,
                    const utils::Vector &visOffsetPos,
                    const utils::Quaternion &visOffsetRot);

      interfaces::NodeId getParentID() {return sNode.relative_id;}
      void setCullMask(int mask);
      void setBrightness(double v);

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

      void addToDataBroker();
      void removeFromDataBroker();

    };

  } // end of namespace sim
} // end of namespace mars

#endif  // SIMNODE_H
