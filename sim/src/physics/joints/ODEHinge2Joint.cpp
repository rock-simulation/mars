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

#include "ODEHinge2Joint.h"

namespace mars {
namespace sim {

  using namespace utils;
  using namespace interfaces;

  ODEHinge2Joint::ODEHinge2Joint(std::shared_ptr<interfaces::PhysicsInterface> world,
                               interfaces::JointData *joint,
                               const std::shared_ptr<interfaces::NodeInterface> node1,
                               const std::shared_ptr<interfaces::NodeInterface> node2)
  : ODEJoint(world) {
    createJoint(joint, node1, node2 );
  }

  ODEHinge2Joint::~ODEHinge2Joint(void) {
  }

  bool ODEHinge2Joint::createODEJoint(interfaces::JointData *jointS, dBodyID body1, dBodyID body2){

    if (body1 || body2){

      jointId= dJointCreateHinge2(theWorld->getWorld(),0);
      dJointAttach(jointId, body1, body2);
      dJointSetHinge2Anchor(jointId, jointS->anchor.x(), jointS->anchor.y(),
                            jointS->anchor.z());
      dJointSetHinge2Axis1(jointId, jointS->axis1.x(), jointS->axis1.y(),
                            jointS->axis1.z());
      dJointSetHinge2Axis2(jointId, jointS->axis2.x(), jointS->axis2.y(),
                            jointS->axis2.z());

      if(damping>0.00000001) {
        dJointSetHinge2Param(jointId, dParamFMax, damping);
        dJointSetHinge2Param(jointId, dParamVel, 0);
        dJointSetHinge2Param(jointId, dParamFMax2, damping);
        dJointSetHinge2Param(jointId, dParamVel2, 0);
      }
      if(spring > 0.00000001) {
        dJointSetHinge2Param(jointId, dParamLoStop, lo1);
        dJointSetHinge2Param(jointId, dParamHiStop, hi1);
        dJointSetHinge2Param(jointId, dParamLoStop2, lo2);
        dJointSetHinge2Param(jointId, dParamHiStop2, hi2);
        dJointSetHinge2Param(jointId, dParamStopCFM, cfm1);
        dJointSetHinge2Param(jointId, dParamStopERP, erp1);
        dJointSetHinge2Param(jointId, dParamStopCFM2, cfm2);
        dJointSetHinge2Param(jointId, dParamStopERP2, erp2);
        dJointSetHinge2Param(jointId, dParamCFM, cfm);
      }
      if(jointCFM > 0) {
        dJointSetHinge2Param(jointId, dParamCFM, jointCFM);
      }
    }
    else {
      return 0;
    }
    return 1;
  }

} // end of namespace sim
} // end of namespace mars
