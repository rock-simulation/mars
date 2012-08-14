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
 * \file DataPackageMapping.h 
 * \author Lorenz Quack
 */

#ifndef DATAPACKAGEMAPPING_H
#define DATAPACKAGEMAPPING_H

#ifdef _PRINT_HEADER_
  #warning "DataPackageMapping.h"
#endif

#include "DataPackage.h"

#include <vector>
#include <string>
#include <cstdio>

namespace mars {

  namespace data_broker {

    // exclude these internal classed from doxygen 
    /// \cond DEBUG
    class DataItemAccessorBase {
    public:
      virtual bool getValue(const DataPackage &package) = 0;
      virtual bool setValue(DataPackage *package) const = 0;
      virtual bool createValue(DataPackage *package) = 0;
      virtual bool getIndex(const DataPackage &package) = 0;
    };

    template<typename T> class DataItemAccessor : public DataItemAccessorBase {
    public:
      DataItemAccessor(const std::string &itemName, T *var)
        : DataItemAccessorBase() {
        this->itemName = itemName;
        this->var = var;
        this->id = -1;
      }
      inline bool getValue(const DataPackage &package) {
        return package.get(id, var);
      }
      inline bool setValue(DataPackage *package) const {
        return package->set(id, *var);
      }
      inline bool getIndex(const DataPackage &package) {
        id = package.getIndexByName(itemName);
        return id != -1;
      }
      inline bool createValue(DataPackage *package) {
        package->add(itemName, *var);
        id = package->getIndexByName(itemName);
        return id != -1;
      }

    private:
      T *var;
      std::string itemName;
      long id;
    }; // end of class DataItemAccessor
    /// \endcond

    /**
     * \brief A DataPackageMapping can be used by a 
     *        \link ReceiverInterface Receiver \endlink to more conviniently 
     *        retrieve values from a DataPackage.
     *
     * The DataPackageMapping is used to map certain 
     * \link DataItem DataItem::names \endlink to variables.
     * This is done by calling \link DataPackageMapping::add add \endlink and 
     * giving the DataItem::name and a pointer to a variable of the appropriate
     * type. The user is responsible for making sure the pointer is valid 
     * during the lifetime of the DataPackageMapping object. 
     * When a new DataPackage is received it can be passed to the 
     * DataPackageMapping's update method and it will retrieve the values from 
     * the DataPackage and write them to the variables.
     */
    class DataPackageMapping {
    public:
      DataPackageMapping();

      /**
       * \brief Add a mapping from a DataItem::name to a variable.
       * \param itemName The name of a DataItem in the DataPackage that 
       *                 should be mapped.
       * \param var A pointer to variable that belongs to the DataItem.
       *            The user is responsible for making sure the pointer is
       *            valid during the lifetime of the DataPackageMapping object.
       */
      template<typename T> void add(const std::string &itemName, T *var) {
        accessors.push_back(new DataItemAccessor<T>(itemName, var));
      }

      /**
       * \brief Writes the values from the package to the mapped variables.
       * \param package The DataPackage whose DataItems should be retrieved.
       * This will retrieve all DataItems for which there is a mapping and 
       * write the value to the corresponding variable.
       */
      bool readPackage(const DataPackage &package);
      bool writePackage(DataPackage *package);

      void clear();

    private:
      bool first;
      std::vector<DataItemAccessorBase*> accessors;
    }; // end of class DataPackageMapping
    
  } // end of namespace data_broker

} // end of namespace mars

#endif // DATAPACKAGEMAPPING_H
