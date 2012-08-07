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

#include "DataInfo.h"

namespace mars {

  namespace data_broker {

    DataInfo::DataInfo() {
      dataId = 0;
      groupName = "";
      dataName = "";
      flags = DATA_PACKAGE_NO_FLAG;
    }
    
    DataInfo::~DataInfo() {
    }

    DataInfo::DataInfo(const DataInfo &other) {
      *this = other;
    }

    DataInfo &DataInfo::operator=(const DataInfo &other) {
      if(this == &other) {
        return *this;
      }
      dataId = other.dataId;
      flags = other.flags;
      groupName = other.groupName.c_str();
      dataName = other.dataName.c_str();
      return *this;
    }

  } // end of namespace data_broker
  
} // end of namespace mars
