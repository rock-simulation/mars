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
 * \file CFGParamBool.cpp
 * \author Michael Rohn
 * \brief CFGParamBool is a class derived from CFGParam to store values of the type bool
 *
 * Version 0.1
 */

#include "CFGParamBool.h"

#include <iostream>

namespace mars {
  namespace cfg_manager {

    using std::string;
    using std::vector;

    const string CFGParamBool::paramPropertyName[dstNrOfParamPropertys] = {
      "value"
    };

    const cfgPropertyType CFGParamBool::paramPropertyType[dstNrOfParamPropertys] = {
      boolProperty
    };


    CFGParamBool::CFGParamBool(const cfgParamId &_id, const string &_group,
                               const string &_name)
      : CFGParam(_id, _group, _name, boolParam) {
      //cout << "create CFGParamBool" << endl;
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


    CFGParamBool::~CFGParamBool() {
      //cout << "destroy CFGParamBool" << endl;

      vector<CFGProperty*>::iterator iter;
      mutexPropertys.lock();
      for(iter = propertys.begin(); iter != propertys.end(); ++iter) {
        delete (*iter);
      } // for
      propertys.clear();
      mutexPropertys.unlock();
    }


    // PUBLIC

    bool CFGParamBool::setProperty(const CFGProperty &_property) {
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


    CFGParam* CFGParamBool::createParam(const cfgParamId &_id,
                                        const string &_group,
                                        const YAML::Node &node) {
      /* *** OLD SOURCE FOR YAML-CPP-0-2-5 ***
      //cout << "creating new boolParam" << endl;
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
        // std::cout << "Key 'name' exists, with value '" << name << "'\n";
        CFGParamBool *paramBool = new CFGParamBool(_id, _group, name);
        paramBool->readFromYAML(node);
        return (CFGParam*)paramBool;
      } else {
        std::cout << "Key 'name' doesn't exist\n";
        return NULL;
      }
    }


    const string& CFGParamBool::getPropertyNameByIndex(unsigned int _index) const {
      if( _index < dstNrOfParamPropertys ) {
        return paramPropertyName[_index];
      } else {
        return emptyString;
      }
    }


    const cfgPropertyType& CFGParamBool::getPropertyTypeByIndex(unsigned int _index) const {
      if( _index < dstNrOfParamPropertys ) {
        return paramPropertyType[_index];
      } else {
        return noType;
      }
    }


    unsigned int CFGParamBool::getPropertyIndexByName(const string &_name) const {
      unsigned int index;
      for(index = 0; index < dstNrOfParamPropertys; ++index) {
        if( _name == getPropertyNameByIndex(index) ) {
          return index;
        } // if
      } // for
      return 0;
    }


    unsigned int CFGParamBool::getNrOfPropertys() const {
      return dstNrOfParamPropertys;
    }


    //PRIVATE

    bool CFGParamBool::setPropertyValue(const CFGProperty &_property) const {
      if( _property.isValueSet() &&
          _property.getPropertyType() == paramPropertyType[value] ) {
        return writeProperty(_property);
      } // if
      return false;
    }


  } // end namespace cfg_manager
} // end namespace mars

