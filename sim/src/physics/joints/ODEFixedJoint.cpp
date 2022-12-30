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

#include "ODEFixedJoint.h"

namespace mars {
namespace sim {

  using namespace utils;
  using namespace interfaces;

  ODEFixedJoint::ODEFixedJoint(std::shared_ptr<interfaces::PhysicsInterface> world,
                               interfaces::JointData *joint,
                               const std::shared_ptr<interfaces::NodeInterface> node1,
                               const std::shared_ptr<interfaces::NodeInterface> node2)
  : ODEJoint(world) {
    createJoint(joint, node1, node2 );
  }

  ODEFixedJoint::~ODEFixedJoint(void) {
  }

  bool ODEFixedJoint::createODEJoint(interfaces::JointData *jointS, dBodyID body1, dBodyID body2){
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
