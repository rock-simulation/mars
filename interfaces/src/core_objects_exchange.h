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

#ifndef MARS_INTERFACES_CORE_OBJECTS_EXCHANGE_H
#define MARS_INTERFACES_CORE_OBJECTS_EXCHANGE_H

#include "MARSDefs.h"
#include <mars/utils/Vector.h>
#include <mars/utils/Quaternion.h>

#include <string>

namespace mars {
  namespace interfaces {

    /**
     * core_objects_exchange is a struct to exchange core objects information
     * between the modules of the simulation. This object is mostly used to
     * get the main object information without any overhead. For instance to
     * create lists of objects in the GUI.
     */
    struct core_objects_exchange {
      core_objects_exchange(){
        setZero();
      }

      void setZero(){
        pos.setZero();
        rot.setIdentity();
        visOffsetPos.setZero();
        visOffsetRot.setIdentity();
        index = 0;
        value = 0;
        groupID = 0;
      }

      /**
       * The name of the object.
       */
      std::string name;

      /**
       * The position of the object.
       */
      utils::Vector pos;

      /**
       * The orientation of the object. \sa Quaternion
       */
      utils::Quaternion rot;

      /**
       * The offset position of the visual representation.
       */
      utils::Vector visOffsetPos;

      /**
       * The offset orientation of the visual representation. \sa Quaternion
       */
      utils::Quaternion visOffsetRot;

      /**
       * The unique id of the object. A joint and a node can have the same id,
       * the ids are only unique for the same types of objects.
       */
      unsigned long index;

      /**
       * The groupID of the object if the object is a node.
       */
      unsigned int groupID;

      /**
       * A value that is used in different ways depending on the type of object.
       */
      sReal value;
    }; // end of struct core_objects_exchange

  } // end of namespace interfaces
} // end of namespace mars

#endif /* MARS_INTERFACES_CORE_OBJECTS_EXCHANGE_H */
