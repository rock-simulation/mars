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

#include "DataPackageMapping.h"

namespace mars {
  
  namespace data_broker {
    
    DataPackageMapping::DataPackageMapping()
      : first(true)
    {}
    
    bool DataPackageMapping::readPackage(const DataPackage &package) {
      bool ret = true;
      if(first) {
        for(std::vector<DataItemAccessorBase*>::iterator it = accessors.begin();
            it != accessors.end(); ++it) {
          ret = (ret && (*it)->getIndex(package));
        }
        if(!ret) {
          //fprintf(stderr, "DataGetter: Could not get all indices\n");
          return ret;
        }
        first = false;
      }
      for(std::vector<DataItemAccessorBase*>::iterator it = accessors.begin();
          it != accessors.end(); ++it) {
        ret = (ret && (*it)->getValue(package));
      }
      return ret;
    }

    bool DataPackageMapping::writePackage(DataPackage *package) {
      /* Writing the package is a bit more difficult.
       * We don't know if the DataItems exist. Even after the first run it might
       * not exist because it could be an entirly new DataPackage.
       * For perfomance reasons we abandon the "Look Before You Leap" style in
       * favour of the "Easier to Ask for Forgiveness Than Permission" style.
       * Only when setting a DataItem fails do we try to create it.
       */
      bool ret = true;
      for(std::vector<DataItemAccessorBase*>::iterator it = accessors.begin();
          it != accessors.end(); ++it) {
        bool tmp = (*it)->setValue(package);
        if(!tmp) {
          ret = (ret && (*it)->createValue(package));
        }
      }
      return ret;
    }

    void DataPackageMapping::clear() {
      accessors.clear();
      first = true;
    }

  } // end of namespace data_broker

} // end of namespace mars
