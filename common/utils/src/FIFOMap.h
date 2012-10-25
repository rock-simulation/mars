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

#ifndef MARS_UTILS_FIFO_MAP_H
#define MARS_UTILS_FIFO_MAP_H

#ifdef _PRINT_HEADER_
#warning "FIFOMap.h"
#endif

#include <map>
#include <list>
#include <iterator>
#include <iostream>
#include <algorithm>

namespace mars {
  namespace utils {

    template <typename Key, typename T>
    class FIFOItem {
    public:
      FIFOItem(const Key &k, T &val)
        : first(k), second(val)
      {}
      bool operator==(const FIFOItem &other) const {
        return first == other.first;
      }

      Key first;
      T &second;
    }; // end of class FIFOItem

    template <typename Key, typename T>
    class FIFOMap : public std::map<Key, T> {
    public:
      typedef typename std::map<Key, T>::iterator mapIterator;
      typedef typename std::map<Key, T>::const_iterator const_mapIterator;
      typedef typename std::list<FIFOItem<Key, T> >::iterator iterator;
      typedef typename std::list<FIFOItem<Key, T> >::const_iterator const_iterator;

      /* iterator stuff */
      iterator begin()
      { return insertOrder.begin(); }
      const_iterator begin() const
      { return insertOrder.begin(); }
      iterator end()
      { return insertOrder.end(); }
      const_iterator end() const
      { return insertOrder.end(); }

      /* ctor, copy-ctor and assignment operator */
      FIFOMap() {}
      FIFOMap(const FIFOMap<Key, T> &other)
      { *this = other; }
      FIFOMap<Key, T>& operator=(const FIFOMap<Key, T> &other);

      /* element access */
      T& operator[](const Key &x);

      /* modifieres */
      std::pair<iterator, bool> insert(const std::pair<const Key, T> &x);

      /* not implemented
         iterator insert(iterator position, const value_type &x);
         template <class InputIterator>
         void insert ( InputIterator first, InputIterator last );
      */
    
      void erase(iterator position);
      size_t erase(const Key &x);
      void erase(iterator first, iterator last);
      void swap( FIFOMap<Key, T> &other);
      void clear();
    
      /* operations */
      iterator find(const Key &x);
      const_iterator find(const Key &x) const
      { return const_iterator(const_cast<FIFOMap*>(this)->find(x)); }

      /* not implemented yet;
         iterator lower_bound ( const key_type& x );
         const_iterator lower_bound ( const key_type& x ) const;
         iterator upper_bound ( const key_type& x );
         const_iterator upper_bound ( const key_type& x ) const;
         pair<iterator,iterator> equal_range ( const key_type& x );
         pair<const_iterator,const_iterator> equal_range ( const key_type& x ) const;
      */

    private:
      std::list<FIFOItem<Key, T> > insertOrder;

    }; // end of class FIFOMap


    /*********************
     * Implementation    *
     *********************/

    template<typename Key, typename T>
    FIFOMap<Key, T>& FIFOMap<Key, T>::operator=(const FIFOMap<Key, T> &other) {
      if(this == &other)
        return *this;
      clear();
      std::map<Key, T>::operator=(other);
      for(const_iterator it = other.begin(); it != other.end(); ++it) {
        FIFOItem<Key, T> newItem(it->first, 
                                 std::map<Key, T>::operator[](it->first));
        insertOrder.push_back(newItem);
      }
      return *this;
    }

    /* element access */
    template<typename Key, typename T>
    T& FIFOMap<Key, T>::operator[](const Key &x) {
      mapIterator it = std::map<Key, T>::find(x);
      if(it != std::map<Key, T>::end()) {
        return it->second;
      } else {
        FIFOItem<Key, T> newItem(x, std::map<Key, T>::operator[](x));
        insertOrder.push_back(newItem);
        return newItem.second;
      }
    }

    /* modifieres */
    template<typename Key, typename T>
    std::pair<typename FIFOMap<Key, T>::iterator, bool> FIFOMap<Key, T>::insert(const std::pair<const Key, T> &x) {
      std::cerr << "FIFOMap::insert is untested" << std::endl;
      mapIterator it = this->find(x.first);
      if(it != std::map<Key, T>::end()) {
        return std::make_pair(std::find(insertOrder.begin(), 
                                        insertOrder.end(), 
                                        FIFOItem<Key, T>(it->first,
                                                         it->second)), 
                              false);
      } else {
        std::pair<mapIterator, bool> tmp;
        tmp = std::map<Key, T>::insert(x);
        insertOrder.push_back(FIFOItem<Key, T>(x.first, tmp.first->second));
        return std::make_pair(--insertOrder.end(), true);
      }
    }
    
    template<typename Key, typename T>
    void FIFOMap<Key, T>::erase(FIFOMap<Key, T>::iterator position) {
      std::cerr << "FIFOMap::erase is untested" << std::endl;
      std::map<Key, T>::erase(position->first);
      insertOrder.erase(position);
    }

    template<typename Key, typename T>
    size_t FIFOMap<Key, T>::erase(const Key &x) {
      std::cerr << "FIFOMap::erase is untested" << std::endl;
      size_t ret = std::map<Key, T>::erase(x);
      if(ret) {
        for(iterator it = begin(); it != end(); ++it) {
          if(it->first == x) {
            insertOrder.erase(it);
            break;
          }
        }
      }
      return ret;
    }

    template<typename Key, typename T>
    void FIFOMap<Key, T>::erase(FIFOMap::iterator first,
                                FIFOMap::iterator last) {
      std::cerr << "FIFOMap::erase is untested" << std::endl;
      for(iterator it = first; it != last; /* do nothing */) {
        std::map<Key, T>::erase(it->first);
        it = insertOrder.erase(it);
      }
    }

    template<typename Key, typename T>
    void FIFOMap<Key, T>::swap(FIFOMap<Key, T> &other) {
      std::cerr << "FIFOMap::swap is untested" << std::endl;
      std::map<Key, T>::swap(other);
      insertOrder.swap(other.insertOrder);
    }

    template<typename Key, typename T>
    void FIFOMap<Key, T>::clear() {
      std::map<Key, T>::clear();
      insertOrder.clear();
    }
    
    /* operations */
    template<typename Key, typename T>
    typename FIFOMap<Key, T>::iterator FIFOMap<Key, T>::find(const Key &x) {
      mapIterator it = std::map<Key, T>::find(x);
      if(it != std::map<Key, T>::end()) {
        return std::find(insertOrder.begin(), 
                         insertOrder.end(), 
                         FIFOItem<Key, T>(it->first, 
                                          it->second));
      }
      return end();
    }

  } // end of namespace utils
} // end of namespace mars

#endif /* MARS_UTILS_FIFO_MAP_H */
