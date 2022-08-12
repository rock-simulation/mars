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

#include "ODECylinder.h"

namespace mars {
namespace sim {

  using namespace utils;
  using namespace interfaces;

  ODECylinder::ODECylinder(std::shared_ptr<PhysicsInterface> world, NodeData * nodeData) : ODEObject(world, nodeData) {
    // At this moment we have not much things to do here. ^_^
    std::cout << "DEBUGGG: in ODECylinder Constructor " << __FILE__ << ":" << __LINE__ << std::endl;
    createNode(nodeData); // pass a function pointer?
  }

  ODECylinder::~ODECylinder(void) {
    std::cout << "DEBUGGG: in ODECylinder Destructor " << __FILE__ << ":" << __LINE__ << std::endl;
  }

  bool ODECylinder::createODEGeometry(interfaces::NodeData *node){
    if (!node->inertia_set && (node->ext.x() <= 0 || node->ext.y() <= 0)) {
      LOG_ERROR("Cannot create Node \"%s\" (id=%lu):\n"
                "  Cylinder Nodes must have ext.x() and ext.y() > 0.\n"
                "  Current values are: x=%g; y=%g",
                node->name.c_str(), node->index,
                node->ext.x(), node->ext.y());
      return false;
    }

    // build the ode representation
    nGeom = dCreateCylinder(theWorld->getSpace(), (dReal)node->ext.x(),
                            (dReal)node->ext.y());

    // create the mass object for the cylinder
    if(node->inertia_set) {
      setInertiaMass(node);
    }
    else if(node->density > 0) {
      dMassSetCylinder(&nMass, (dReal)node->density, 3, (dReal)node->ext.x(),
                        (dReal)node->ext.y());
    }
    else if(node->mass > 0) {
      dMassSetCylinderTotal(&nMass, (dReal)node->mass, 3, (dReal)node->ext.x(),
                            (dReal)node->ext.y());
    }
    std::cout << "Created ODECylinder!" << std::endl;
    return true;
  }
} // end of namespace sim
} // end of namespace mars
