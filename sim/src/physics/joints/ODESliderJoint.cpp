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

#include "ODESliderJoint.h"

namespace mars {
namespace sim {

  using namespace utils;
  using namespace interfaces;

  ODESliderJoint::ODESliderJoint(std::shared_ptr<interfaces::PhysicsInterface> world,
                               interfaces::JointData *joint,
                               const std::shared_ptr<interfaces::NodeInterface> node1,
                               const std::shared_ptr<interfaces::NodeInterface> node2)
  : ODEJoint(world) {
    createJoint(joint, node1, node2 );
  }

  ODESliderJoint::~ODESliderJoint(void) {
  }

  ODEJoint* ODESliderJoint::instanciate(std::shared_ptr<interfaces::PhysicsInterface> world,
                            interfaces::JointData *joint,
                            const std::shared_ptr<interfaces::NodeInterface> node1,
                            const std::shared_ptr<interfaces::NodeInterface> node2){
    return new ODESliderJoint(world, joint, node1, node2);                            
  }

  bool ODESliderJoint::createODEJoint(interfaces::JointData *jointS, dBodyID body1, dBodyID body2){

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

} // end of namespace sim
} // end of namespace mars
