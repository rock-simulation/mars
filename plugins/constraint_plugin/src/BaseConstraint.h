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
 * \file BaseConstraint.h
 * \author Lorenz Quack (lorenz.quack@dfki.de)
 * \brief Base class to all constraints.
 *
 * Version 0.1
 */

#ifndef BASE_CONSTRAINT_H
#define BASE_CONSTRAINT_H

#ifdef _PRINT_HEADER_
  #warning "ConstraintsPlugin.h"
#endif

#include <mars/cfg_manager/CFGClient.h>

namespace mars {

  namespace interfaces {
    class ControlCenter;
  }

  namespace plugins {

    namespace constraints_plugin {

      enum ConstraintType {
        CONSTRAINT_UNDEFINED,
        CONSTRAINT_NODE,
      };
  
      enum AttributeType {
        ATTRIBUTE_UNDEFINED,
        ATTRIBUTE_POSITION_X,
        ATTRIBUTE_POSITION_Y,
        ATTRIBUTE_POSITION_Z,
        ATTRIBUTE_SIZE_X,
        ATTRIBUTE_SIZE_Y,
        ATTRIBUTE_SIZE_Z,
      };

      class BaseConstraint : public cfg_manager::CFGClient {
      public:
        BaseConstraint(interfaces::ControlCenter *controlCenter) 
          : control(controlCenter),
            type(CONSTRAINT_UNDEFINED), 
            parameterName("<unnamed>")
        {}
        virtual ~BaseConstraint() {}
        virtual void cfgUpdateProperty(cfg_manager::cfgPropertyStruct propertyStruct)=0;
        virtual void reset() = 0;

      protected:
        interfaces::ControlCenter *control;
        ConstraintType type;
        std::string parameterName;
      }; // end of class Constraint

    } // end of namespace constraints_plugin
  } // end of namespace plugins
} // end of namespace mars

#endif // BASE_CONSTRAINT_H
