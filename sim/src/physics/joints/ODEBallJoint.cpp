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

#include "ODEBallJoint.h"

namespace mars {
namespace sim {

  using namespace utils;
  using namespace interfaces;

  ODEBallJoint::ODEBallJoint(std::shared_ptr<interfaces::PhysicsInterface> world,
                               interfaces::JointData *joint,
                               const std::shared_ptr<interfaces::NodeInterface> node1,
                               const std::shared_ptr<interfaces::NodeInterface> node2)
  : ODEJoint(world) {
    createJoint(joint, node1, node2 );
  }

  ODEBallJoint::~ODEBallJoint(void) {
  }

  bool ODEBallJoint::createODEJoint(interfaces::JointData *jointS, dBodyID body1, dBodyID body2){

    jointId= dJointCreateBall(theWorld->getWorld(),0);
    dJointAttach(jointId, body1, body2);
    dJointSetBallAnchor(jointId, jointS->anchor.x(), jointS->anchor.y(),
                        jointS->anchor.z());
    dJointSetBallParam(jointId, dParamCFM, cfm1);
    dJointSetBallParam(jointId, dParamERP, erp1);
    /*
      ball_motor = dJointCreateAMotor(theWorld->getWorld(), 0);
      dJointAttach(ball_motor, body1, body2);
      dJointSetAMotorNumAxes(ball_motor, 3);
      dJointSetAMotorAxis(ball_motor, 0, 1, 1,0,0);
      dJointSetAMotorAxis(ball_motor, 2, 2, 0,1,0);

      if(damping > 0.00000001) {
      dJointSetAMotorParam(ball_motor, dParamVel, 0);
      dJointSetAMotorParam(ball_motor, dParamVel2, 0);
      dJointSetAMotorParam(ball_motor, dParamVel3, 0);
      dJointSetAMotorParam(ball_motor, dParamFMax, damping);
      dJointSetAMotorParam(ball_motor, dParamFMax2, damping);
      dJointSetAMotorParam(ball_motor, dParamFMax3, damping);
      }
      else if(spring > 0.00000001) {
      dJointSetAMotorParam(ball_motor, dParamLoStop, lo2);
      dJointSetAMotorParam(ball_motor, dParamLoStop2, lo2);
      dJointSetAMotorParam(ball_motor, dParamLoStop3, lo2);
      dJointSetAMotorParam(ball_motor, dParamHiStop, hi1);
      dJointSetAMotorParam(ball_motor, dParamHiStop2, hi2);
      dJointSetAMotorParam(ball_motor, dParamHiStop3, hi2);
      dJointSetAMotorParam(ball_motor, dParamCFM, cfm1);
      dJointSetAMotorParam(ball_motor, dParamCFM2, cfm2);
      dJointSetAMotorParam(ball_motor, dParamCFM3, cfm2);
      dJointSetAMotorParam(ball_motor, dParamERP, erp1);
      dJointSetAMotorParam(ball_motor, dParamERP2, erp2);
      dJointSetAMotorParam(ball_motor, dParamERP3, erp2);
      }
      dJointSetAMotorMode (ball_motor, dAMotorEuler);
    */
   return 1;
  }

} // end of namespace sim
} // end of namespace mars
