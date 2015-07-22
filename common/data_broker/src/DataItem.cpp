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

#include "DataItem.h"
#include <cstdio>

namespace mars {

  namespace data_broker {

    DataItem::DataItem() {
    }
    DataItem::~DataItem() {
    }

    DataItem::DataItem(const DataItem &other) {
      *this = other;
    }
    // make sure to explicitly copy the string to avoid threading problems 
    // in certain std::string implementations.
    DataItem &DataItem::operator=(const DataItem &other) {
      if(this == &other) {
        return *this;
      }
      if (other.type == STRING_TYPE) {
        this->s = other.s.c_str();
      } else {
        this->l = other.l;
        this->d = other.d;
      }
      this->type = other.type;
      this->name = other.name.c_str();
      return *this;
    }

    ////////////////////////////////////
    // Getter Methods
    ////////////////////////////////////

    std::string DataItem::getName() const {
      return name;
    }

    bool DataItem::get(int *val) const {
      if(type != INT_TYPE) {
        return false;
      }
      *val = i;
      return true;
    }

    bool DataItem::get(unsigned int *val) const {
      if(type != UINT_TYPE) {
        return false;
      }
      *val = ui;
      return true;
    }

    bool DataItem::get(long *val) const {
      if(type != LONG_TYPE) {
        return false;
      }
      *val = l;
      return true;
    }

    bool DataItem::get(unsigned long *val) const {
      if(type != ULONG_TYPE) {
        return false;
      }
      *val = ul;
      return true;
    }

    bool DataItem::get(float *val) const {
      if(type != FLOAT_TYPE) {
        return false;
      }
      *val = f;
      return true;
    }

    bool DataItem::get(double *val) const {
      if(type != DOUBLE_TYPE) {
        return false;
      }
      *val = d;
      return true;
    }

    bool DataItem::get(std::string *val) const {
      if(type != STRING_TYPE) {
        return false;
      }
      *val = s.c_str();
      return true;
    }

    bool DataItem::get(bool *val) const {
      if(type != BOOL_TYPE) {
        return false;
      }
      *val = b;
      return true;
    }


    ////////////////////////////////////
    // Setter Methods
    ////////////////////////////////////

    void DataItem::setName(const std::string &newName) {
      name = newName.c_str();
    }

    bool DataItem::set(int val) {
      if(type != INT_TYPE) {
        return false;
      }
      i = val;
      return true;
    }

    bool DataItem::set(unsigned int val) {
      if(type != UINT_TYPE) {
        return false;
      }
      ui = val;
      return true;
    }

    bool DataItem::set(long val) {
      if(type != LONG_TYPE) {
        return false;
      }
      l = val;
      return true;
    }

    bool DataItem::set(unsigned long val) {
      if(type != ULONG_TYPE) {
        return false;
      }
      ul = val;
      return true;
    }

    bool DataItem::set(float val) {
      if(type != FLOAT_TYPE) {
        return false;
      }
      f = val;
      return true;
    }

    bool DataItem::set(double val) {
      if(type != DOUBLE_TYPE) {
        return false;
      }
      d = val;
      return true;
    }

    bool DataItem::set(const std::string &val) {
      if(type != STRING_TYPE) {
        return false;
      }
      s = val.c_str();
      return true;
    }

    bool DataItem::set(bool val) {
      if(type != BOOL_TYPE) {
        return false;
      }
      b = val;
      return true;
    }


  } // end of namespace data_broker

} // end of namespace mars
