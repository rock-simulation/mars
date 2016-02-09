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
 * \file CFGParamInt.cpp
 * \author Michael Rohn
 * \brief CFGParamInt is a class derived from CFGParam to store values of the type int
 *
 * Version 0.1
 */

#include "CFGParamInt.h"

#include <iostream>

namespace mars {
  namespace cfg_manager {

    using std::string;
    using std::vector;

    const string CFGParamInt::paramPropertyName[dstNrOfParamPropertys] = {
      "value",
      "min",
      "max",
      "stepsize"
    };

    const cfgPropertyType CFGParamInt::paramPropertyType[dstNrOfParamPropertys] = {
      intProperty,
      intProperty,
      intProperty,
      intProperty
    };


    CFGParamInt::CFGParamInt(const cfgParamId &_id, const string &_group,
                             const string &_name)
      : CFGParam(_id, _group, _name, intParam) {
      //cout << "create CFGParamInt" << endl;
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
      }
    }


    CFGParamInt::~CFGParamInt() {
      //cout << "destroy CFGParamInt" << endl;

      vector<CFGProperty*>::iterator iter;
      mutexPropertys.lock();
      for(iter = propertys.begin(); iter != propertys.end(); ++iter) {
        delete (*iter);
      }
      propertys.clear();
      mutexPropertys.unlock();
    }


    // PUBLIC

    bool CFGParamInt::setProperty(const CFGProperty &_property) {
      unsigned int state = _property.getState();
      bool rValue = false;
      if( state & CFGProperty::allSet ) {
        switch ( _property.getPropertyIndex() ) {
        case value :
          rValue = setPropertyValue(_property);
          break;
        case min :
          rValue = setPropertyMin(_property);
          break;
        case max :
          rValue = setPropertyMax(_property);
          break;
        case stepsize :
          rValue = setPropertyStepsize(_property);
          break;
        default :
          rValue = false;
        } // switch
        if(rValue) {
          updateClients(_property);
        }
      }
      return rValue;
    }


    CFGParam* CFGParamInt::createParam(const cfgParamId &_id,
                                       const string &_group,
                                       const YAML::Node &node) {
      /* *** OLD SOURCE FOR YAML-CPP-0-2-5 ***
      //cout << "creating new intParam" << endl;
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
        CFGParamInt *paramInt = new CFGParamInt(_id, _group, name);
        paramInt->readFromYAML(node);
        return (CFGParam*)paramInt;
      } else {
        std::cout << "Key 'name' doesn't exist\n";
        return NULL;
      }
    }


    const string& CFGParamInt::getPropertyNameByIndex(unsigned int _index) const {
      if( _index < dstNrOfParamPropertys ) {
        return paramPropertyName[_index];
      } else {
        return emptyString;
      } // if
    }


    const cfgPropertyType& CFGParamInt::getPropertyTypeByIndex(unsigned int _index) const {
      if( _index < dstNrOfParamPropertys ) {
        return paramPropertyType[_index];
      } else {
        return noType;
      } // if
    }


    unsigned int CFGParamInt::getPropertyIndexByName(const string &_name) const {
      unsigned int index;
      for(index = 0; index < dstNrOfParamPropertys; ++index) {
        if( _name == getPropertyNameByIndex(index) ) {
          return index;
        } // if
      } // for
      return 0;
    }


    unsigned int CFGParamInt::getNrOfPropertys() const {
      return dstNrOfParamPropertys;
    }


    //PRIVATE

    bool CFGParamInt::setPropertyValue(const CFGProperty &_property) const {
      if( _property.isValueSet() &&
          _property.getPropertyType() == paramPropertyType[value] ) {
        int newValue;
        _property.getValue(&newValue);

        // test for min-boundary
        if ( propertys.at(min)->isValueSet() ) {
          int minValue;
          propertys.at(min)->getValue(&minValue);
          if (newValue < minValue) {
            return false;
          }
        }

        // test for max-boundary
        if ( propertys.at(max)->isValueSet() ) {
          int maxValue;
          propertys.at(max)->getValue(&maxValue);
          if (newValue > maxValue) {
            return false;
          }
        }
        return writeProperty(_property);
      } // if
      return false;
    }


    bool CFGParamInt::setPropertyMin(const CFGProperty &_property) const {
      if( _property.isValueSet() &&
          _property.getPropertyType() == paramPropertyType[min] ) {
        int newMin;
        _property.getValue(&newMin);

        // test if min > max
        if( propertys.at(max)->isValueSet() ) {
          int maxValue;
          propertys.at(max)->getValue(&maxValue);
          if (newMin > maxValue) {
            return false;
          }
        }

        // test if min > value
        if( propertys.at(value)->isValueSet() ) {
          int currentValue;
          propertys.at(value)->getValue(&currentValue);
          if(newMin > currentValue) {
            return false;
          }
        }

        return writeProperty(_property);
      } // if
      return false;
    }


    bool CFGParamInt::setPropertyMax(const CFGProperty &_property) const {
      if( _property.isValueSet() &&
          _property.getPropertyType() == paramPropertyType[max] ) {
        int newMax;
        _property.getValue(&newMax);

        // test if max < min
        if( propertys.at(min)->isValueSet() ) {
          int minValue;
          propertys.at(min)->getValue(&minValue);
          if(newMax < minValue) {
            return false;
          }
        }

        // test if max < value
        if( propertys.at(value)->isValueSet() ) {
          int currentValue;
          propertys.at(value)->getValue(&currentValue);
          if(newMax < currentValue) {
            return false;
          }
        }

        return writeProperty(_property);
      } // if
      return false;
    }


    bool CFGParamInt::setPropertyStepsize(const CFGProperty &_property) const {
      if( _property.isValueSet() &&
          _property.getPropertyType() == paramPropertyType[stepsize] ) {
        return writeProperty(_property);
      } // if
      return false;
    }

  } // end namespace cfg_manager
} // end namespace mars
