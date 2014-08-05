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

#ifndef SIMJOINT_H
#define SIMJOINT_H

#ifdef _PRINT_HEADER_
#warning "SimJoint.h"
#endif

#include <mars/interfaces/sim/JointInterface.h>

#include <mars/data_broker/ProducerInterface.h>
#include <mars/data_broker/DataPackageMapping.h>

namespace mars {
  
  namespace interfaces {
    class ControlCenter;
  }

  namespace sim {

    class SimNode;

    /**
     * exchange structure for joint angles
     */
    struct jointInfo {
      interfaces::sReal actualAngle1;
      interfaces::sReal actualAngle2;
      interfaces::sReal sliderPosition;
      utils::Vector anchor;
    };

    /**
     * SimJoint represents the simulated joints
     *
     * Each SimJoint object publishes its state on the dataBroker.
     * The name under which the data is published can be obtained from the 
     * jointId via JointManager::getDataBrokerNames.
     * The data_broker::DataPackage will contain the following items:
     *  - "id" (int)
     *  - "axis1/x" (double)
     *  - "axis1/y" (double)
     *  - "axis1/z" (double)
     *  - "axis1/angle" (double)
     *  - "axis1/speed" (double)
     *  - "axis1/torque/x" (double)
     *  - "axis1/torque/y" (double)
     *  - "axis1/torque/z" (double)
     *  - "axis2/x" (double)
     *  - "axis2/y" (double)
     *  - "axis2/z" (double)
     *  - "axis2/angle" (double)
     *  - "axis2/speed" (double)
     *  - "axis2/torque/x" (double)
     *  - "axis2/torque/y" (double)
     *  - "axis2/torque/z" (double)
     *  - "force1/x" (double)
     *  - "force1/y" (double)
     *  - "force1/z" (double)
     *  - "torque1/x" (double)
     *  - "torque1/y" (double)
     *  - "torque1/z" (double)
     *  - "force2/x" (double)
     *  - "force2/y" (double)
     *  - "force2/z" (double)
     *  - "torque2/x" (double)
     *  - "torque2/y" (double)
     *  - "torque2/z" (double)
     *  - "anchor/x" (double)
     *  - "anchor/y" (double)
     *  - "anchor/z" (double)
     *  - "jointLoad/x" (double)
     *  - "jointLoad/y" (double)
     *  - "jointLoad/z" (double)
     *  - "motorTorque" (double)
     */
    class SimJoint : public data_broker::ProducerInterface {
    public:

      explicit SimJoint(interfaces::ControlCenter *control,
                        const interfaces::JointData &sJoint);
      ~SimJoint();

      /**
       * returns the anchor of the joint
       *
       * @return position of the anchor
       */
      const utils::Vector getAnchor(void) const;

      /**
       * get the node to which node 2 is attached
       *
       * @return pointer to attached node
       */
      SimNode* getAttachedNode1(void) const;

      /**
       * get the node that is attached to node 1
       *
       * @return pointer to attached node
       */
      SimNode* getAttachedNode2(void) const;

      /**
       * returns orientation of the first axis of the joint
       *
       * @return orientation of the axis
       */
      const utils::Vector getAxis1(void) const;

      /**
       * returns orientation of the second axis of the joint
       *
       * @return orientation of the axis
       */
      const utils::Vector getAxis2(void) const;

      /**
       * returns joint type
       *
       * @return joint type
       */
      interfaces::JointType getJointType(void) const;

      /**
       * set the anchor of the joint
       *
       * @param pos position of the anchor
       */
      void setAnchor(const utils::Vector &pos);

      /**
       * set the nodes that this joint combines
       *
       * @param node SimNode to that node2 is attached
       * @param node2 SimNode that is attached to node
       */
      void setAttachedNodes(SimNode *node, SimNode *node2);

      /**
       * set orientation of the first axis of the joint
       *
       * @param axis orientation of the axis
       */
      void setAxis1(const utils::Vector &axis);

      /**
       * rotates the first axis of the joint
       *
       * @param rotate rotation Quaternion for the first axis
       */
      void rotateAxis1(const utils::Quaternion &rotate);

      /**
       * set orientation of the second axis of the joint
       *
       * @param axis orientation of the axis
       */
      void setAxis2(const utils::Vector &axis);

      /**
       * set the joint type
       *
       * @param type joint type
       */
      void setJointType(interfaces::JointType type);

      /**
       * update joint angles
       */
      void update(interfaces::sReal calc_ms);

      /**
       * returns the actual angle of axis 1
       *
       * @return angle as double value
       */
      interfaces::sReal getActualAngle1() const;

      /**
       * returns the actual angle of axis 2
       *
       * @return angle as double value
       */
      interfaces::sReal getActualAngle2() const;

      const utils::Vector getForce1(void) const;
      const utils::Vector getForce2(void) const;
      const utils::Vector getTorque1(void) const;
      const utils::Vector getTorque2(void) const;
      const utils::Vector getAxis1Torque(void) const;
      const utils::Vector getAxis2Torque(void) const;
      const utils::Vector getJointLoad(void) const;
      interfaces::sReal getMotorTorque(void) const;
      void reattacheJoint(void);

      //-----------------------------------------------------------------------------
      //Further part is still under development and shall not be shown in the doc
      //until finished


      void setIndex(unsigned long i);

      unsigned long getIndex(void) const;
      unsigned long getNodeIndex1(void) const;
      unsigned long getNodeIndex2(void) const;
      const interfaces::JointData getSJoint(void) const;
      void setInterface(interfaces::JointInterface *my_interface);
      void setForceLimit(interfaces::sReal force);
      void setVelocity(interfaces::sReal velocity);
      void setForceLimit2(interfaces::sReal force);
      void setVelocity2(interfaces::sReal velocity);
      interfaces::sReal getVelocity(void) const;
      interfaces::sReal getVelocity2(void) const;
      void setTorque(interfaces::sReal torque);
      void setTorque2(interfaces::sReal torque);
      void setJointAsMotor(int axis);
      void getCoreExchange(interfaces::core_objects_exchange *obj) const;
      void unsetJointAsMotor(int axis);
      void changeStepSize(void);
      void setSDParams(interfaces::JointData *sJoint);
      void getDataBrokerNames(std::string *groupName, std::string *dataName) const;

      virtual void produceData(const data_broker::DataInfo &info,
                               data_broker::DataPackage *package,
                               int callbackParam);


      void setSJoint(const interfaces::JointData &sJoint);
      void setOfflineValue(interfaces::sReal value);
      interfaces::sReal getLowStop() const;
      interfaces::sReal getHighStop() const;
      interfaces::sReal getLowStop2() const;
      interfaces::sReal getHighStop2() const;
      void setLowStop(interfaces::sReal lowStop);
      void setHighStop(interfaces::sReal highStop);
      void setLowStop2(interfaces::sReal lowStop2);
      void setHighStop2(interfaces::sReal highStop2);

    private:

      interfaces::ControlCenter *control;
      interfaces::JointData sJoint;
      interfaces::JointInterface *my_interface;
      SimNode *snode1, *snode2;
      long id;
      interfaces::sReal actualAngle1, actualAngle2;
      interfaces::sReal speed1, speed2;
      interfaces::sReal lowStop1, lowStop2, highStop1, highStop2;
      utils::Vector anchor;
      utils::Vector axis1;
      utils::Vector axis2;
      utils::Vector f1, f2;
      utils::Vector t1, t2;
      utils::Vector axis1_torque, axis2_torque, joint_load;
      interfaces::sReal motor_torque, invert;
      utils::Vector axis1InNode1;
      utils::Vector node1ToAnchor;

      // for dataBroker communication
      void setupDataPackageMapping();
      data_broker::DataPackageMapping dbPackageMapping;
    };

  } // end of namespace sim
} // end of namespace mars

#endif  // SIMJOINT_H
