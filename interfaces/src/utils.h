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

#ifndef MARS_INTERFACES_UTILS_H
#define MARS_INTERFACES_UTILS_H



#include "MARSDefs.h"
#include "sim_common.h"
#include "NodeData.h"


#include <string>

namespace mars {

  namespace interfaces {

    /**\brief get the absolute position of a new node by its relative position */
    void getAbsFromRel(const NodeData &node1, NodeData *node2);
    void getRelFromAbs(const NodeData &node1, NodeData *node2);

    /**\brief Helper function to get type-id from string in scene file.
     * \return the found id or JOINT_TYPE_UNDEFINED on error.
     */
    JointType getJointType(const std::string &text);

    /**\brief Return a string for the joint-type.
     * \return NULL on error.
     */
    const char* getJointTypeString(JointType type);

  } // end of namespace interfaces

} // namespace mars

#endif /* MARS_INTERFACES_UTILS_H */
