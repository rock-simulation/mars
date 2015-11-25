/*
 *  Copyright 2011, 2012, 2014, DFKI GmbH Robotics Innovation Center
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

#include "JointData.h"
#include "utils.h"
#define FORWARD_DECL_ONLY
#include "Logging.hpp"
#include "sim/ControlCenter.h"

#include "sim/LoadCenter.h"
#include <mars/utils/mathUtils.h>

// should add some error handling for ConfigItem::get...()

#define GET_VALUE(str, val, type)                    \
  if((it = config->find(str)) != config->end())      \
    val = it->second

#define GET_OBJECT(str, val, type)              \
  if((it = config->find(str)) != config->end()) \
    type##FromConfigItem(it->second, &val);

#define SET_VALUE(str, val)                              \
  if(val != defaultJoint.val)                             \
    (*config)[str] = val

#define SET_OBJECT(str, val, type)                                      \
  if(1 || val.squaredNorm() - defaultJoint.val.squaredNorm() < 0.0000001) {               \
    type##ToConfigItem((*config)[str], &val);                       \
  }

namespace mars {
  namespace interfaces {

    using namespace mars::utils;

    JointData::JointData( const std::string& name, JointType type,
                          unsigned long node_id1,
                          unsigned long node_id2) {
      init( name, type, node_id1, node_id2 );
    }

    void JointData::init( const std::string& name, JointType type,
                          unsigned long node_id1,
                          unsigned long node_id2) {
      this->name = name;
      index = 0;
      this->type = type;
      nodeIndex1 = node_id1;
      nodeIndex2 = node_id2;
      anchorPos = 0;
      spring_constant = 0;
      damping_constant = 0;
      lowStopAxis1 = 0;
      highStopAxis1 = 0;
      damping_const_constraint_axis1 = 0;
      spring_const_constraint_axis1 = 0;
      lowStopAxis2 = 0;
      highStopAxis2 = 0;
      damping_const_constraint_axis2 = 0;
      spring_const_constraint_axis2 = 0;
      angle1_offset = 0;
      angle2_offset = 0;
      anchor.setZero();
      axis1.setZero();
      axis1.setZero();
      axis2.setZero();
      invertAxis = false;
    }

    bool JointData::fromConfigMap(ConfigMap *config,
                                  std::string filenamePrefix,
                                  LoadCenter *loadCenter) {
      CPP_UNUSED(filenamePrefix);
      CPP_UNUSED(loadCenter);
      ConfigMap::iterator it;
      unsigned int mapIndex;
      GET_VALUE("mapIndex", mapIndex, UInt);
      
      name = config->get("name", name);
      if((it = config->find("type")) != config->end()) {
        type = getJointType((std::string)it->second);
        if(type == JOINT_TYPE_UNDEFINED) {
          LOG_ERROR("JointData: type given is undefined");          
        }
      }
      else {
        LOG_ERROR("JointData: no type given for joint");
      }
      GET_VALUE("index", index, ULong);
      GET_VALUE("nodeindex1", nodeIndex1, ULong);
      if(!nodeIndex1) {
        LOG_ERROR("JointData: no first node attached to joint");
      }
      else if(mapIndex && loadCenter) {
          nodeIndex1 = loadCenter->getMappedID(nodeIndex1, MAP_TYPE_NODE,
                                               mapIndex);
      }

      GET_VALUE("nodeindex2", nodeIndex2, ULong);
      if(mapIndex && loadCenter) {
        nodeIndex2 = loadCenter->getMappedID(nodeIndex2, MAP_TYPE_NODE,
                                             mapIndex);
      }
      
      { // handle axis 1
        GET_OBJECT("axis1", axis1, vector);
        GET_VALUE("lowStopAxis1", lowStopAxis1, Double);
        GET_VALUE("highStopAxis1", highStopAxis1, Double);
        GET_VALUE("damping_const_constraint_axis1",
                  damping_const_constraint_axis1, Double);
        GET_VALUE("spring_const_constraint_axis1",
                  spring_const_constraint_axis1, Double);
        GET_VALUE("angle1_offset", angle1_offset, Double);
      }

      { // handle axis 2
        GET_OBJECT("axis2", axis2, vector);
        GET_VALUE("lowStopAxis2", lowStopAxis2, Double);
        GET_VALUE("highStopAxis2", highStopAxis2, Double);
        GET_VALUE("damping_const_constraint_axis2",
                  damping_const_constraint_axis2, Double);
        GET_VALUE("spring_const_constraint_axis2",
                  spring_const_constraint_axis2, Double);
        GET_VALUE("angle2_offset", angle2_offset, Double);
      }

      GET_VALUE("anchorpos", anchorPos, Int);
      GET_OBJECT("anchor", anchor, vector);        
      GET_VALUE("invertAxis", invertAxis, Bool);

      return true;
    }

    void JointData::toConfigMap(ConfigMap *config, bool skipFilenamePrefix) {
      CPP_UNUSED(skipFilenamePrefix);
      JointData defaultJoint;

      SET_VALUE("name", name);

      {
        std::string tmp = getJointTypeString(type);
        (*config)["type"] = tmp;
      }

      SET_VALUE("index", index);
      SET_VALUE("nodeindex1", nodeIndex1);
      SET_VALUE("nodeindex2", nodeIndex2);

      { // handle axis 1
        SET_OBJECT("axis1", axis1, vector);
        SET_VALUE("lowStopAxis1", lowStopAxis1);
        SET_VALUE("highStopAxis1", highStopAxis1);
        SET_VALUE("damping_const_constraint_axis1",
                  damping_const_constraint_axis1);
        SET_VALUE("spring_const_constraint_axis1",
                  spring_const_constraint_axis1);
        SET_VALUE("angle1_offset", angle1_offset);
      }

      { // handle axis 2
        SET_OBJECT("axis2", axis2, vector);
        SET_VALUE("lowStopAxis2", lowStopAxis2);
        SET_VALUE("highStopAxis2", highStopAxis2);
        SET_VALUE("damping_const_constraint_axis2",
                  damping_const_constraint_axis2);
        SET_VALUE("spring_const_constraint_axis2",
                  spring_const_constraint_axis2);
        SET_VALUE("angle2_offset", angle2_offset);
      }

      SET_VALUE("anchorpos", anchorPos);
      SET_OBJECT("anchor", anchor, vector);        
      SET_VALUE("invertAxis", invertAxis);
    }

    void JointData::getFilesToSave(std::vector<std::string> *fileList) {
      CPP_UNUSED(fileList);
    }

  } // end of namespace interfaces
} // end of namespace mars
