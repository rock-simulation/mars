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

#include "ODESphere.h"

namespace mars {
  namespace sim {

    using namespace utils;
    using namespace interfaces;

    ODESphere::ODESphere(std::shared_ptr<PhysicsInterface> world, NodeData * nodeData) : ODEObject(world, nodeData) {
      // At this moment we have not much things to do here. ^_^
      std::cout << "DEBUGGG: in ODESphere Constructor " << __FILE__ << ":" << __LINE__ << std::endl;
      createNode(nodeData);
    }

    ODESphere::~ODESphere(void) {
      std::cout << "DEBUGGG: in ODESphere Destructor " << __FILE__ << ":" << __LINE__ << std::endl;
    }

    /**
     * The method creates an ode shpere representation of the given node.
     *
     */
    bool ODESphere::createODEGeometry(NodeData* node) {
      if (!node->inertia_set && node->ext.x() <= 0) {
        LOG_ERROR("Cannot create Node \"%s\" (id=%lu):\n"
                  "  Sphere Nodes must have ext.x() > 0.\n"
                  "  Current value is: x=%g",
                  node->name.c_str(), node->index, node->ext.x());
        return false;
      }

      // build the ode representation
      nGeom = dCreateSphere(theWorld->getSpace(), (dReal)node->ext.x());

      // create the mass object for the sphere
      if(node->inertia_set) {
        setInertiaMass(node);
      }
      else if(node->density > 0) {
        dMassSetSphere(&nMass, (dReal)node->density, (dReal)node->ext.x());
      }
      else if(node->mass > 0) {
        dMassSetSphereTotal(&nMass, (dReal)node->mass, (dReal)node->ext.x());
      }
      std::cout <<"Created ODESphere!" << std::endl;
      return true;
    }

  } // end of namespace sim
} // end of namespace mars
