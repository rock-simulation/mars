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

#include "ODEPlane.h"

namespace mars {
namespace sim {

  using namespace utils;
  using namespace interfaces;

  ODEPlane::ODEPlane(std::shared_ptr<PhysicsInterface> world, NodeData * nodeData) : ODEObject(world, nodeData) {
    // At this moment we have not much things to do here. ^_^
    std::cout << "DEBUGGG: in ODEPlane Constructor " << __FILE__ << ":" << __LINE__ << std::endl;
    createNode(nodeData); // pass a function pointer?
  }

  ODEPlane::~ODEPlane(void) {
    std::cout << "DEBUGGG: in ODEPlane Destructor " << __FILE__ << ":" << __LINE__ << std::endl;
  }

  bool ODEPlane::createODEGeometry(interfaces::NodeData *node){
    // build the ode representation
    nGeom = dCreatePlane(theWorld->getSpace(), 0, 0, 1, (dReal)node->pos.z());
    std::cout << "Created Plane!" << std::endl;
    return true;
  }
} // end of namespace sim
} // end of namespace mars
