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
 * \file CFGParam.cpp
 * \author Michael Rohn
 * \brief CFGParam is the root class for param objects
 *
 * Version 0.1
 */

#include "CFGParam.h"

namespace mars {
  namespace cfg_manager {

    using std::string;
    using std::vector;

    CFGParam::CFGParam(const cfgParamId &_id, const string &_group,
                       const string &_name, const cfgParamType &_type)
      : emptyString(""), noType(noTypeSet) {
      this->id    = _id;
      this->group = _group;
      this->paramName = _name;
      this->paramType = _type;
      this->options = noParamOption;
    }


    CFGParam::~CFGParam() {
      //cout << "destroy CFGParam" << endl;

      //vector<CFGClient*>::iterator iter;
      //mutexCFGClients.lock();
      //for (iter = cfgClients.begin(); iter != cfgClients.end(); ++iter) {
      //delete (*iter);
      //}
      //cfgClients.clear();
      //mutexCFGClients.unlock();
    }


    // PUBLIC

    const cfgParamType& CFGParam::getParamType() const {
      return paramType;
    }


    const cfgParamId& CFGParam::getId() const {
      return id;
    }


    const string& CFGParam::getGroup() const {
      return group;
    }


    const string& CFGParam::getName() const {
      return paramName;
    }


    unsigned char CFGParam::getOption() const {
      return options;
    }


    void CFGParam::setOption(const unsigned char _newOption) {
      options |= _newOption;
    }


    void CFGParam::unsetOption(const unsigned char _newOption) {
      unsigned char tmpChar = 11111111 ^ _newOption;
      options &= tmpChar;
    }


    void CFGParam::addClient(CFGClient *client) {
      if(client != NULL) {
        mutexCFGClients.lock();
        cfgClients.push_back(client);
        mutexCFGClients.unlock();
      } // if
    }


    void CFGParam::removeClient(CFGClient *client) {
      vector<CFGClient*>::iterator iter;
      mutexCFGClients.lock();
      for(iter = cfgClients.begin(); iter != cfgClients.end(); ++iter) {
        if(*iter == client) {
          cfgClients.erase(iter);
          break;
        } // if
      } // for
      mutexCFGClients.unlock();
    }


    bool CFGParam::getProperty(CFGProperty *property) const {
      unsigned int state = property->getState();
      if( (state & CFGProperty::allSetButValue) == CFGProperty::allSetButValue ) {
        CFGProperty *tmpProperty = propertys.at(property->getPropertyIndex());
        if( tmpProperty->isSameAs(*property) && tmpProperty->isValueSet() ) {
          double dValue = 0.0;
          int iValue = 0;
          bool bValue = false;
          string sValue = "";
          switch( property->getPropertyType() ) {
          case doubleProperty:
            tmpProperty->getValue(&dValue);
            property->setValue(dValue);
            break;
          case intProperty:
            tmpProperty->getValue(&iValue);
            property->setValue(iValue);
            break;
          case boolProperty:
            tmpProperty->getValue(&bValue);
            property->setValue(bValue);
            break;
          case stringProperty:
            tmpProperty->getValue(&sValue);
            property->setValue(sValue);
            break;
          default:
            return false;
          } //switch
          return true;
        } //if
      } //if
      return false;
    }


    void CFGParam::writeToYAML(YAML::Emitter &out) const {
      out << YAML::BeginMap;

      out << YAML::Key << "name";
      out << YAML::Value << paramName;

      out << YAML::Key << "type";
      out << YAML::Value << cfgParamTypeString[paramType];

      if (options & userSave) {
        out << YAML::Key << "userSave";
        out << YAML::Value << true;
      }

      if (options & saveOnClose) {
        out << YAML::Key << "saveOnClose";
        out << YAML::Value << true;
      }

      unsigned int i = 0;
      mutexPropertys.lock();
      for(i = 0; i < propertys.size(); ++i) {
        CFGProperty *prop = propertys.at(i);
        if(prop->isValueSet()) {
          out << YAML::Key << getPropertyNameByIndex(i);
          double dValue = 0.0;
          int iValue = 0;
          string sValue = "";
          bool bValue = false;
          switch (prop->getPropertyType()) {
          case intProperty:
            prop->getValue(&iValue);
            out << YAML::Value << iValue;
            break;
          case boolProperty:
            prop->getValue(&bValue);
            out << YAML::Value << bValue;
            break;
          case stringProperty:
            prop->getValue(&sValue);
            out << YAML::Value << sValue;
            break;
          case doubleProperty:
            prop->getValue(&dValue);
            out << YAML::Value << dValue;
            break;
          default:
            out << YAML::Value << "error";
            break;
          } //switch
        }
      }
      mutexPropertys.unlock();

      out << YAML::EndMap;
    }


    // PROTECTED

    void CFGParam::updateClients(const CFGProperty &property) {
      vector<CFGClient*>::iterator iter;
      cfgPropertyStruct tmpS = property.getAsStruct();

      mutexCFGClients.lock();
      for(iter = cfgClients.begin(); iter != cfgClients.end(); ++iter) {
        (*iter)->cfgUpdateProperty(tmpS);
      }
      mutexCFGClients.unlock();
    }


    void CFGParam::readFromYAML(const YAML::Node &node) {
      unsigned int index = 0;
      for(index = 0; index < getNrOfPropertys(); ++index) {
        readPropertyFromYAML(index, node);
      }
      readSaveSettingFromYAML(node);
    }


    void CFGParam::readSaveSettingFromYAML(const YAML::Node &node) {
#ifdef YAML_03_API
      if( const YAML::Node *pName = node.FindValue("userSave") ) {
        bool bValue = false;
        *pName >> bValue;
#else
      if( const YAML::Node &pName = node["userSave"] ) {
        bool bValue = pName.as<bool>();
#endif
        if( bValue ) {
          setOption(userSave);
        }
      }

#ifdef YAML_03_API
      if( const YAML::Node *pName = node.FindValue("saveOnClose") ) {
        bool bValue = false;
        *pName >> bValue;
#else
      if( const YAML::Node &pName = node["saveOnClose"] ) {
        bool bValue = pName.as<bool>();
        bValue = pName.as<bool>();
#endif
        if( bValue ) {
          setOption(saveOnClose);
        }
      }
    }


    void CFGParam::readPropertyFromYAML(unsigned int index, const YAML::Node &node) const {
      double dValue = 0.0;
      int iValue = 0;
      bool bValue = false;
      string sValue = "";

#ifdef YAML_03_API
      if( const YAML::Node *pName = node.FindValue( getPropertyNameByIndex(index) ) ) {
        switch( getPropertyTypeByIndex(index) ) {
        case doubleProperty:
          *pName >> dValue;
          propertys.at(index)->setValue(dValue);
          break;
        case intProperty:
          *pName >> iValue;
          propertys.at(index)->setValue(iValue);
          break;
        case boolProperty:
          *pName >> bValue;
          propertys.at(index)->setValue(bValue);
          break;
        case stringProperty:
          *pName >> sValue;
          propertys.at(index)->setValue(sValue);
          break;
        default:
          // do nothing
          break;
        } // switch
      } // if
#else
      if( const YAML::Node &pName = node[getPropertyNameByIndex(index)] ) {
        switch( getPropertyTypeByIndex(index) ) {
        case doubleProperty:
          dValue = pName.as<double>();
          propertys.at(index)->setValue(dValue);
          break;
        case intProperty:
          iValue = pName.as<int>();
          propertys.at(index)->setValue(iValue);
          break;
        case boolProperty:
          bValue = pName.as<bool>();
          propertys.at(index)->setValue(bValue);
          break;
        case stringProperty:
          sValue = pName.as<std::string>();
          propertys.at(index)->setValue(sValue);
          break;
        default:
          // do nothing
          break;
        } // switch
      } // if
#endif
    }


    bool CFGParam::writeProperty(const CFGProperty &property) const {
      unsigned int state = property.getState();
      if( (state & CFGProperty::allSet) == CFGProperty::allSet ) {
        double dValue = 0.0;
        int iValue = 0;
        bool bValue = false;
        string sValue = "";
        switch( property.getPropertyType() ) {
        case doubleProperty :
          if( property.getValue(&dValue) ) {
            return propertys.at( property.getPropertyIndex() )->setValue(dValue);
          } else {
            return false;
          }
        case intProperty :
          if( property.getValue(&iValue) ) {
            return propertys.at( property.getPropertyIndex() )->setValue(iValue);
          } else {
            return false;
          }
        case boolProperty :
          if( property.getValue(&bValue) ) {
            return propertys.at( property.getPropertyIndex() )->setValue(bValue);
          } else {
            return false;
          }
        case stringProperty :
          if( property.getValue(&sValue) ) {
            return propertys.at( property.getPropertyIndex() )->setValue(sValue);
          } else {
            return false;
          }
        default :
          // do nothing
          return false;
        } //switch
      } //if
      return false;
    }

  } // end namespace cfg_manager
} // end namespace mars
