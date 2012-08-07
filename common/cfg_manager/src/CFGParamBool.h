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
 * \file CFGParamBool.h
 * \author Michael Rohn
 * \brief CFGParamBool is a class derived from CFGParam to store values of the type bool
 *
 * Version 0.1
 */

#ifndef CFG_PARAM_BOOL_H
#define CFG_PARAM_BOOL_H

#ifdef _PRINT_HEADER_
  #warning "CFGParamBool.h"
#endif

#include "CFGParam.h"

namespace mars {
  namespace cfg_manager {

    class CFGParamBool : public CFGParam {

    public:
      enum paramProperty {
        value = 0,
        dstNrOfParamPropertys
      };

      static const std::string paramPropertyName[dstNrOfParamPropertys];
      static const cfgPropertyType paramPropertyType[dstNrOfParamPropertys];


    public:
      CFGParamBool(const cfgParamId &_id, const std::string &_group,
                   const std::string &_name);
      ~CFGParamBool();

      virtual bool setProperty(const CFGProperty &_property);

      virtual const std::string& getPropertyNameByIndex(unsigned int _index) const ;
      virtual const cfgPropertyType& getPropertyTypeByIndex(unsigned int _index) const;
      virtual unsigned int getPropertyIndexByName(const std::string &_name) const;

      virtual unsigned int getNrOfPropertys() const;

      static CFGParam* createParam(const cfgParamId &_id, const std::string &_group,
                                   const YAML::Node &node);


    private:
      bool setPropertyValue(const CFGProperty &_property) const;

    }; // end class CFGParamBool

  } // end namespace cfg_manager
} // end namespace mars

#endif /* CFG_PARAM_BOOL_H */
