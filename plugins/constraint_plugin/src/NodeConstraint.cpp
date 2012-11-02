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
#include <mars/cfg_manager/CFGManagerInterface.h>
#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/utils.h>

namespace mars {
  namespace plugins {

    namespace constraints_plugin {

      using cfg_manager::cfgPropertyStruct;

      NodeConstraint::NodeConstraint(interfaces::ControlCenter *controlCenter,
                                     unsigned long paramId,
                                     interfaces::NodeId nodeId, AttributeType attr,
                                     double factor, double refValue)
        : BaseConstraint(controlCenter),
          nodeId(nodeId),
          nodeAttribute(attr),
          nodeFactor(factor),
          refValue(refValue),
          paramId(paramId),
          oldValue(refValue)
      {
        this->initialValue = getAttribute();
        if(control->cfg) {
          control->cfg->registerToParam(paramId, this);
        }
      }

      NodeConstraint::~NodeConstraint()
      {
        if(control->cfg) {
          control->cfg->unregisterFromParam(paramId, this);
        }
      }

      double NodeConstraint::getAttribute() const {
        double attr = 0.;
        interfaces::NodeData n = control->nodes->getFullNode(nodeId);
        if(n.relative_id) {
          interfaces::NodeData rel = control->nodes->getFullNode(n.relative_id);
          interfaces::getRelFromAbs(rel, &n);
        }
        switch(nodeAttribute) {
        case ATTRIBUTE_POSITION_X:
          attr = n.pos.x();
          break;
        case ATTRIBUTE_POSITION_Y:
          attr = n.pos.y();
          break;
        case ATTRIBUTE_POSITION_Z:
          attr = n.pos.z();
          break;
        case ATTRIBUTE_SIZE_X:
          attr = n.ext.x();
          break;
        case ATTRIBUTE_SIZE_Y:
          attr = n.ext.y();
          break;
        case ATTRIBUTE_SIZE_Z:
          attr = n.ext.z();
          break;
        default:
          LOG_ERROR("NodeConstraint: attribute %d not supported.", nodeAttribute);
        }
        return attr;
      }

      void NodeConstraint::setAttribute(double value) {
        interfaces::NodeData n = control->nodes->getFullNode(nodeId);
        if(n.relative_id) {
          interfaces::NodeData rel = control->nodes->getFullNode(n.relative_id);
          interfaces::getRelFromAbs(rel, &n);
        }
        switch(nodeAttribute) {
        case ATTRIBUTE_POSITION_X:
          {
            n.pos.x() = value;
            LOG_DEBUG("NodeContraint: edit node pos x");
            control->nodes->editNode(&n, interfaces::EDIT_NODE_POS);
            break;
          }
        case ATTRIBUTE_POSITION_Y:
          {
            n.pos.y() = value;
            LOG_DEBUG("NodeContraint: edit node pos y");
            control->nodes->editNode(&n, interfaces::EDIT_NODE_POS);
            break;
          }
        case ATTRIBUTE_POSITION_Z:
          {
            n.pos.z() = value;
            LOG_DEBUG("NodeContraint: edit node pos z");
            control->nodes->editNode(&n, interfaces::EDIT_NODE_POS);
            break;
          }
        case ATTRIBUTE_SIZE_X:
          {
            n.ext.x() = value;
            control->nodes->editNode(&n, interfaces::EDIT_NODE_SIZE);
            break;
          }
        case ATTRIBUTE_SIZE_Y:
          {
            n.ext.y() = value;
            control->nodes->editNode(&n, interfaces::EDIT_NODE_SIZE);
            break;
          }
        case ATTRIBUTE_SIZE_Z:
          {
            n.ext.z() = value;
            LOG_DEBUG("NodeContraint: edit node size z");
            control->nodes->editNode(&n, interfaces::EDIT_NODE_SIZE);
            break;
          }
        default:
          LOG_ERROR("NodeConstraint: attribute %d not supported.", nodeAttribute);
        }
      }

      void NodeConstraint::cfgUpdateProperty(cfgPropertyStruct property) {
        if(property.propertyType != cfg_manager::doubleProperty) {
          LOG_ERROR("ConstraintPlugin: Currently only double properties are supported.");
          return;
        }

        double currentValue = getAttribute();
        double diff = (property.dValue - oldValue) * nodeFactor;
        setAttribute(currentValue + diff);
        oldValue = property.dValue;
      }

      void NodeConstraint::reset() {
        double newValue = 0.;
        if(control->cfg) {
          control->cfg->getPropertyValue(paramId, "value", &newValue);
        }
        double currentValue = getAttribute();
        double diff = (newValue - refValue) * nodeFactor;
        setAttribute(currentValue + diff);
        oldValue = newValue;
      }

    } // end of namespace constraints_plugin
  } // end of namespace plugins
} // end of namespace mars
