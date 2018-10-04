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

#include "SimNode.h"

#include <mars/data_broker/DataBrokerInterface.h>
#include <mars/utils/Color.h>
#include <mars/utils/MutexLocker.h>
#include <mars/interfaces/terrainStruct.h>
#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/interfaces/sim/SimulatorInterface.h>
#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>
#include <mars/interfaces/Logging.hpp>

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>

namespace mars {
  namespace sim {

    using std::isinf;
    using std::isnan;
    using std::string;
    using namespace utils;
    using namespace interfaces;

    /**
     * \brief Set all important things.
     *
     * important initial values:
     *       - interface
     */

    /*
      SimNode::SimNode(ControlCenter *c, unsigned long index)
      : control(c) {
      ZERO_NODE_STRUCT(sNode);
      sNode.index = index;
    */
    SimNode::SimNode(ControlCenter *c, const NodeData &sNode_)
      : control(c), sNode(sNode_) {

      fRotation.x() = fRotation.y() = fRotation.z() = 0.0;
      fRotation.w() = 1.0;
      frictionDirNode = 0;
      fDirNode = Vector(1, 0, 0);
      my_interface = 0;
      l_vel = Vector(0.0, 0.0, 0.0);
      a_vel = Vector(0.0, 0.0, 0.0);
      f = Vector(0.0, 0.0, 0.0);
      t = Vector(0.0, 0.0, 0.0);
      ground_contact = 0;
      ground_contact_force = 0;
      i_velocity_sum = 0.0;
      for(int i=0; i<BACK_VEL; i++)
        i_velocity[i] = 0.0;
      vel_ptr = 0;
      graphics_id = 0;
      graphics_id2 = 0;
      update_ray = false;
      visual_rep = 1;

      pushToDataBroker = 2;
      configmaps::ConfigMap &map = sNode.map;
      if(map.hasKey("frictionDirNode")) {
        frictionDirNode = control->nodes->getID((std::string)map["frictionDirNode"]);
        if(frictionDirNode) {
          std::string groupName, dataName;
          control->nodes->getDataBrokerNames(frictionDirNode, &groupName, &dataName);
          control->dataBroker->registerSyncReceiver(this, groupName, dataName, 0);

          if(map.hasKey("fDirInitial")) {
            fDirNode.x() = map["fDirInitial"]["x"];
            fDirNode.y() = map["fDirInitial"]["y"];
            fDirNode.z() = map["fDirInitial"]["z"];
          }
        }
      }
      if(map.hasKey("noDataPackage") && (bool)map["noDataPackage"] == true) {
        pushToDataBroker = 0;
      }
      else if(map.hasKey("reducedDataPackage") && (bool)map["reducedDataPackage"] == true) {
        pushToDataBroker = 1;
      }
      if(pushToDataBroker > 0) {
        dbPackageMapping.add("id", &sNode.index);
        dbPackageMapping.add("position/x", &sNode.pos.x());
        dbPackageMapping.add("position/y", &sNode.pos.y());
        dbPackageMapping.add("position/z", &sNode.pos.z());
        dbPackageMapping.add("rotation/x", &sNode.rot.x());
        dbPackageMapping.add("rotation/y", &sNode.rot.y());
        dbPackageMapping.add("rotation/z", &sNode.rot.z());
        dbPackageMapping.add("rotation/w", &sNode.rot.w());
        dbPackageMapping.add("contact", &ground_contact);
        dbPackageMapping.add("contactForce", &ground_contact_force);
      }
      if(pushToDataBroker > 1) {
        dbPackageMapping.add("linearVelocity/x", &l_vel.x());
        dbPackageMapping.add("linearVelocity/y", &l_vel.y());
        dbPackageMapping.add("linearVelocity/z", &l_vel.z());
        dbPackageMapping.add("angularVelocity/x", &a_vel.x());
        dbPackageMapping.add("angularVelocity/y", &a_vel.y());
        dbPackageMapping.add("angularVelocity/z", &a_vel.z());
        dbPackageMapping.add("linearAcceleration/x", &l_acc.x());
        dbPackageMapping.add("linearAcceleration/y", &l_acc.y());
        dbPackageMapping.add("linearAcceleration/z", &l_acc.z());
        dbPackageMapping.add("angularAcceleration/x", &a_acc.x());
        dbPackageMapping.add("angularAcceleration/y", &a_acc.y());
        dbPackageMapping.add("angularAcceleration/z", &a_acc.z());
        dbPackageMapping.add("force/x", &f.x());
        dbPackageMapping.add("force/y", &f.y());
        dbPackageMapping.add("force/z", &f.z());
        dbPackageMapping.add("torque/x", &t.x());
        dbPackageMapping.add("torque/y", &t.y());
        dbPackageMapping.add("torque/z", &t.z());
      }
      if(pushToDataBroker > 0) {
        addToDataBroker();
      }
    }

    /**
     * \brief Clear a Node and its physically representation.
     *
     * If the node has a physically representation, it has to be cleared as well.
     *
     */
    SimNode::~SimNode(void) {
      MutexLocker locker(&iMutex);
      removeFromDataBroker();
      if(control->dataBroker) {
        control->dataBroker->unregisterSyncReceiver(this, "*", "*");
      }
      if (my_interface) {
        delete my_interface;
        my_interface = 0;
      }
      if (sNode.mesh.vertices) {
        delete[] sNode.mesh.vertices;
        sNode.mesh.vertices = 0;
      }

      if(sNode.mesh.indices){
        delete[] sNode.mesh.indices;
        sNode.mesh.indices = 0;
      }
      if (sNode.c_params.friction_direction1) {
        delete (sNode.c_params.friction_direction1);
        sNode.c_params.friction_direction1 = 0;
      }
      if (sNode.terrain) {
        if(sNode.terrain->pixelData) free(sNode.terrain->pixelData);
        delete sNode.terrain;
        sNode.terrain = 0;
      }
    }


    void SimNode::produceData(const data_broker::DataInfo &info,
                              data_broker::DataPackage *dbPackage,
                              int callbackParam) {
      dbPackageMapping.writePackage(dbPackage);
    }


    void SimNode::setName(const std::string &objectname) {
      MutexLocker locker(&iMutex);
      sNode.name = objectname;
    }

    const std::string SimNode::getName() const {
      MutexLocker locker(&iMutex);
      return sNode.name;
    }

    void SimNode::setGraphicsID(unsigned long g_id) {
      MutexLocker locker(&iMutex);
      graphics_id = g_id;
    }

    unsigned long SimNode::getGraphicsID(void) const {
      MutexLocker locker(&iMutex);
      return graphics_id;
    }

    void SimNode::setGraphicsID2(unsigned long g_id) {
      MutexLocker locker(&iMutex);
      graphics_id2 = g_id;
    }

    unsigned long SimNode::getGraphicsID2(void) const {
      MutexLocker locker(&iMutex);
      return graphics_id2;
    }

    const Vector SimNode::setPosition(const Vector &newPosition,
                                      bool move_group) {
      MutexLocker locker(&iMutex);
      bool update = false;

      if(sNode.pos != newPosition) {
        update = true;
        sNode.pos = newPosition;
      }

      if (my_interface && update) {
        if (sNode.movable) {
          Vector p = my_interface->setPosition(sNode.pos, move_group);
          return p;
        }
        else {
          my_interface->destroyNode();
          if(!my_interface->createNode(&sNode)) {
            LOG_ERROR("SimNode: unhandled error in setPosition");
          }
          // ToDo: return offset of positions
        }
      }
      return Vector(0.0, 0.0, 0.0);
    }


    void SimNode::setPositionOffset(const Vector &offset) {
      setPosition(sNode.pos + offset, 1);
      //if(my_interface) my_interface->setPosition(&sNode.pos, 1);
    }

    const Vector SimNode::getPosition() const {
      MutexLocker locker(&iMutex);
      return sNode.pos;
    }

    const Vector SimNode::getVisualPosition() const {
      MutexLocker locker(&iMutex);
      Vector offset = (sNode.rot * sNode.visual_offset_pos);
      return sNode.pos + offset;
    }

    void SimNode::setVisualRep(int val) {
      MutexLocker locker(&iMutex);
      visual_rep = val;
    }

    int SimNode::getVisualRep(void) const {
      MutexLocker locker(&iMutex);
      return visual_rep;
    }


    const Quaternion SimNode::setRotation(const Quaternion &rotation,
                                          bool move_all) {
      MutexLocker locker(&iMutex);

      sNode.rot = rotation;

      if (my_interface) {
        if (sNode.movable) {
          Quaternion q =  my_interface->setRotation(sNode.rot, move_all);
          return q;
        } else {
          my_interface->destroyNode();
          if(!my_interface->createNode(&sNode)) {
            LOG_ERROR("SimNode: unhandled error in setRotation.");
          }
        }
      }
      return sNode.rot;
    }
    /**
     * \return \c rotation of the node
     */
    const Quaternion SimNode::getRotation() const {
      MutexLocker locker(&iMutex);
      return sNode.rot;
    }

    const Quaternion SimNode::getVisualRotation() const {
      MutexLocker locker(&iMutex);
      return sNode.rot * sNode.visual_offset_rot;
    }

    void SimNode::updatePR(const Vector &pos,
                           const Quaternion &rot,
                           const Vector &visOffsetPos,
                           const Quaternion &visOffsetRot) {

      sNode.visual_offset_pos = visOffsetPos;
      sNode.visual_offset_rot = visOffsetRot;
      setPosition(pos, true);
      setRotation(rot, true);
    }


    const Vector SimNode::getLinearVelocity() const {
      MutexLocker locker(&iMutex);
      return l_vel;
    }
    const Vector SimNode::getAngularVelocity() const {
      MutexLocker locker(&iMutex);
      return a_vel;
    }
    const Vector SimNode::getLinearAcceleration() const {
      MutexLocker locker(&iMutex);
      return l_acc;
    }
    const Vector SimNode::getAngularAcceleration() const {
      MutexLocker locker(&iMutex);
      return a_acc;
    }
    const Vector SimNode::getForce() const {
      MutexLocker locker(&iMutex);
      return f;
    }
    const Vector SimNode::getTorque() const {
      MutexLocker locker(&iMutex);
      return t;
    }

    int SimNode::getGroupID(void) const {
      MutexLocker locker(&iMutex);
      return sNode.groupID;
    }

    void SimNode::setMass(double objectmass){
      MutexLocker locker(&iMutex);
      sNode.mass = objectmass;
    }

    double SimNode::getMass() const {
      MutexLocker locker(&iMutex);
      return sNode.mass;
    }

    void SimNode::setDensity(double objectdensity) {
      MutexLocker locker(&iMutex);
      sNode.density = objectdensity;
    }

    double SimNode::getDensity() const {
      MutexLocker locker(&iMutex);
      return sNode.density;
    }

    void SimNode::setMesh(const snmesh &objectmesh) {
      MutexLocker locker(&iMutex);
      sNode.mesh = objectmesh;
    }

    const snmesh SimNode::getMesh() const {
      MutexLocker locker(&iMutex);
      return sNode.mesh;
    }

    void SimNode::setPhysicMode(NodeType mode) {
      MutexLocker locker(&iMutex);
      sNode.physicMode = mode;
    }

    NodeType SimNode::getPhysicMode() const {
      MutexLocker locker(&iMutex);
      return sNode.physicMode;
    }

    void SimNode::setMovable(bool movable) {
      MutexLocker locker(&iMutex);
      sNode.movable = movable;
    }

    bool SimNode::isMovable() const {
      MutexLocker locker(&iMutex);
      return sNode.movable;
    }

    void SimNode::setTexture(const std::string &tname) {
      MutexLocker locker(&iMutex);
      sNode.material.texturename = tname;
    }

    void SimNode::setMaterial(const MaterialData &material) {
      MutexLocker locker(&iMutex);
      sNode.material = material;
    }

    const MaterialData SimNode::getMaterial(void) const {
      MutexLocker locker(&iMutex);
      return sNode.material;
    }

    /**
     * \return name of texture of of node
     */
    const std::string SimNode::getTexture() const {
      MutexLocker locker(&iMutex);
      return sNode.material.texturename;
    }

    void SimNode::setExtent(const Vector &ext, bool update) {
      MutexLocker locker(&iMutex);
      sNode.ext = ext;
      if(update){
        if(control->graphics) {
          Vector scale;
          if(sNode.filename == "PRIMITIVE") {
            scale = sNode.ext;
          } else {
            scale = sNode.visual_size;
          }
          control->graphics->setDrawObjectScale(graphics_id, scale);
        }
        if (my_interface) my_interface->changeNode(&sNode);

        /*  //TODO is this really needed?
            nodes = simNodes;
            nodesj = simNodes;
            jointsj = control->joints->getSimJoints();
            nodes.erase(nodes.find(editedNode->getID()));
            resetRelativeNodes(*editedNode, &nodes);
            iMutex.unlock(); // is this desired???
            resetRelativeJoints(*editedNode, &nodesj, &jointsj);
            iMutex.lock();
        */
      }
    }


    const Vector SimNode::getExtent() const {
      MutexLocker locker(&iMutex);
      return sNode.ext;
    }

    void SimNode::setInterface(NodeInterface* _interface) {
      MutexLocker locker(&iMutex);
      my_interface = _interface;
    }

    NodeInterface *SimNode::getInterface(void) const {
      MutexLocker locker(&iMutex);
      return my_interface;
    }

    unsigned long SimNode::getID(void) const {
      MutexLocker locker(&iMutex);
      return sNode.index;
    }


    void SimNode::setFromSNode(const NodeData &sNode_) {
      MutexLocker locker(&iMutex);
      this->sNode.name = sNode.name.c_str();
      this->sNode.origName = sNode.origName.c_str();
      this->sNode.filename = sNode.filename.c_str();
      this->sNode.groupID = sNode.groupID;
      this->sNode.physicMode = sNode.physicMode;
      this->sNode.movable = sNode.movable;
      this->sNode.noPhysical = sNode.noPhysical;
      this->sNode.density = sNode.density;
      this->sNode.mass = sNode.mass;
      this->sNode.mesh.setZero();
      //ZERO_SNMESH(this->sNode.mesh);
      //ZERO_MATERIAL_STRUCT(this->sNode.material);
      this->sNode.material.setZero();
      this->sNode.shaderSources = sNode.shaderSources;
      this->sNode.relative_id = sNode.relative_id;
      this->sNode.terrain = sNode.terrain;
      this->sNode.inertia_set = sNode.inertia_set;
      memcpy(this->sNode.inertia, sNode.inertia, sizeof(this->sNode.inertia));
      this->sNode.linear_damping = sNode.linear_damping;
      this->sNode.angular_damping = sNode.angular_damping;
      this->sNode.angular_treshold = sNode.angular_treshold;
      this->sNode.angular_low = sNode.angular_low;
      this->sNode.shadow_id = sNode.shadow_id;
      this->sNode.isShadowCaster = sNode.isShadowCaster;
      this->sNode.isShadowReceiver = sNode.isShadowReceiver;
      this->sNode.c_params.setZero();
      //  this->sNode = sNode;
      // I think we have to copy the std:strings seperate
      // otherwise we will get problems with the memory
      //this->sNode.name = sNode->name;
      //this->sNode.origName = sNode->origName;
      //this->sNode.filename = sNode->filename;
      //this->sNode.material.texturename = sNode->material.texturename;

      //pos = sNode->pos;
      //rot = sNode->rot;
    }


    /**
     * returns a pointer to the sNode
     * @return NodeData pointer
     */
    const NodeData SimNode::getSNode(void) const {
      MutexLocker locker(&iMutex);
      return sNode;
    }


    void SimNode::setColor(Vector color){
      MutexLocker locker(&iMutex);
      Color c;
      c.r = color[0];
      c.g = color[1];
      c.b = color[2];
      c.a = 1;
      sNode.material.diffuseFront = c;
      sNode.material.diffuseBack  = c;
      sNode.material.ambientFront = c;
      sNode.material.ambientBack  = c;
      sNode.material.emissionFront = c;
      sNode.material.emissionBack  = c;
      sNode.material.specularFront = c;
      sNode.material.specularBack  = c;
      if(control->graphics)
        control->graphics->setDrawObjectMaterial(graphics_id,sNode.material);
    }

    /**
     * pre:
     *     - interface != 0
     *
     */
    void SimNode::update(sReal calc_ms, bool physics_thread) {
      MutexLocker locker(&iMutex);
      if (my_interface) {
        Vector damping;
        sReal d;
        last_l_vel = l_vel;
        last_a_vel = a_vel;
        // update the position and rotation of the node
        my_interface->getPosition(&sNode.pos);
        my_interface->getRotation(&sNode.rot);
        my_interface->getLinearVelocity(&l_vel);
        my_interface->getAngularVelocity(&a_vel);
        my_interface->getForce(&f);
        my_interface->getTorque(&t);
        ground_contact = my_interface->getGroundContact();
        ground_contact_force = my_interface->getGroundContactForce();
        if(calc_ms > 0) {
          l_acc = (l_vel - last_l_vel) / (calc_ms / 1000.);
          a_acc = (a_vel - last_a_vel) / (calc_ms / 1000.);
        } else {
          l_acc = Vector(0, 0, 0);
          a_acc = Vector(0, 0, 0);
        }
        //i_velocity_sum -= i_velocity[vel_ptr];
        //i_velocity[vel_ptr] = fabs(a_vel.length());
        //i_velocity_sum += i_velocity[vel_ptr];
        //d = i_velocity_sum / BACK_VEL;

        //d = fabs(a_vel.length());
        d = fabs(a_vel.norm());

        // here we can handle damping
        if (sNode.linear_damping != 0) {
          damping = l_vel;
          damping *= 1-sNode.linear_damping;
          my_interface->setLinearVelocity(damping);
        }
        if (sNode.angular_treshold && d < sNode.angular_treshold) {
          damping = a_vel;
          /*
               damping.normalize();
               damping *= ((i_velocity[1]-i_velocity[2])*(1-sNode.angular_low)+
               i_velocity[1]);
               //damping *= i_velocity[0];
               */
          damping *= 1-sNode.angular_low;
          //i_velocity_sum -= i_velocity[vel_ptr];
          //i_velocity[vel_ptr] = damping.length();
          //i_velocity_sum += i_velocity[vel_ptr];
          my_interface->setAngularVelocity(damping);
        }
        else if (sNode.angular_damping != 0) {
          damping = a_vel;
          /*damping.normalize();
            damping *= ((i_velocity[1]-i_velocity[2])*(1-sNode.angular_damping)+
            i_velocity[1]);
            //damping *= i_velocity[0];
            */
          damping *= 1-sNode.angular_damping;
          //i_velocity_sum -= i_velocity[vel_ptr];
          //i_velocity[vel_ptr] = damping.length();
          /*
            if(i_velocity[vel_ptr] > sNode.angular_damping) {
            damping.normalize();
            damping *= i_velocity[0] - sNode.angular_damping;
            }
            else {
            damping *= 0;
            }*/
          //i_velocity_sum += i_velocity[vel_ptr];
          my_interface->setAngularVelocity(damping);
        }
        // handle friction direction by mirror node orientation
        if(frictionDirNode && my_interface) {
          Vector v = fRotation*fDirNode;
          if(!sNode.c_params.friction_direction1) {
            sNode.c_params.friction_direction1 = new Vector();
          }
          *(sNode.c_params.friction_direction1) = v;
          my_interface->setContactParams(sNode.c_params);
        }
        //vel_ptr = (vel_ptr+1)%BACK_VEL;
        if(update_ray || true) {
          my_interface->handleSensorData(physics_thread);
          update_ray = false;
        }
        checkNodeState();
      }
    }

    void SimNode::getCoreExchange(core_objects_exchange *obj) const {
      MutexLocker locker(&iMutex);
      obj->index = sNode.index;
      if(sNode.name.length() > 1000) {
        fprintf(stderr, "to long name: %d\n", (int)sNode.name.length());
        obj->name = "";
      }
      else
        obj->name = std::string(sNode.name.c_str());
      obj->groupID = sNode.groupID;
      obj->pos = sNode.pos;
      obj->rot = sNode.rot;

      obj->visOffsetPos = sNode.visual_offset_pos;
      obj->visOffsetRot = sNode.visual_offset_rot;
    }

    void SimNode::rotateAtPoint(const Vector &rotation_point,
                                const Quaternion &rotation,
                                bool move_group) {
      MutexLocker locker(&iMutex);
      //rot = *rotation;
      if (my_interface) {
        sNode.pos = my_interface->rotateAtPoint(rotation_point,
                                                rotation, move_group);
        my_interface->getRotation(&sNode.rot);
      }
    }

    /*
     * \param \c mySpace: collision space of the node
     */
    /*
      void SimNode::setSpace(int mySpace) {
      targetspace = mySpace;
      }
     */

    /**
     * \return collision space of node
     */
    /*
      int SimNode::getSpace() {
      return targetspace;
      }
    */

    void SimNode::changeNode(NodeData *node) {
      MutexLocker locker(&iMutex);
      if (my_interface) my_interface->changeNode(node);
      bool handleDataBroker = false;
      if(sNode.name != node->name) {
        // handle databroker
        removeFromDataBroker();
        handleDataBroker = true;
      }
      sNode = *node;
      if(handleDataBroker) {
        addToDataBroker();
      }
    }

    void SimNode::setPhysicalState(const nodeState &state) {
      MutexLocker locker(&iMutex);
      if (my_interface) {
        my_interface->setLinearVelocity(state.l_vel);
        l_vel = state.l_vel;
        my_interface->setAngularVelocity(state.a_vel);
        a_vel = state.a_vel;
      }
    }

    void SimNode::getPhysicalState(nodeState *state) const {
      MutexLocker locker(&iMutex);
      if (my_interface) {
        state->l_vel = l_vel;
        state->a_vel = a_vel;
      }
    }

    void SimNode::setLinearVelocity(const Vector &vel) {
      MutexLocker locker(&iMutex);
      if (my_interface) {
        my_interface->setLinearVelocity(vel);
        l_vel = vel;
      }
    }

    void SimNode::setAngularVelocity(const Vector &vel) {
      MutexLocker locker(&iMutex);
      if (my_interface) {
        my_interface->setAngularVelocity(vel);
        a_vel = vel;
      }
    }

    bool SimNode::getGroundContact(void) const {
      MutexLocker locker(&iMutex);
      return ground_contact;
    }

    sReal SimNode::getGroundContactForce(void) const {
      MutexLocker locker(&iMutex);
      return ground_contact_force;
    }

    void SimNode::clearRelativePosition(void) {
      MutexLocker locker(&iMutex);
      sNode.relative_id = 0;
    }

    void SimNode::setRelativePosition(const NodeData &node) {
      MutexLocker locker(&iMutex);
      sNode.relative_id = node.relative_id;
      //sNode.pos = node.pos;
      //sNode.rot = node.rot;
    }
    void SimNode::applyForce(const Vector &force, const Vector &pos) {
      MutexLocker locker(&iMutex);
      if (my_interface) my_interface->addForce(force, pos);
    }
    void SimNode::applyForce(const Vector &force) {
      MutexLocker locker(&iMutex);
      if (my_interface) my_interface->addForce(force);
    }

    void SimNode::applyTorque(const Vector &torque) {
      MutexLocker locker(&iMutex);
      if (my_interface) my_interface->addTorque(torque);
    }

    void SimNode::setContactMotion1(sReal motion) {
      MutexLocker locker(&iMutex);
      sNode.c_params.motion1 = motion;
      if (my_interface) my_interface->setContactParams(sNode.c_params);
    }

    void SimNode::addSensor(BaseSensor *s_cfg) {
      MutexLocker locker(&iMutex);
      if (my_interface) my_interface->addSensor(s_cfg);
    }

    void SimNode::reloadSensor(BaseSensor *s_cfg) {
      MutexLocker locker(&iMutex);
      if (my_interface) {
        my_interface->removeSensor(s_cfg);
        my_interface->addSensor(s_cfg);
      }
    }

    const contact_params SimNode::getContactParams() const {
      MutexLocker locker(&iMutex);
      return sNode.c_params;
    }

    void SimNode::setContactParams(const contact_params &cp) {
      MutexLocker locker(&iMutex);
      sNode.c_params = cp;
      if (my_interface) my_interface->setContactParams(sNode.c_params);
    }

    void SimNode::getMass(sReal *mass, sReal *inertia) const {
      MutexLocker locker(&iMutex);
      if (my_interface) my_interface->getMass(mass, inertia);
    }

    void SimNode::setAngularDamping(sReal damping) {
      MutexLocker locker(&iMutex);
      sNode.angular_damping = damping;
    }

    void SimNode::addRotation(const Quaternion &q) {
      MutexLocker locker(&iMutex);
      sNode.rot = q*sNode.rot;
    }

    void SimNode::checkNodeState(void) {
      if (std::isnan(sNode.pos.x())) sNode.pos.x() = 0.0;
      else if (std::isinf(sNode.pos.x())) sNode.pos.x() = 0.0;
      if (std::isnan(sNode.pos.y())) sNode.pos.y() = 0.0;
      else if (std::isinf(sNode.pos.y())) sNode.pos.y() = 0.0;
      if (std::isnan(sNode.pos.z())) sNode.pos.z() = 0.0;
      else if (std::isinf(sNode.pos.z())) sNode.pos.z() = 0.0;
      if (std::isnan(sNode.rot.x())) sNode.rot.x() = 0.0;
      else if (std::isinf(sNode.rot.x())) sNode.rot.x() = 0.0;
      if (std::isnan(sNode.rot.y())) sNode.rot.y() = 0.0;
      else if (std::isinf(sNode.rot.y())) sNode.rot.y() = 0.0;
      if (std::isnan(sNode.rot.z())) sNode.rot.z() = 0.0;
      else if (std::isinf(sNode.rot.z())) sNode.rot.z() = 0.0;
      if (std::isnan(sNode.rot.w())) sNode.rot.w() = 1.0;
      else if (std::isinf(sNode.rot.w())) sNode.rot.w() = 1.0;
    }

    void SimNode::getContactPoints(std::vector<Vector> *contact_points) const {
      MutexLocker locker(&iMutex);
      if(my_interface) {
        my_interface->getContactPoints(contact_points);
      }
    }

    void SimNode::getContactIDs(std::list<interfaces::NodeId> *ids) const {
      MutexLocker locker(&iMutex);
      if(my_interface) {
        my_interface->getContactIDs(ids);
      }
    }

    const Vector SimNode::getContactForce(void) const {
      MutexLocker locker(&iMutex);
      if(my_interface) {
        return my_interface->getContactForce();
      }
      return Vector(0.0, 0.0, 0.0);
    }

    void SimNode::updateRay(void) {
      MutexLocker locker(&iMutex);
      update_ray = true;
    }

    double SimNode::getCollisionDepth(void) const {
      MutexLocker locker(&iMutex);

      if(my_interface) {
        return my_interface->getCollisionDepth();
      }
      return 0.0;
    }

    void SimNode::getDataBrokerNames(std::string *groupName,
                                     std::string *dataName) const {
      char format[] = "Nodes/%05lu_%s";
      int size = snprintf(0, 0, format, sNode.index, sNode.name.c_str());
      char buffer[size];
      sprintf(buffer, format, sNode.index, sNode.name.c_str());
      *groupName = "mars_sim";
      *dataName = buffer;
    }

    void SimNode::setCullMask(int mask) {
      sNode.map["cullMask"] = mask;
      if(control->graphics)
        control->graphics->setDrawObjectNodeMask(graphics_id, mask);
    }

    void SimNode::setBrightness(double v) {
      sNode.map["brightness"] = v;
      if(control->graphics)
        control->graphics->setDrawObjectBrightness(graphics_id, v);
    }

    void SimNode::addToDataBroker() {
      if(control->dataBroker) {
        std::string groupName, dataName;
        getDataBrokerNames(&groupName, &dataName);
        // initialize the dataBroker Package
        data_broker::DataPackage dbPackage;
        dbPackageMapping.writePackage(&dbPackage);
        control->dataBroker->pushData(groupName, dataName, dbPackage, NULL,
                                      data_broker::DATA_PACKAGE_READ_FLAG);
        // register as producer
        control->dataBroker->registerTimedProducer(this, groupName, dataName,
                                                   "mars_sim/simTimer", 0);
      }
    }

    void SimNode::removeFromDataBroker() {
      std::string groupName, dataName;
      getDataBrokerNames(&groupName, &dataName);
      if(control->dataBroker) {
        control->dataBroker->unregisterTimedProducer(this, groupName, dataName,
                                                     "mars_sim/simTimer");
      }
    }

    void SimNode::receiveData(const data_broker::DataInfo& info,
                              const data_broker::DataPackage& package,
                              int id) {
      sReal value;
      package.get(4, &value);
      fRotation.x() = value;
      package.get(5, &value);
      fRotation.y() = value;
      package.get(6, &value);
      fRotation.z() = value;
      package.get(7, &value);
      fRotation.w() = value;
    }

  } // end of namespace sim
} // end of namespace mars
