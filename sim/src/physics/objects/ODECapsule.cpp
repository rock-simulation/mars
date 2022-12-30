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

#include "ODECapsule.h"
#include <mars/interfaces/terrainStruct.h>

namespace mars {
namespace sim {

  using namespace utils;
  using namespace interfaces;

  ODECapsule::ODECapsule(std::shared_ptr<PhysicsInterface> world, NodeData * nodeData) : ODEObject(world, nodeData) {
    createNode(nodeData);
  }

  ODECapsule::~ODECapsule(void) {
  }

  bool ODECapsule::createODEGeometry(interfaces::NodeData *node){
    if (!node->inertia_set && (node->ext.x() <= 0 || node->ext.y() <= 0)) {
      LOG_ERROR("Cannot create Node \"%s\" (id=%lu):\n"
                "  Capsule Nodes must have ext.x() and ext.y() > 0.\n"
                "  Current values are: x=%g; y=%g",
                node->name.c_str(), node->index,
                node->ext.x(), node->ext.y());
      return false;
    }

    // build the ode representation
    nGeom = dCreateCapsule(theWorld->getSpace(), (dReal)node->ext.x(),
                            (dReal)node->ext.y());

    // create the mass object for the capsule
    if(node->inertia_set) {
      setInertiaMass(node);
    }
    else if(node->density > 0) {
      dMassSetCapsule(&nMass, (dReal)node->density, 3, (dReal)node->ext.x(),
                      (dReal)node->ext.y());
    }
    else if(node->mass > 0) {
      dMassSetCapsuleTotal(&nMass, (dReal)node->mass, 3, (dReal)node->ext.x(),
                            (dReal)node->ext.y());
    }
    return true;
  }

} // end of namespace sim
} // end of namespace mars
