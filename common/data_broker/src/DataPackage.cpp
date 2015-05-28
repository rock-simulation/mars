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

#include "DataPackage.h"

namespace mars {

  namespace data_broker {

    DataPackage::DataPackage() {
    }

    DataPackage::~DataPackage() {
      clear();
    }

    // we need theses because the std::string in the nameLookup map
    // needs to be deep-copied to be thread safe.
    DataPackage::DataPackage(const DataPackage &other) {
      *this = other;
    }
    DataPackage &DataPackage::operator=(const DataPackage &other) {
      if(this == &other) {
        return *this;
      }
      package = other.package;
      /*
        std::map<std::string, int>::const_iterator lookupIt;
        for(lookupIt = other.nameLookup.begin();
        lookupIt != other.nameLookup.end();
        ++lookupIt)
        nameLookup[lookupIt->first.c_str()] = lookupIt->second;
      */
      return *this;
    }


    /////////////////////////////////////////
    // Getter Methods
    /////////////////////////////////////////

    DataType DataPackage::getType(const std::string &itemName) const {
      const DataItem *dataItem = getItemByName(itemName);
      return (dataItem ? dataItem->type : UNDEFINED_TYPE);
    }

    DataType DataPackage::getType(long index) const {
      if((0 <= index) && (index < static_cast<long>(package.size())))
        return package[index].type;
      else
        return UNDEFINED_TYPE;
    }

    long DataPackage::getIndexByName(const std::string &itemName) const {
      std::vector<DataItem>::const_iterator it;
      long i = 0;
      for(it = package.begin(); it != package.end(); ++it, ++i) {
        if(itemName == it->getName()) {
          return i;
        }
      }
      return -1;
    }
    /*
      long DataPackage::getIndexByName(const std::string &itemName) const {
      std::map<std::string, int>::const_iterator lookupIt;
      lookupIt = nameLookup.find(itemName);
      return ((lookupIt != nameLookup.end()) ? lookupIt->second : -1);
      }
    */
    const DataItem *DataPackage::getItemByName(const std::string &itemName) const {
      return const_cast<DataPackage*>(this)->getItemByName(itemName);
    }

    DataItem *DataPackage::getItemByName(const std::string &itemName) {
      std::vector<DataItem>::iterator it;

      for(it = package.begin(); it != package.end(); ++it) {
        if(itemName == it->getName()) {
          return &(*it);
        }
      }
      return NULL;
    }

    /////////////////////////////////////////
    // Adder Methods
    /////////////////////////////////////////

    void DataPackage::add(const std::string &itemName, int val) {
      DataItem item;
      item.setName(itemName);
      item.type = INT_TYPE;
      item.i = val;
      add(item);
    }

    void DataPackage::add(const std::string &itemName, unsigned int val) {
      DataItem item;
      item.setName(itemName);
      item.type = UINT_TYPE;
      item.i = val;
      add(item);
    }

    void DataPackage::add(const std::string &itemName, long val) {
      DataItem item;
      item.setName(itemName);
      item.type = LONG_TYPE;
      item.l = val;
      add(item);
    }

    void DataPackage::add(const std::string &itemName, unsigned long val) {
      DataItem item;
      item.setName(itemName);
      item.type = ULONG_TYPE;
      item.l = val;
      add(item);
    }

    void DataPackage::add(const std::string &itemName, float val) {
      DataItem item;
      item.setName(itemName);
      item.type = FLOAT_TYPE;
      item.f = val;
      add(item);
    }

    void DataPackage::add(const std::string &itemName, double val) {
      DataItem item;
      item.setName(itemName);
      item.type = DOUBLE_TYPE;
      item.d = val;
      add(item);
    }

    void DataPackage::add(const std::string &itemName, const std::string &val) {
      DataItem item;
      item.setName(itemName);
      item.type = STRING_TYPE;
      item.s = val.c_str();
      add(item);
    }

    void DataPackage::add(const std::string &itemName, bool val) {
      DataItem item;
      item.setName(itemName);
      item.type = BOOL_TYPE;
      item.b = val;
      add(item);
    }

  } // end of namespace data_broker

} // end of namespace mars
