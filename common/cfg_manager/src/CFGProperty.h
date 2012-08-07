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
 * \file CFGProperty.h
 * \author Michael Rohn
 * \brief CFGProperty holds the value for one property of a CFGParam
 *
 * Version 0.1
 */

#ifndef CFG_PROPERTY_H
#define CFG_PROPERTY_H

#ifdef _PRINT_HEADER_
  #warning "CFGProperty.h"
#endif

#include "CFGDefs.h"

#include <string>

namespace mars {
  namespace cfg_manager {

    class CFGProperty {

    public:
      enum propertyOption {
        nothingSet = 0,
        paramIdSet = 1,
        propertyIndexSet = 2,
        propertyTypeSet = 4,
        propertyValueSet = 8,
        allSetButValue = 7,
        allSet = 15
      };

    public:
      CFGProperty();
      ~CFGProperty();

      const cfgParamId& getParamId() const;
      bool setParamId(const cfgParamId &_id);
      bool changeParamId(const cfgParamId &_id);

      unsigned int getPropertyIndex() const;
      bool setPropertyIndex(const unsigned int _propertyIndex);

      const cfgPropertyType& getPropertyType() const;
      bool setPropertyType(const cfgPropertyType &_propertyType);

      unsigned int getState() const;

      cfgPropertyStruct getAsStruct() const;

      bool isSameAs(const CFGProperty &property) const;

      bool isValueSet() const;

      bool setValue(const int    _iValue);
      bool setValue(const bool   _bValue);
      bool setValue(const double _dValue);
      bool setValue(const std::string &_sValue);

      bool getValue(int    *value) const;
      bool getValue(bool   *value) const;
      bool getValue(double *value) const;
      bool getValue(std::string *value) const;

    private:
      cfgParamId paramId;
      unsigned int propertyIndex;
      cfgPropertyType propertyType;

      int    iValue;
      bool   bValue;
      double dValue;
      std::string sValue;

      unsigned int propertyState;

    }; // end class CFGProperty

  } // end namespace cfg_manager
} // end namespace mars

#endif /* CFG_PROPERTY_H */
