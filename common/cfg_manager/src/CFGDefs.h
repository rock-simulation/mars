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
 * \file CFGDefs.h
 * \author Michael Rohn
 * \brief This file stores types and structs for the CFG class
 *
 * Version 0.1
 */

#ifndef CFG_DEFS_H
#define CFG_DEFS_H

#ifdef _PRINT_HEADER_
#warning "CFGDefs.h"
#endif

#include <string>


// defines for the CFGInterface-class

namespace mars {
  namespace cfg_manager {

    typedef unsigned long cfgParamId;

    enum cfgParamType {
      doubleParam = 0,
      intParam,
      boolParam,
      stringParam,
      noParam,
      dstNrOfParamTypes
    };

    static const std::string cfgParamTypeString[dstNrOfParamTypes] = {
      "double",
      "int",
      "bool",
      "string",
      "noParam"
    };

    struct cfgParamInfo {
      cfgParamId id;
      std::string group;
      std::string name;
      cfgParamType type;
    };

    enum cfgPropertyType {
      noTypeSet = 0,
      intProperty,
      doubleProperty,
      boolProperty,
      stringProperty
    };

    enum cfgParamOption {
      noParamOption = 0,
      userSave = 1,
      saveOnClose = 2
    };

    struct cfgPropertyStruct {
      cfgParamId paramId;
      unsigned int propertyIndex;
      cfgPropertyType propertyType;

      int    iValue;
      bool   bValue;
      double dValue;
      std::string sValue;

      unsigned int propertyState;
    };

  } // namespace cfg_manager
} // namespace mars

#endif /* CFG_DEFS_H */
