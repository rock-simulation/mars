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
 * \file CFGParamString.cpp
 * \author Michael Rohn
 * \brief CFGParamString is a class derived from CFGParam to store values of the type string
 *
 * Version 0.1
 */

#include "CFGParamString.h"

#include <iostream>

namespace mars {
  namespace cfg_manager {

    using std::string;
    using std::vector;

    const string CFGParamString::paramPropertyName[dstNrOfParamPropertys] = {
      "value"
    };

    const cfgPropertyType CFGParamString::paramPropertyType[dstNrOfParamPropertys] = {
      stringProperty
    };


    CFGParamString::CFGParamString(const cfgParamId &_id,
                                   const string &_group, const string &_name)
      : CFGParam(_id, _group, _name, stringParam) {
      //cout << "create CFGParamString" << endl;
      unsigned int i = 0;
      // create every single CFGProperty
      for(i = 0; i < dstNrOfParamPropertys; ++i) {
        CFGProperty *newProperty = new CFGProperty;
        newProperty->setParamId(_id);
        newProperty->setPropertyIndex(i);
        newProperty->setPropertyType(paramPropertyType[i]);
        mutexPropertys.lock();
        propertys.push_back(newProperty);
        mutexPropertys.unlock();
      } // for
    }


    CFGParamString::~CFGParamString() {
      //cout << "destroy CFGParamString" << endl;

      vector<CFGProperty*>::iterator iter;
      mutexPropertys.lock();
      for(iter = propertys.begin(); iter != propertys.end(); ++iter) {
        delete (*iter);
      } // for
      propertys.clear();
      mutexPropertys.unlock();
    }


    // PUBLIC

    bool CFGParamString::setProperty(const CFGProperty &_property) {
      unsigned int state = _property.getState();
      bool rValue = false;
      if( state & CFGProperty::allSet ) {
        switch( _property.getPropertyIndex() ) {
        case value :
          rValue = setPropertyValue(_property);
          break;
        default :
          rValue = false;
        } // switch
      }
      if(rValue) {
        updateClients(_property);
      }
      return rValue;
    }


    CFGParam* CFGParamString::createParam(const cfgParamId &_id,
                                          const string &_group,
                                          const YAML::Node &node) {
      /* *** OLD SOURCE FOR YAML-CPP-0-2-5 ***
      //cout << "creating new stringParam" << endl;
      //string name = node["name"];
      */
#ifdef YAML_03_API
      if(const YAML::Node *pName = node.FindValue("name")) {
        std::string name;
        *pName >> name;
#else
      if(const YAML::Node &pName = node["name"]) {
        std::string name = pName.as<std::string>();
#endif
        CFGParamString *paramString = new CFGParamString(_id, _group, name);
        paramString->readFromYAML(node);
        return (CFGParam*)paramString;
      } else {
        std::cout << "Key 'name' doesn't exist\n";
        return NULL;
      }
    }


    const string& CFGParamString::getPropertyNameByIndex(unsigned int _index) const {
      if( _index < dstNrOfParamPropertys ) {
        return paramPropertyName[_index];
      } else {
        return emptyString;
      }
    }


    const cfgPropertyType& CFGParamString::getPropertyTypeByIndex(unsigned int _index) const {
      if( _index < dstNrOfParamPropertys ) {
        return paramPropertyType[_index];
      } else {
        return noType;
      }
    }


    unsigned int CFGParamString::getPropertyIndexByName(const string &_name) const {
      unsigned int index;
      for(index = 0; index < dstNrOfParamPropertys; ++index) {
        if( _name == getPropertyNameByIndex(index) ) {
          return index;
        } // if
      } // for
      return 0;
    }


    unsigned int CFGParamString::getNrOfPropertys() const {
      return dstNrOfParamPropertys;
    }


    //PRIVATE

    bool CFGParamString::setPropertyValue(const CFGProperty &_property) const {
      if( _property.isValueSet() &&
          _property.getPropertyType() == paramPropertyType[value] ) {
        return writeProperty(_property);
      } // if
      return false;
    }

  } // end namespace cfg_manager
} // end namespace mars
