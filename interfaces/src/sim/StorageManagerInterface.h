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

/**
 * \file NodeManagerInterface.h
 * \author Malte Roemmermann \n
 * \brief "NodeManagerInterface" declares the interfaces for all NodeOperations
 * that are used for the communication between the simulation modules.
 *
 * \version 1.2
 * \date 02.01.2009
 */

#ifndef STORAGE_MANAGER_INTERFACE_H
#define STORAGE_MANAGER_INTERFACE_H

#ifdef _PRINT_HEADER_
  #warning "StorageManagerInterface.h"
#endif

#include <string>
#include <memory>

namespace envire { namespace core {
  class EnvireGraph;
  class TreeView;
} }

namespace mars {
  namespace interfaces {

    class StorageManagerInterface {
      public:
        virtual ~StorageManagerInterface() {}

        virtual std::shared_ptr<envire::core::EnvireGraph> getGraph() = 0;

        virtual std::shared_ptr<envire::core::TreeView> getGraphTreeView() = 0;

        virtual std::string getRootFrame() = 0;

    };

  } // end of namespace interfaces
} // end of namespace mars

#endif  // STORAGE_MANAGER_INTERFACE_H
