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

 /**
 * \file ObjectFactoryInterface.h
 * \author Muhammad Haider Khan Lodhi
 * \brief "ObjectFactoryInterface" is an interface to create physical ode objects for the nodes.
 *
 */

#pragma once

#include <mars/interfaces/sim/NodeInterface.h>

namespace mars{
namespace sim{

using namespace ::mars::interfaces;

class ObjectFactoryInterface{
public:
  virtual std::shared_ptr<NodeInterface> createObject(std::shared_ptr<PhysicsInterface> worldPhysics, NodeData * nodeData) = 0;
};
}
}