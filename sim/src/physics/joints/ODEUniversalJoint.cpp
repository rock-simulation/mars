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

#include "ODEUniversalJoint.h"

namespace mars {
namespace sim {

  using namespace utils;
  using namespace interfaces;

  ODEUniversalJoint::ODEUniversalJoint(std::shared_ptr<interfaces::PhysicsInterface> world,
                               interfaces::JointData *joint,
                               const std::shared_ptr<interfaces::NodeInterface> node1,
                               const std::shared_ptr<interfaces::NodeInterface> node2)
  : ODEJoint(world) {
    createJoint(joint, node1, node2 );
  }

  ODEUniversalJoint::~ODEUniversalJoint(void) {
  }

  ODEJoint* ODEUniversalJoint::instanciate(std::shared_ptr<interfaces::PhysicsInterface> world,
                            interfaces::JointData *joint,
                            const std::shared_ptr<interfaces::NodeInterface> node1,
                            const std::shared_ptr<interfaces::NodeInterface> node2){
    return new ODEUniversalJoint(world, joint, node1, node2);                            
  }

  bool ODEUniversalJoint::createODEJoint(interfaces::JointData *jointS, dBodyID body1, dBodyID body2) {
    if (body1 || body2){
      jointId= dJointCreateUniversal(theWorld->getWorld(),0);
      dJointAttach(jointId, body1, body2);
      dJointSetUniversalAnchor(jointId, jointS->anchor.x(), jointS->anchor.y(),
                               jointS->anchor.z());
      dJointSetUniversalAxis1(jointId, jointS->axis1.x(), jointS->axis1.y(),
                              jointS->axis1.z());
      dJointSetUniversalAxis2(jointId, jointS->axis2.x(), jointS->axis2.y(),
                              jointS->axis2.z());

      if(damping>0.00000001) {
        dJointSetUniversalParam(jointId, dParamFMax, damping);
        dJointSetUniversalParam(jointId, dParamVel, 0);
        dJointSetUniversalParam(jointId, dParamFMax2, damping);
        dJointSetUniversalParam(jointId, dParamVel2, 0);
      }
      if(spring > 0.00000001) {
        dJointSetUniversalParam(jointId, dParamLoStop, lo1);
        dJointSetUniversalParam(jointId, dParamHiStop, hi1);
        dJointSetUniversalParam(jointId, dParamLoStop2, lo2);
        dJointSetUniversalParam(jointId, dParamHiStop2, hi2);
        dJointSetUniversalParam(jointId, dParamStopCFM, cfm1);
        dJointSetUniversalParam(jointId, dParamStopERP, erp1);
        dJointSetUniversalParam(jointId, dParamStopCFM2, cfm2);
        dJointSetUniversalParam(jointId, dParamStopERP2, erp2);
        dJointSetUniversalParam(jointId, dParamCFM, cfm);
      }
      if(lo1 > 0.0001 || lo1 < -0.0001)
        dJointSetUniversalParam(jointId, dParamLoStop, lo1);
      if(lo2 > 0.0001 || lo2 < -0.0001)
        dJointSetUniversalParam(jointId, dParamHiStop, hi1);
      if(hi1 > 0.0001 || hi1 < -0.0001)
        dJointSetUniversalParam(jointId, dParamLoStop2, lo2);
      if(hi2 > 0.0001 || hi2 < -0.0001)
        dJointSetUniversalParam(jointId, dParamHiStop2, hi2);
      if(jointCFM > 0) {
        dJointSetUniversalParam(jointId, dParamCFM, jointCFM);
      }
    }
    else {
      return 0;
    }
    return 1;
  }

} // end of namespace sim
} // end of namespace mars
