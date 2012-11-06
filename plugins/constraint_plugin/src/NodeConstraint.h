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
 * \file NodeConstraint.h
 * \author Lorenz Quack (lorenz.quack@dfki.de)
 * \brief Constraints between Nodes.
 *
 * Version 0.1
 */

#ifndef NODE_CONSTRAINT_H
#define NODE_CONSTRAINT_H

#ifdef _PRINT_HEADER_
  #warning "NodeConstraint.h"
#endif

#include "BaseConstraint.h"
#include <mars/interfaces/MARSDefs.h>

namespace mars {
  namespace plugins {
    namespace constraints_plugin {

      class NodeConstraint : public BaseConstraint {
      public:
        NodeConstraint(interfaces::ControlCenter *control,
                       unsigned long paramId,
                       interfaces::NodeId nodeId, AttributeType attr,
                       double factor, double initialValue);
        ~NodeConstraint();
        virtual void cfgUpdateProperty(cfg_manager::cfgPropertyStruct propertyStruct);
        void reset();
    
      private:
        interfaces::NodeId nodeId;
        AttributeType nodeAttribute;
        double nodeFactor;
        double refValue;
        double initialValue;
        unsigned long paramId;
        double oldValue;

        double getAttribute() const;
        void setAttribute(double value);
      };

    } // end of namespace constraints_plugin
  } // end of namespace plugins
} // end of namespace mars

#endif // NODE_CONSTRAINT_H
