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
     *  - "axis1/speed" (double)       // speed is deprecated
     *  - "axis1/velocity" (double)
     *  - "axis1/torque/x" (double)
     *  - "axis1/torque/y" (double)
     *  - "axis1/torque/z" (double)
     *  - "axis2/x" (double)
     *  - "axis2/y" (double)
     *  - "axis2/z" (double)
     *  - "axis2/angle" (double)
     *  - "axis2/speed" (double)       // speed is deprecated
     *  - "axis2/velocity" (double)
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

      // function members
      void rotateAxis(const utils::Quaternion &rotatem, unsigned char axis_index=1);
      void update(interfaces::sReal calc_ms);
      void reattachJoint(void);
      void attachMotor(unsigned char axis_index);
      void detachMotor(unsigned char axis_index);
      void updateStepSize(void);

      // getters
      const utils::Vector getAnchor(void) const;
      SimNode* getAttachedNode(unsigned char axis_index=1) const;
      const utils::Vector getAxis(unsigned char axis_index=1) const;
      void getCoreExchange(interfaces::core_objects_exchange *obj) const;
      interfaces::sReal getPosition(unsigned char axis_index=1) const;
      const utils::Vector getForceVector(unsigned char axis_index=1) const;
      unsigned long getIndex(void) const;
      interfaces::JointType getJointType(void) const;
      const utils::Vector getJointLoad(void) const;
      interfaces::sReal getLowerLimit(unsigned char axis_index=1) const;
      interfaces::sReal getUpperLimit(unsigned char axis_index=1) const;
      interfaces::sReal getMotorTorque(void) const;  // FIXME: this should not be in the joint
      interfaces::NodeId getNodeId(unsigned char node_index=1) const;
      const interfaces::JointData getSJoint(void) const;
      interfaces::sReal getVelocity(unsigned char axis_index=1) const;
      interfaces::sReal getTorque(interfaces::sReal torque, unsigned char axis_index=1) const;
      const utils::Vector getTorqueVector(unsigned char axis_index=1) const;
      const utils::Vector getTorqueVectorAroundAxis(unsigned char axis_index=1) const;

      // setters
      void setAnchor(const utils::Vector &pos);
      void setAttachedNodes(SimNode *node, SimNode *node2);
      void setAxis(const utils::Vector &axis, unsigned char axis_index=1);
      void setEffortLimit(interfaces::sReal force, unsigned char axis_index=1);
      void setId(unsigned long i);
      void setJointType(interfaces::JointType type);
      void setOfflinePosition(interfaces::sReal value);
      void setPhysicalJoint(interfaces::JointInterface *physical_joint);
      void setSDParams(interfaces::JointData *sJoint);
      void setSJoint(const interfaces::JointData &sJoint);
      void setVelocity(interfaces::sReal velocity, unsigned char axis_index=1);
      void setEffort(interfaces::sReal torque, unsigned char axis_index=1);
      void setLowerLimit(interfaces::sReal limit, unsigned char axis_index=1);
      void setUpperLimit(interfaces::sReal limit, unsigned char axis_index=1);

      // inherited from DataBroker ProducerInterface
      void getDataBrokerNames(std::string *groupName, std::string *dataName) const;
      virtual void produceData(const data_broker::DataInfo &info,
                               data_broker::DataPackage *package,
                               int callbackParam);

      // the following functions are going to be deprecated in the coming releases of MARS
      void changeStepSize(void) __attribute__ ((deprecated("use updateStepSize")));
      void setJointAsMotor(int axis) __attribute__ ((deprecated("use attachMotor")));
      void unsetJointAsMotor(int axis) __attribute__ ((deprecated("use detachMotor")));
      unsigned long getNodeIndex1(void) const __attribute__ ((deprecated("use getNodeId")));
      unsigned long getNodeIndex2(void) const __attribute__ ((deprecated("use getNodeId")));
      void setInterface(interfaces::JointInterface *physical_joint) __attribute__ ((deprecated("use setPhysicalJoint")));
      void setForceLimit(interfaces::sReal force) __attribute__ ((deprecated("use setEffortLimit")));
      void setForceLimit2(interfaces::sReal force) __attribute__ ((deprecated("use setEffortLimit")));
      void setVelocity2(interfaces::sReal velocity) __attribute__ ((deprecated("use setVelocity(v, 1)")));
      interfaces::sReal getVelocity2(void) const __attribute__ ((deprecated("use getVelocity(1)")));
      void setTorque(interfaces::sReal torque) __attribute__ ((deprecated("use setEffort")));
      void setTorque2(interfaces::sReal torque) __attribute__ ((deprecated("use setEffort")));
      interfaces::sReal getLowStop() const __attribute__ ((deprecated("use getLowerLimit")));
      interfaces::sReal getLowStop2() const __attribute__ ((deprecated("use getLowerLimit")));
      interfaces::sReal getHighStop() const __attribute__ ((deprecated("use getUpperLimit")));
      interfaces::sReal getHighStop2() const __attribute__ ((deprecated("use getUpperLimit")));
      void setLowStop(interfaces::sReal lowStop) __attribute__ ((deprecated("use setLowerLimit")));
      void setHighStop(interfaces::sReal highStop) __attribute__ ((deprecated("use setUpperLimit")));
      void setLowStop2(interfaces::sReal lowStop2) __attribute__ ((deprecated("use setLowerLimit")));
      void setHighStop2(interfaces::sReal highStop2) __attribute__ ((deprecated("use setUpperLimit")));
      const utils::Vector getForce1(void) const __attribute__ ((deprecated("use getForceVector")));
      const utils::Vector getForce2(void) const __attribute__ ((deprecated("use getForceVector")));
      const utils::Vector getTorque1(void) const __attribute__ ((deprecated("use getTorqueVector")));
      const utils::Vector getTorque2(void) const __attribute__ ((deprecated("use getTorqueVector")));
      interfaces::sReal getActualAngle1() const __attribute__ ((deprecated("use getPosition")));
      interfaces::sReal getActualAngle2() const __attribute__ ((deprecated("use getPosition")));
      SimNode* getAttachedNode1(void) const __attribute__ ((deprecated("use getAttachedNode")));
      SimNode* getAttachedNode2(void) const __attribute__ ((deprecated("use getgetAttachedNode")));
      void setAxis1(const utils::Vector &axis) __attribute__ ((deprecated("use setAxis")));
      void setAxis2(const utils::Vector &axis) __attribute__ ((deprecated("use setAxis")));
      void rotateAxis1(const utils::Quaternion &rotatem) __attribute__ ((deprecated("use rotateAxis")));
      const utils::Vector getAxis1(void) const __attribute__ ((deprecated("use getAxis")));
      const utils::Vector getAxis2(void) const __attribute__ ((deprecated("use getAxis")));
      void reattacheJoint(void) __attribute__ ((deprecated("use reattachJoint")));
      const utils::Vector getAxis1Torque(void) const __attribute__ ((deprecated("use getTorqueAroundAxis")));
      const utils::Vector getAxis2Torque(void) const __attribute__ ((deprecated("use getTorqueAroundAxis")));
      void setOfflineValue(interfaces::sReal value) __attribute__ ((deprecated("use setOfflinePosition")));

    private:
      interfaces::ControlCenter *control;
      interfaces::JointData sJoint;
      interfaces::JointInterface *physical_joint;
      SimNode *snode1, *snode2;
      interfaces::JointId id;
      interfaces::sReal position1, position2;
      interfaces::sReal velocity1, velocity2;
      interfaces::sReal lowerLimit1, lowerLimit2, upperLimit1, upperLimit2;
      utils::Vector anchor;
      utils::Vector axis1, axis2; // axes
      utils::Vector f1, f2; // forces
      utils::Vector t1, t2; // torques
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
