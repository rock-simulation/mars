/*
 *  Copyright 2012, DFKI GmbH Robotics Innovation Center
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
 * \file MarsGuiInterface.h
 */


#ifndef MARS_GUI_INTERFACE_H
#define MARS_GUI_INTERFACE_H

#include <lib_manager/LibInterface.hpp>

namespace mars {
  namespace interfaces {

    /**
     * \brief Sets up the main gui of the simulation and handles various generic gui options.
     */
    class MarsGuiInterface : public lib_manager::LibInterface {

    public:
      MarsGuiInterface(lib_manager::LibManager *theManager) : lib_manager::LibInterface(theManager) {}
      virtual ~MarsGuiInterface() {}
  
      virtual void setupGui() = 0;
    };

  } // end of namespace interfaces
} // end of namespace mars

#endif // MARS_GUI_INTERFACE_H
