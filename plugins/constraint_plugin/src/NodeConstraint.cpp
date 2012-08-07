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

#include "NodeConstraint.h"

#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/cfg_manager/CFGDefs.h>
#include <mars/interfaces/sim/NodeManagerInterface.h>

namespace mars {
  namespace plugins {

    namespace constraints_plugin {

      using cfg_manager::cfgPropertyStruct;

      NodeConstraint::NodeConstraint(interfaces::ControlCenter *controlCenter,
                                     interfaces::NodeId nodeId, AttributeType attr,
                                     double factor, double initialValue)
        : BaseConstraint(controlCenter),
          nodeId(nodeId),
          nodeAttribute(attr),
          nodeFactor(factor)
      {
        oldValue = initialValue;
      }

      NodeConstraint::~NodeConstraint()
      {}

      void NodeConstraint::cfgUpdateProperty(cfgPropertyStruct property) {
        if(property.propertyType != cfg_manager::doubleProperty) {
          LOG_ERROR("ConstraintPlugin: Currently only double properties are supported.");
          return;
        }

        double diff = (property.dValue - oldValue) * nodeFactor;
        interfaces::NodeData n;
        n = control->nodes->getFullNode(nodeId);
        switch(nodeAttribute) {
        case ATTRIBUTE_POSITION_X:
          {
            n.pos.x() += diff;
            control->nodes->editNode(&n, interfaces::EDIT_NODE_POS); 
            break;
          }
        case ATTRIBUTE_POSITION_Y:
          {
            n.pos.y() += diff;
            control->nodes->editNode(&n, interfaces::EDIT_NODE_POS); 
            break;
          }
        case ATTRIBUTE_POSITION_Z:
          {
            n.pos.z() += diff;
            control->nodes->editNode(&n, interfaces::EDIT_NODE_POS); 
            break;
          }
          /*
            case ATTRIBUTE_ROTATION_X:
            {
            n.pos = control->nodes->getPosition(nodeId);
            n.pos.x() += diff;
            control->nodes->editNode(&n, EDIT_NODE_POS); 
            break;
            }
            case ATTRIBUTE_ROTATION_Y:
            {
            n.pos = control->nodes->getPosition(nodeId);
            n.pos.y() += diff;
            control->nodes->editNode(&n, EDIT_NODE_POS); 
            break;
            }
            case ATTRIBUTE_ROTATION_Z:
            {
            n.pos = control->nodes->getPosition(nodeId);
            n.pos.z() += diff;
            control->nodes->editNode(&n, EDIT_NODE_POS); 
            break;
            }
          */
        case ATTRIBUTE_SIZE_X:
          {
            n.ext.x() += diff;
            control->nodes->editNode(&n, interfaces::EDIT_NODE_SIZE);
            break;
          }
        case ATTRIBUTE_SIZE_Y:
          {
            n.ext.y() += diff;
            control->nodes->editNode(&n, interfaces::EDIT_NODE_SIZE);
            break;
          }
        case ATTRIBUTE_SIZE_Z:
          {
            n.ext.z() += diff;
            LOG_DEBUG("NodeContraint: edit node size z");
            control->nodes->editNode(&n, interfaces::EDIT_NODE_SIZE);
            break;
          }
        default:
          LOG_ERROR("NodeConstraint: attribute %d not supported.", nodeAttribute);
        }
        oldValue = property.dValue;
      }

    } // end of namespace constraints_plugin
  } // end of namespace plugins
} // end of namespace mars
