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
 * \file CFGParamDouble.cpp
 * \author Michael Rohn
 * \brief CFGParamDouble is a class derived from CFGParam to store values of the type double
 *
 * Version 0.1
 */

#include "CFGParamDouble.h"

#include <iostream>

namespace mars {
  namespace cfg_manager {

    using std::string;
    using std::vector;

    const string CFGParamDouble::paramPropertyName[dstNrOfParamPropertys] = {
      "value",
      "min",
      "max",
      "stepsize"
    };

    const cfgPropertyType CFGParamDouble::paramPropertyType[dstNrOfParamPropertys] = {
      doubleProperty,
      doubleProperty,
      doubleProperty,
      doubleProperty
    };


    CFGParamDouble::CFGParamDouble(const cfgParamId &_id, const string &_group,
                                   const string &_name)
      : CFGParam(_id, _group, _name, doubleParam) {
      //cout << "create CFGParamDouble" << endl;
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


    CFGParamDouble::~CFGParamDouble() {
      //cout << "destroy CFGParamDouble" << endl;

      vector<CFGProperty*>::iterator iter;
      mutexPropertys.lock();
      for(iter = propertys.begin(); iter != propertys.end(); ++iter) {
        delete (*iter);
      } // for
      propertys.clear();
      mutexPropertys.unlock();
    }


    // PUBLIC

    bool CFGParamDouble::setProperty(const CFGProperty &_property) {
      unsigned int state = _property.getState();
      bool rValue = false;
      if( state & CFGProperty::allSet ) {
        switch( _property.getPropertyIndex() ) {
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
      } // if
      if(rValue) {
        updateClients(_property);
      }
      return rValue;
    }


    CFGParam* CFGParamDouble::createParam(const cfgParamId &_id,
                                          const string &_group,
                                          const YAML::Node &node) {
      /* *** OLD SOURCE FOR YAML-CPP-0-2-5 ***
      //cout << "creating new doubleParam" << endl;
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
        CFGParamDouble *paramDouble = new CFGParamDouble(_id, _group, name);
        paramDouble->readFromYAML(node);
        return (CFGParam*)paramDouble;
      } else {
        std::cout << "Key 'name' doesn't exist\n";
        return NULL;
      }
    }


    const string& CFGParamDouble::getPropertyNameByIndex(unsigned int _index) const {
      if( _index < dstNrOfParamPropertys ) {
        return paramPropertyName[_index];
      } else {
        return emptyString;
      }
    }


    const cfgPropertyType& CFGParamDouble::getPropertyTypeByIndex(unsigned int _index) const {
      if( _index < dstNrOfParamPropertys ) {
        return paramPropertyType[_index];
      } else {
        return noType;
      }
    }


    unsigned int CFGParamDouble::getPropertyIndexByName(const string &_name) const {
      unsigned int index;
      for(index = 0; index < dstNrOfParamPropertys; ++index) {
        if( _name == getPropertyNameByIndex(index) ) {
          return index;
        } // if
      } // for
      return 0;
    }


    unsigned int CFGParamDouble::getNrOfPropertys() const {
      return dstNrOfParamPropertys;
    }


    //PRIVATE

    bool CFGParamDouble::setPropertyValue(const CFGProperty &_property) const {
      if( _property.isValueSet() &&
          _property.getPropertyType() == paramPropertyType[value] ) {
        double newValue;
        _property.getValue(&newValue);

        // test for min-boundary
        if( propertys.at(min)->isValueSet() ) {
          double minValue;
          propertys.at(min)->getValue(&minValue);
          if(newValue < minValue) {
            return false;
          }
        }

        // test for max-boundary
        if( propertys.at(max)->isValueSet() ) {
          double maxValue;
          propertys.at(max)->getValue(&maxValue);
          if(newValue > maxValue) {
            return false;
          }
        }

        return writeProperty(_property);
      } // if
      return false;
    }


    bool CFGParamDouble::setPropertyMin(const CFGProperty &_property) const {
      if( _property.isValueSet() &&
          _property.getPropertyType() == paramPropertyType[min] ) {
        double newMin;
        _property.getValue(&newMin);

        // test if min > max
        if( propertys.at(max)->isValueSet() ) {
          double maxValue;
          propertys.at(max)->getValue(&maxValue);
          if(newMin > maxValue) {
            return false;
          }
        }

        // test if min > value
        if( propertys.at(value)->isValueSet() ) {
          double currentValue;
          propertys.at(value)->getValue(&currentValue);
          if(newMin > currentValue) {
            return false;
          }
        }

        return writeProperty(_property);
      } // if
      return false;
    }


    bool CFGParamDouble::setPropertyMax(const CFGProperty &_property) const {
      if( _property.isValueSet() &&
          _property.getPropertyType() == paramPropertyType[max] ) {
        double newMax;
        _property.getValue(&newMax);

        // test if max < min
        if( propertys.at(min)->isValueSet() ) {
          double minValue;
          propertys.at(min)->getValue(&minValue);
          if(newMax < minValue) {
            return false;
          }
        }

        // test if max < value
        if( propertys.at(value)->isValueSet() ) {
          double currentValue;
          propertys.at(value)->getValue(&currentValue);
          if(newMax < currentValue) {
            return false;
          }
        }

        return writeProperty(_property);
      } // if
      return false;
    }


    bool CFGParamDouble::setPropertyStepsize(const CFGProperty &_property) const {
      if( _property.isValueSet() &&
          _property.getPropertyType() == paramPropertyType[stepsize] ) {
        return writeProperty(_property);
      } // if
      return false;
    }


  } // end namespace cfg_manager
} // end namespace mars
