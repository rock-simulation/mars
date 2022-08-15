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

#include "CoreODEJoints.h"

namespace mars {
namespace sim {

  using namespace utils;
  using namespace interfaces;

  ODEHingeJoint::ODEHingeJoint(std::shared_ptr<interfaces::PhysicsInterface> world, 
                               interfaces::JointData *joint,
                               const std::shared_ptr<interfaces::NodeInterface> node1, 
                               const std::shared_ptr<interfaces::NodeInterface> node2)
  : ODEJoint(world) {
    createJoint(joint, node1, node2 );    
  }

  ODEHingeJoint::~ODEHingeJoint(void) {
  }

  bool ODEHingeJoint::createODEJoint(JointData *jointS, dBodyID body1, dBodyID body2){

    jointId= dJointCreateHinge(theWorld->getWorld(),0);
    dJointAttach(jointId, body1, body2);
    dJointSetHingeAnchor(jointId, jointS->anchor.x(), jointS->anchor.y(),
                          jointS->anchor.z()); 
    dJointSetHingeAxis(jointId, jointS->axis1.x(), jointS->axis1.y(),
                        jointS->axis1.z());
  
    if(damping>0.00000001) {
      dJointSetHingeParam(jointId, dParamFMax, damping);
      dJointSetHingeParam(jointId, dParamVel, 0);
    }
    if(spring > 0.00000001) {
      dJointSetHingeParam(jointId, dParamLoStop, lo1);
      dJointSetHingeParam(jointId, dParamHiStop, hi1);
      dJointSetHingeParam(jointId, dParamStopCFM, cfm1);
      dJointSetHingeParam(jointId, dParamStopERP, erp1);
      //dJointSetHingeParam(jointId, dParamCFM, cfm);
      //dJointSetHingeParam(jointId, dParamFudgeFactor, 1);
    }
    else if(lo1 != 0) {
      dJointSetHingeParam(jointId, dParamLoStop, lo1);
      dJointSetHingeParam(jointId, dParamHiStop, hi1);    
    }
    if(jointCFM > 0) {
      dJointSetHingeParam(jointId, dParamCFM, jointCFM);
    }
    // good value for the SpaceClimber robot
    //dJointSetHingeParam(jointId, dParamCFM, 0.03);
    //dJointSetHingeParam(jointId, dParamCFM, 0.018);
    //dJointSetHingeParam(jointId, dParamCFM, 0.09);

    return 1;   
  }

  ODEHinge2Joint::ODEHinge2Joint(std::shared_ptr<interfaces::PhysicsInterface> world, 
                               interfaces::JointData *joint,
                               const std::shared_ptr<interfaces::NodeInterface> node1, 
                               const std::shared_ptr<interfaces::NodeInterface> node2)
  : ODEJoint(world) {
    createJoint(joint, node1, node2 ); 
  }

  ODEHinge2Joint::~ODEHinge2Joint(void) {
  }
  
  bool ODEHinge2Joint::createODEJoint(JointData *jointS, dBodyID body1, dBodyID body2){

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

  ODESliderJoint::ODESliderJoint(std::shared_ptr<interfaces::PhysicsInterface> world, 
                               interfaces::JointData *joint,
                               const std::shared_ptr<interfaces::NodeInterface> node1, 
                               const std::shared_ptr<interfaces::NodeInterface> node2)
  : ODEJoint(world) {
    createJoint(joint, node1, node2 ); 
  }

  ODESliderJoint::~ODESliderJoint(void) {
  }

  bool ODESliderJoint::createODEJoint(JointData *jointS, dBodyID body1, dBodyID body2){

    jointId= dJointCreateSlider(theWorld->getWorld(),0);
    dJointAttach(jointId, body1, body2);
    dJointSetSliderAxis(jointId, jointS->axis1.x(), jointS->axis1.y(),
                        jointS->axis1.z());

    if(spring > 0.00000001) {
      dJointSetSliderParam(jointId, dParamLoStop, lo1);
      dJointSetSliderParam(jointId, dParamHiStop, hi1);
      dJointSetSliderParam(jointId, dParamStopCFM, cfm1);
      dJointSetSliderParam(jointId, dParamStopERP, erp1);
    }
    else if(damping > 0.00000001) {
      dJointSetSliderParam(jointId, dParamFMax, damping);
      dJointSetSliderParam(jointId, dParamVel, 0);
    }
    else if(lo1 != 0) {
      dJointSetSliderParam(jointId, dParamLoStop, lo1);
      dJointSetSliderParam(jointId, dParamHiStop, hi1);
    }
    if(jointCFM > 0) {
      dJointSetSliderParam(jointId, dParamCFM, jointCFM);
    }
    //dJointSetSliderParam(jointId, dParamCFM, cfm);
    return 1;
  }

  ODEBallJoint::ODEBallJoint(std::shared_ptr<interfaces::PhysicsInterface> world, 
                               interfaces::JointData *joint,
                               const std::shared_ptr<interfaces::NodeInterface> node1, 
                               const std::shared_ptr<interfaces::NodeInterface> node2)
  : ODEJoint(world) {
    createJoint(joint, node1, node2 ); 
  }

  ODEBallJoint::~ODEBallJoint(void) {
  }

  bool ODEBallJoint::createODEJoint(JointData *jointS, dBodyID body1, dBodyID body2){

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

  ODEUniversalJoint::ODEUniversalJoint(std::shared_ptr<interfaces::PhysicsInterface> world, 
                               interfaces::JointData *joint,
                               const std::shared_ptr<interfaces::NodeInterface> node1, 
                               const std::shared_ptr<interfaces::NodeInterface> node2)
  : ODEJoint(world) {
    createJoint(joint, node1, node2 );
  }

  ODEUniversalJoint::~ODEUniversalJoint(void) {
  }

  bool ODEUniversalJoint::createODEJoint(JointData *jointS, dBodyID body1, dBodyID body2) {
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

  ODEFixedJoint::ODEFixedJoint(std::shared_ptr<interfaces::PhysicsInterface> world, 
                               interfaces::JointData *joint,
                               const std::shared_ptr<interfaces::NodeInterface> node1, 
                               const std::shared_ptr<interfaces::NodeInterface> node2)
  : ODEJoint(world) {
    createJoint(joint, node1, node2 ); 
  }

  ODEFixedJoint::~ODEFixedJoint(void) {
  }

  bool ODEFixedJoint::createODEJoint(JointData *jointS, dBodyID body1, dBodyID body2){
    if (body1 || body2){  
      CPP_UNUSED(jointS);
      jointId = dJointCreateFixed(theWorld->getWorld(), 0);
      dJointAttach(jointId, body1, body2);
      dJointSetFixed(jointId);
      // used for the integration study of the SpaceClimber
      //dJointSetFixedParam(jointId, dParamCFM, cfm1);//0.0002);
      //dJointSetFixedParam(jointId, dParamERP, erp1);//0.0002);
      //dJointSetFixedParam(jointId, dParamCFM, 0.001);
    }
    else {
      return 0;
    }  
    return 1;
  }
  
} // end of namespace sim
} // end of namespace mars
