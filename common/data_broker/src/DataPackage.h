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
 * \file DataPackage.h
 */

#ifndef DATAPACKAGE_H
#define DATAPACKAGE_H

#ifdef _PRINT_HEADER_
  #warning "DataPackage.h"
#endif

#include "DataItem.h"

#include <map>
#include <vector>
#include <string>


namespace mars {

  namespace data_broker {

    /** \brief A collection of \ref DataItem "DataItems" */
    class DataPackage {
    public:
      DataPackage();
      ~DataPackage();

      DataPackage(const DataPackage &other);
      DataPackage &operator=(const DataPackage &other);

      /** \brief returns the \ref DataItem at position \a index.
       *         There is no bounds checking
       */
      inline DataItem &operator[](size_t index) {
        return package[index];
      }
      /// \copybrief operator[](size_t)
      inline const DataItem &operator[](size_t index) const {
        return package[index];
      }

      /** \brief remove all \ref DataItem "DataItems" from this package */
      inline void clear() {
        package.clear();
      }

      /** \brief return the number of \ref DataItem "DataItems" in this package
       */
      inline size_t size() const {
        return package.size();
      }

      /** \brief returns \c true if there is no \ref DataItem in the package. 
       *         \c false otherwise.
       */
      inline bool empty() const {
        return package.empty();
      }

      /** \brief adds the \ref DataItem \a item to the end of the package. */
      inline void add(const DataItem &item) {
        //      nameLookup[item.name.c_str()] = package.size();
        package.push_back(item);
      }

      /** 
       * \brief gets the value of the DataItem with the given name
       * \param itemName The name of the DataItem whose value should be
       *                 retrieved.
       * \param val A pointer to a variable where the value can be written to.
       * \return \c true if the value was successfully retrieved.
       *         \c false if the pointer \a val was of the wrong type or
       *         the DataItem with the name \a itemName doesn't exist.
       *         In case \c false is returned the content of the pointer
       *         remains unchanged.
       */
      template<typename T> bool get(const std::string &itemName, T *val) const {
        const DataItem *dataItem = getItemByName(itemName);
        return (dataItem ? dataItem->get(val) : false);
      }

      /** 
       * \brief gets the value of the DataItem at the given index in the package
       * \param index The index into the package of the DataItem whose 
       *              value should be retrieved.
       * \param val A pointer to a variable where the value can be written to.
       * \return \c true if the value was successfully retrieved.
       *         \c false if the pointer \a val was of the wrong type or the
       *         index is out of bounds.
       *         In case \c false is returned the content of the pointer 
       *         remains unchanged.
       */
      template<typename T> bool get(long index, T *val) const {
        if((0 <= index) && (index < (long)package.size())) {
          return package[index].get(val);
        } else {
          return false;
        }
      }
      
      /**
       * \brief gets the type of the DataItem with the given name
       * \param itemName The name of the DataItem whose type should be 
       *                 retrieved.
       * \return The DataType of the DataItem if a DataItem named \a itemName
       *         exists. \ref DataType "DataType::UNDEFINED_TYPE" otherwise.
       */
      DataType getType(const std::string &itemName) const;

      /**
       * \brief gets the type of the DataItem at the given index in the package
       * \param index The index into the package of the DataItem whose 
       *              type should be retrieved.
       * \return The DataType of the DataItem if \a index is a valid. 
       *         \ref DataType "DataType::UNDEFINED_TYPE" otherwise.
       */
      DataType getType(long index) const;

      /**
       * \brief tries to set the value of the DataItem with the given name
       * \param itemName The name of the DataItem whose value should be set.
       * \param val The value to which the DataItem should be set.
       * \return \c true if the value was successfully set.
       *         \c false if the type of \a val does not match the \ref type of
       *         the DataItem or the DataItem doesn't exist.
       *         When \c false is returned the current value of the
       *         DataItem is unchanged.
       */
      template<typename T> bool set(const std::string &itemName, T val) {
        DataItem *dataItem = getItemByName(itemName);
        return (dataItem ? dataItem->set(val) : false);
      }

      /**
       * \brief tries to set the value of the DataItem at the given index
       *        in the package.
       * \param index The index into the package of the DataItem whose 
       *              value should be set.
       * \param val The value to which the DataItem should be set.
       * \return \c true if the value was successfully set.
       *         \c false if the type of \a val does not match the \ref type of
       *         the DataItem or the index is out of bounds.
       *         When \c false is returned the current value of the
       *         DataItem is unchanged.
       */
      template<typename T> bool set(long index, T val) {
        if((0 <= index) && (index < (long)package.size())) {
          return package[index].set(val);
        } else {
          return false;
        }
      }

      /**
       * \brief add a new DataItem to the end of the package.
       * \param itemName The name of the new DataItem.
       * \param val The initial value of the DataItem.
       *
       * The \ref DataItem::type will be derived from the type of \a val.
       * Note that the DataPackage does *not* check for name collisions.
       */
      void add(const std::string &itemName, int val);
      /// \copydoc add(const std::string&, int)
      void add(const std::string &itemName, long val);
      /// \copydoc add(const std::string&, int)
      void add(const std::string &itemName, float val);
      /// \copydoc add(const std::string&, int)
      void add(const std::string &itemName, double val);
      /// \copydoc add(const std::string&, int)
      void add(const std::string &itemName, const std::string &val);
      /// \copydoc add(const std::string&, int)
      void add(const std::string &itemName, bool val);

      /**
       * \brief returns the index of the \ref DataItem with the given name.
       * \param itemName The name of the DataItem whose index to retrieve.
       * \return The index of the \ref DataItem with the given name or -1
       *         if no such DataItem exists.
       */
      long getIndexByName(const std::string &itemName) const;


    private:
      DataItem* getItemByName(const std::string &name);
      const DataItem* getItemByName(const std::string &name) const;

      //    std::map<std::string, int> nameLookup;
      std::vector<DataItem> package;
      std::vector<DataItemConnection> connections;

    }; // end of class DataPackage

  } // end of namespace data_broker

} // end of namespace mars

#endif // DATAPACKAGE_H
