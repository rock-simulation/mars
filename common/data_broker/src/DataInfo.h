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

#ifndef DATAINFO_H
#define DATAINFO_H

#ifdef _PRINT_HEADER_
  #warning "DataInfo.h"
#endif

#include <string>

namespace mars {

  namespace data_broker {

    /**
     * \brief indicates to nature of the data contained in a DataPackage
     */
    enum PackageFlag {
      DATA_PACKAGE_NO_FLAG         = 0,
      DATA_PACKAGE_READ_FLAG       = (1 << 0),
      DATA_PACKAGE_WRITE_FLAG      = (1 << 1),
      DATA_PACKAGE_READ_WRITE_FLAG = (DATA_PACKAGE_READ_FLAG | 
                                      DATA_PACKAGE_WRITE_FLAG),
      DATA_PACKAGE_ALL_FLAGS       = 0xFF
    };

    /** \brief Class containing information about a DataPackage. */
    class DataInfo {
    public:
      DataInfo();
      ~DataInfo();
      DataInfo(const DataInfo &other);
      DataInfo &operator=(const DataInfo &other);

      /** \brief A unique numeric identifier assigned by the DataBroker
       */
      unsigned long dataId;
      /** \brief A name describing the group/category this belongs to. 
       *         The groupName-dataName combination should be unique.
       */
      std::string groupName; 
      /** \brief A name describing this DataPackage within the group
       *         The groupName-dataName combination should be unique.
       */
      std::string dataName;
      PackageFlag flags;
    };

  } // end of namespace data_broker

} // end of namespace mars

#endif // DATAINFO_H

