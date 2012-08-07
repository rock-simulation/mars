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

/**
 * \file DataItem.h
 */

#ifndef DATAITEM_H
#define DATAITEM_H

#ifdef _PRINT_HEADER_
  #warning "DataItem.h"
#endif

#include <vector>
#include <string>

namespace mars {

  namespace data_broker {

    /** \brief indicates the type of a \ref data_broker::DataItem "DataItem" */
    enum DataType {
      UNDEFINED_TYPE,
      INT_TYPE,
      LONG_TYPE,
      FLOAT_TYPE,
      DOUBLE_TYPE,
      BOOL_TYPE,
      STRING_TYPE,
    };

    struct DataElement;
    struct DataItemConnection {
      DataElement *fromElement, *toElement;
      long fromDataItemIndex, toDataItemIndex;
    };

    /** \brief class containing a single value.
     */
    class DataItem {
    public:
      DataItem();
      ~DataItem();

      DataItem(const DataItem &other);
      DataItem &operator=(const DataItem &other);

      DataType type; ///< the type of the DataItem
      //    std::string name;
      union {
        int i;
        long l;
        float f;
        double d;
        bool b;
      };
      std::string s;

      std::string getName() const;
      void setName(const std::string &newName);

      /**
       * \brief tries to retrieve the value from this DataItem
       * \param val A pointer to a variable where the value can be written to.
       * \return \c true if the value was successfully retrieved.
       *         \c false if the pointer \a val was of the wrong type. 
       *         In the latter case the content of the pointer remains
       *         unchanged.
       */
      bool get(int *val) const;
      /// \copydoc get(int*) const
      bool get(long *val) const;
      /// \copydoc get(int*) const
      bool get(float *val) const;
      /// \copydoc get(int*) const
      bool get(double *val) const;
      /// \copydoc get(int*) const
      bool get(std::string *val) const;
      /// \copydoc get(int*) const
      bool get(bool *val) const;

      /**
       * \brief tries to set the value of this DataItem
       * \param val The value to which the DataItem should be set.
       * \return \c true if the value was successfully set.
       *         \c false if the type of \a val does not match the \ref type of
       *         this DataItem. In the latter case the current value of the
       *         DataItem is unchanged.
       */
      bool set(int val);
      /// \copydoc set(int val)
      bool set(long val);
      /// \copydoc set(int val)
      bool set(float val);
      /// \copydoc set(int val)
      bool set(double val);
      /// \copydoc set(int val)
      bool set(const std::string &val);
      /// \copydoc set(int val)
      bool set(bool val);

    private:
      std::string name;

    }; // end of class DataItem

  } // end of namespace data_broker

} // end of namespace mars

#endif // DATAITEM_H

