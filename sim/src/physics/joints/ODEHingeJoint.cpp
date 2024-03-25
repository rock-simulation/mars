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

#include "ODEHingeJoint.h"

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

  ODEJoint* ODEHingeJoint::instanciate(std::shared_ptr<interfaces::PhysicsInterface> world,
                            interfaces::JointData *joint,
                            const std::shared_ptr<interfaces::NodeInterface> node1,
                            const std::shared_ptr<interfaces::NodeInterface> node2){
    return new ODEHingeJoint(world, joint, node1, node2);                            
  }

  bool ODEHingeJoint::createODEJoint(interfaces::JointData *jointS, dBodyID body1, dBodyID body2){

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

} // end of namespace sim
} // end of namespace mars
