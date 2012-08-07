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
 * \file CFGClient.h
 * \author Michael Rohn
 * \brief 'CFGClient' is a interface to register to a value
 *
 * Version 0.1
 */

#ifndef CFG_CLIENT_H
#define CFG_CLIENT_H

#ifdef _PRINT_HEADER_
  #warning "CFGClient.h"
#endif

#include "CFGDefs.h"

namespace mars {
  namespace cfg_manager {

    class CFGClient {

    public:

      virtual void cfgUpdateProperty(cfgPropertyStruct _propertyS) {
        (void) _propertyS;
      }
      virtual void cfgParamCreated(cfgParamId _id) { (void) _id; }
      virtual void cfgParamRemoved(cfgParamId _id) { (void) _id; }

    }; // end class CFGClient

  } // end namespace cfg_manager
} // end namespace mars

#endif /* CFG_CLIENT_H */
