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
 * \file CFGManager.cpp
 * \author Michael Rohn
 * \brief CFGManager is a class to handle the config file
 *
 * Version 0.1
 */

#include "CFGManager.h"

#include "CFGParam.h"
#include "CFGParamDouble.h"
#include "CFGParamInt.h"
#include "CFGParamBool.h"
#include "CFGParamString.h"

#include <yaml-cpp03/yaml.h>

#include <sys/stat.h>
#include <fstream>
#include <sstream>

#include <mars/utils/MutexLocker.h>

namespace mars {
  namespace cfg_manager {

    using namespace std;

    CFGManager::CFGManager(lib_manager::LibManager *theManager,
                           const char *filename)
      : CFGManagerInterface(theManager),
        mutexCFGParams(utils::MUTEX_TYPE_RECURSIVE),
        mutexVecClients(utils::MUTEX_TYPE_RECURSIVE) {
      //cout << "create CFGManager" << endl;
      mutexNextId.lock();
      nextId = 1;
      mutexNextId.unlock();
    }


    CFGManager::~CFGManager() {
      //cout << "destroy CFGManager" << endl;

      cfgPropertyStruct path = getOrCreateProperty("Config", "config_path",
						   ".");
      path.sValue += "/mars_saveOnClose.yaml";
      writeConfig(path.sValue.c_str());

      mapIdToParam::iterator iter;
      mutexCFGParams.lock();
      for(iter = cfgParamsById.begin(); iter != cfgParamsById.end(); ++iter) {
        CFGParam *pointerToParam = iter->second;
        deleteParam(pointerToParam);
      } // for
      cfgParamsById.clear();
      cfgParamsByString.clear();
      mutexCFGParams.unlock();
      fprintf(stderr, "Delete cfg_manager\n");
    }


    // PUBLIC

    bool CFGManager::loadConfigFromStream(istream &in, const char *group) {
      if(!in.good()) {
        return false;
      }
      try {
        YAML::Parser parser(in);
        YAML::Node doc;
        YAML::Iterator it;
        string currentGroup = "";

        while(parser.GetNextDocument(doc)) {
          //cout << "Found document" << endl;

          for(it = doc.begin(); it != doc.end(); ++it) {
            it.first() >> currentGroup;
            //cout << "Found group: " << group << endl;
            if(group && (currentGroup != group))
              continue;

            const YAML::Node &paramNodes = it.second();
            readGroup(currentGroup, paramNodes);
          } // for

        } // while
      } catch(YAML::ParserException &e) {
        cout << e.what() << endl;
      } catch(YAML::BadDereference &e) {
        cout << e.what() << endl;
      }
      return true;
    }


    bool CFGManager::loadConfig(const char *filename) {
      return loadConfig(filename, NULL);
    }

    bool CFGManager::loadConfig(const char *filename, const char *group) {
      if( fileExists(filename) ) {
        cout << "found config file: " << filename << endl;
        ifstream fin(filename);
        bool ret = loadConfigFromStream(fin, group);
        fin.close();
        return ret;
      } else {
        cout << "config file not found: " << filename << endl;
        return false;
      }
    }


    bool CFGManager::loadConfigFromString(const std::string &configString) {
      istringstream in(configString);
      return loadConfigFromStream(in, NULL);
    }


    void CFGManager::writeConfig(const char *filename, const char *group,
                          const unsigned char saveOption) const {
      ofstream ofstr(filename);
      ofstr << writeConfigToString(group, saveOption);
      ofstr.close();
    }

    const std::string CFGManager::writeConfigToString(const char *group,
                                                      const unsigned char saveOption) const {
      YAML::Emitter out;
      mapStringToParam::const_iterator iter;
      string lastGroup = "";
      CFGParam *param = NULL;
      out << YAML::BeginMap;
      mutexCFGParams.lock();

      for(iter = cfgParamsByString.begin(); iter != cfgParamsByString.end(); ++iter) {
        param = iter->second;
        if(!group || param->getGroup() == string(group)) {
          if( (param->getOption() & saveOption)  || group) {
            if(lastGroup != param->getGroup()) {
              if(lastGroup != "") {
                out << YAML::EndSeq;
              }
              lastGroup = param->getGroup();
              out << YAML::Key << lastGroup;
              out << YAML::Value << YAML::BeginSeq;
            }
            param->writeToYAML(out);
          }
        }

      } //for
      out << YAML::EndSeq;
      out << YAML::EndMap;

      mutexCFGParams.unlock();

      return out.c_str();
    }

    cfgParamId CFGManager::createParam(const string &_group, const string &_name,
                                       const cfgParamType &_paramType) {
      if( (_group != "") && (_name != "") ) {
        cfgParamId newId;
        newId = getParamId(_group, _name);
        if( newId != 0 ) {
          return newId;
        } else {
          CFGParam *param = NULL;
          switch (_paramType) {
          case doubleParam :
            newId = getNextId();
            param = (CFGParam*) (new CFGParamDouble(newId, _group, _name));
            insertParam(param);
            break;
          case intParam :
            newId = getNextId();
            param = (CFGParam*) (new CFGParamInt(newId, _group, _name));
            insertParam(param);
            break;
          case boolParam :
            newId = getNextId();
            param = (CFGParam*) (new CFGParamBool(newId, _group, _name));
            insertParam(param);
            break;
          case stringParam :
            newId = getNextId();
            param = (CFGParam*) (new CFGParamString(newId, _group, _name));
            insertParam(param);
            break;
          default :
            newId = 0;
          } // switch
          if(newId != 0) {
            addedCFGParam(newId);
          }
          return newId;
        }
      }
      return 0;
    }


    bool CFGManager::getAllParams(vector<cfgParamInfo> *allParams) const {
      mapIdToParam::const_iterator iterId = cfgParamsById.begin();

      for(; iterId != cfgParamsById.end(); iterId++) {
        allParams->push_back(getParamInfo(iterId->first));
      }
      return true;
    }


    bool CFGManager::removeParam(const cfgParamId &_id) {
      utils::MutexLocker locker(&mutexCFGParams);
      mapIdToParam::iterator iterId = cfgParamsById.find(_id);
      mapStringToParam::iterator iterS;
      CFGParam *pointerToParam = NULL;

      if( iterId != cfgParamsById.end() ) {
        pointerToParam = iterId->second;

        string stringId = pointerToParam->getGroup() + ":" + pointerToParam->getName();
        iterS = cfgParamsByString.find(stringId);
        if( iterS != cfgParamsByString.end() ) {
          cfgParamsByString.erase(iterS);
        }

        deleteParam(pointerToParam);
        cfgParamsById.erase(iterId);
        removedCFGParam(_id);
        return true;
      } else {
        return false;
      }
    }


    bool CFGManager::removeParam(const string &_group, const string &_name) {
      utils::MutexLocker locker(&mutexCFGParams);
      string stringId = _group + ":" + _name;
      mapStringToParam::iterator iterS = cfgParamsByString.find(stringId);
      mapIdToParam::iterator iterId;
      CFGParam *pointerToParam = NULL;

      if( iterS != cfgParamsByString.end() ) {
        pointerToParam = iterS->second;

        cfgParamId id = pointerToParam->getId();
        iterId = cfgParamsById.find(id);
        if( iterId != cfgParamsById.end() ) {
          cfgParamsById.erase(iterId);
        }

        deleteParam(pointerToParam);
        cfgParamsByString.erase(iterS);
        removedCFGParam(id);
        return true;
      } else {
        return false;
      }
    }


    bool CFGManager::setProperty(const cfgPropertyStruct &_propertyS) {
      utils::MutexLocker locker(&mutexCFGParams);
      CFGParam *param = NULL;
      CFGProperty property;
      property.setParamId(_propertyS.paramId);
      property.setPropertyIndex(_propertyS.propertyIndex);
      property.setPropertyType(_propertyS.propertyType);
      switch (_propertyS.propertyType) {
      case boolProperty:
        property.setValue(_propertyS.bValue);
        break;
      case doubleProperty:
        property.setValue(_propertyS.dValue);
        break;
      case intProperty:
        property.setValue(_propertyS.iValue);
        break;
      case stringProperty:
        property.setValue(_propertyS.sValue);
        break;
      default:
        return false;
      } // switch
      if( getParam(&param, _propertyS.paramId) ) {
        return param->setProperty(property);
      } else {
        return false;
      }
    }


    bool CFGManager::getProperty(cfgPropertyStruct *_propertyS) const {
      utils::MutexLocker locker(&mutexCFGParams);
      CFGParam *param = NULL;
      CFGProperty tmp_prop;
      tmp_prop.setParamId(_propertyS->paramId);
      tmp_prop.setPropertyIndex(_propertyS->propertyIndex);
      tmp_prop.setPropertyType(_propertyS->propertyType);

      if( getParam(&param, _propertyS->paramId) ) {
        if( param->getProperty(&tmp_prop) ) {
          *_propertyS = tmp_prop.getAsStruct();
          return true;
        }
      }
      return false;
    }


    bool CFGManager::setProperty(const CFGProperty &_property) {
      utils::MutexLocker locker(&mutexCFGParams);
      CFGParam *param = NULL;
      if( getParam(&param, _property.getParamId()) ) {
        return param->setProperty(_property);
      } else {
        return false;
      }
    }


    bool CFGManager::getProperty(CFGProperty *_property) const {
      utils::MutexLocker locker(&mutexCFGParams);
      CFGParam *param = NULL;
      if( getParam(&param, _property->getParamId()) ) {
        return param->getProperty(_property);
      } else {
        return false;
      }
    }


    bool CFGManager::getPropertyValue(cfgParamId paramId,
                                      const string &_propertyName,
                                      double *rValue) const {
      utils::MutexLocker locker(&mutexCFGParams);
      CFGParam *param = NULL;
      if( getParam(&param, paramId) ) {
        CFGProperty property;
        property.setParamId( paramId );
        property.setPropertyIndex( param->getPropertyIndexByName(_propertyName) );
        property.setPropertyType(doubleProperty);
        if( getProperty(&property) ) {
          property.getValue(rValue);
          return true;
        } else {
          return false;
        }
      } else {
        return false;
      }
    }

    bool CFGManager::getPropertyValue(cfgParamId paramId,
                                      const string &_propertyName,
                                      int *rValue) const {
      utils::MutexLocker locker(&mutexCFGParams);
      CFGParam *param = NULL;
      if( getParam(&param, paramId) ) {
        CFGProperty property;
        property.setParamId( paramId );
        property.setPropertyIndex( param->getPropertyIndexByName(_propertyName) );
        property.setPropertyType(intProperty);
        if( getProperty(&property) ) {
          property.getValue(rValue);
          return true;
        } else {
          return false;
        }
      } else {
        return false;
      }
    }

    bool CFGManager::getPropertyValue(cfgParamId paramId,
                                      const string &_propertyName,
                                      bool *rValue) const {
      utils::MutexLocker locker(&mutexCFGParams);
      CFGParam *param = NULL;
      if( getParam(&param, paramId) ) {
        CFGProperty property;
        property.setParamId( paramId );
        property.setPropertyIndex( param->getPropertyIndexByName(_propertyName) );
        property.setPropertyType(boolProperty);
        if( getProperty(&property) ) {
          property.getValue(rValue);
          return true;
        } else {
          return false;
        }
      } else {
        return false;
      }
    }


    bool CFGManager::getPropertyValue(cfgParamId paramId,
                                      const string &_propertyName,
                                      string *rValue) const {
      utils::MutexLocker locker(&mutexCFGParams);
      CFGParam *param = NULL;
      if( getParam(&param, paramId) ) {
        CFGProperty property;
        property.setParamId( paramId );
        property.setPropertyIndex( param->getPropertyIndexByName(_propertyName) );
        property.setPropertyType(stringProperty);
        if( getProperty(&property) ) {
          property.getValue(rValue);
          return true;
        } else {
          return false;
        }
      } else {
        return false;
      }
    }


    bool CFGManager::setPropertyValue(const string &_group,
                                      const string &_name,
                                      const string &_propertyName,
                                      const double rValue) {
      utils::MutexLocker locker(&mutexCFGParams);
      CFGParam *param = NULL;
      if( getParam(&param, _group, _name) ) {
        CFGProperty property;
        property.setParamId( param->getId() );
        property.setPropertyIndex( param->getPropertyIndexByName(_propertyName) );
        property.setPropertyType(doubleProperty);
        property.setValue(rValue);
        if( setProperty(property) ) {
          return true;
        } else {
          return false;
        }
      } else {
        return false;
      }
    }


    bool CFGManager::setPropertyValue(const string &_group,
                                      const string &_name,
                                      const string &_propertyName,
                                      const int rValue) {
      utils::MutexLocker locker(&mutexCFGParams);
      CFGParam *param = NULL;
      if( getParam(&param, _group, _name) ) {
        CFGProperty property;
        property.setParamId( param->getId() );
        property.setPropertyIndex( param->getPropertyIndexByName(_propertyName) );
        property.setPropertyType(intProperty);
        property.setValue(rValue);
        if( setProperty(property) ) {
          return true;
        } else {
          return false;
        }
      } else {
        return false;
      }
    }


    bool CFGManager::setPropertyValue(const string &_group,
                                      const string &_name,
                                      const string &_propertyName,
                                      const bool rValue) {
      utils::MutexLocker locker(&mutexCFGParams);
      CFGParam *param = NULL;
      if( getParam(&param, _group, _name) ) {
        CFGProperty property;
        property.setParamId( param->getId() );
        property.setPropertyIndex( param->getPropertyIndexByName(_propertyName) );
        property.setPropertyType(boolProperty);
        property.setValue(rValue);
        if( setProperty(property) ) {
          return true;
        } else {
          return false;
        }
      } else {
        return false;
      }
    }


    bool CFGManager::setPropertyValue(const string &_group,
                                      const string &_name,
                                      const string &_propertyName,
                                      const string &rValue) {
      utils::MutexLocker locker(&mutexCFGParams);
      CFGParam *param = NULL;
      if( getParam(&param, _group, _name) ) {
        CFGProperty property;
        property.setParamId( param->getId() );
        property.setPropertyIndex( param->getPropertyIndexByName(_propertyName) );
        property.setPropertyType(stringProperty);
        property.setValue(rValue);
        if( setProperty(property) ) {
          return true;
        } else {
          return false;
        }
      } else {
        return false;
      }
    }

    bool CFGManager::setPropertyValue(const string &_group,
                                      const string &_name,
                                      const string &_propertyName,
                                      const char *rValue) {
      return setPropertyValue(_group, _name, _propertyName,
                              std::string(rValue));
    }


    cfgParamId CFGManager::getParamId(const string &_group,
                                      const string &_name) const {
      utils::MutexLocker locker(&mutexCFGParams);
      CFGParam *param = NULL;
      if( getParam(&param, _group, _name) ) {
        return param->getId();
      } else {
        return 0;
      }
    }


    const cfgParamInfo CFGManager::getParamInfo(const cfgParamId &_id) const {
      utils::MutexLocker locker(&mutexCFGParams);
      CFGParam *param = NULL;
      getParam(&param, _id);
      return getParamInfo(param);
    }


    const cfgParamInfo CFGManager::getParamInfo(const string &_group,
                                                const string &_name) const {
      utils::MutexLocker locker(&mutexCFGParams);
      CFGParam *param = NULL;
      getParam(&param, _group, _name);
      return getParamInfo(param);
    }


    cfgParamId CFGManager::registerToParam(const string &_group,
                                           const string &_name,
                                           CFGClient *client) {
      utils::MutexLocker locker(&mutexCFGParams);
      CFGParam *param = NULL;
      if( getParam(&param, _group, _name) ) {
        param->addClient(client);
        return param->getId();
      } else {
        return 0;
      }
    }


    bool CFGManager::registerToParam(const cfgParamId &_id, CFGClient *client) {
      utils::MutexLocker locker(&mutexCFGParams);
      CFGParam *param = NULL;
      if ( getParam(&param, _id) ) {
        param->addClient(client);
        return true;
      } else {
        return false;
      }
    }


    bool CFGManager::unregisterFromParam(const string &_group,
                                         const string &_name,
                                         CFGClient *client) {
      utils::MutexLocker locker(&mutexCFGParams);
      CFGParam *param = NULL;
      if( getParam(&param, _group, _name) ) {
        param->removeClient(client);
        return true;
      } else {
        return false;
      }
    }


    bool CFGManager::unregisterFromParam(const cfgParamId &_id,
                                         CFGClient *client) {
      utils::MutexLocker locker(&mutexCFGParams);
      CFGParam *param = NULL;
      if( getParam(&param, _id) ) {
        param->removeClient(client);
        return true;
      } else {
        return false;
      }
    }


    bool CFGManager::registerToCFG(CFGClient *client) {
      mutexVecClients.lock();
      vecClients.push_back(client);
      mutexVecClients.unlock();
      return true;
    }


    bool CFGManager::unregisterFromCFG(CFGClient *client) {
      vector<CFGClient*>::iterator iter;
      mutexVecClients.lock();
      for(iter = vecClients.begin(); iter != vecClients.end(); ++iter) {
        if (*iter == client) {
          vecClients.erase(iter);
          break;
        } // if
      } // for
      mutexVecClients.unlock();
      return true;
    }


    // PRIVAT

    cfgParamId CFGManager::getNextId() {
      mutexNextId.lock();
      cfgParamId tmpNextId = nextId;
      ++nextId;
      mutexNextId.unlock();
      return tmpNextId;
    }


    void CFGManager::readGroup(const string &group,
                               const YAML::Node &paramNodes) {
      unsigned int i, j;
      string type = "";
      cfgParamId newId = 0;

      for(i = 0; i < paramNodes.size(); ++i) {
        paramNodes[i]["type"] >> type;
        //cout << "Found type: " << type << endl;

        for(j = 0; j < dstNrOfParamTypes; ++j) {
          if( cfgParamTypeString[j] == type ) {
            break;
          }
        } // for
        CFGParam *param = NULL;
        switch (j) {
        case doubleParam :
          newId = getNextId();
          param = CFGParamDouble::createParam(newId, group, paramNodes[i]);
          insertParam(param);
          break;
        case intParam :
          newId = getNextId();
          param = CFGParamInt::createParam(newId, group, paramNodes[i]);
          insertParam(param);
          break;
        case boolParam :
          newId = getNextId();
          param = CFGParamBool::createParam(newId, group, paramNodes[i]);
          insertParam(param);
          break;
        case stringParam :
          newId = getNextId();
          param = CFGParamString::createParam(newId, group, paramNodes[i]);
          insertParam(param);
          break;
        default:
          // do nothing
          cout << "CFG: unknown type: " << type;
        } // switch
        if(newId != 0) {
          addedCFGParam(newId);
          newId = 0;
        }
      } // for
    }


    void CFGManager::insertParam(CFGParam *newParam) {
      mutexCFGParams.lock();
      cfgParamId id = newParam->getId();
      mapStringToParam::const_iterator iter;
      string stringID = newParam->getGroup() + ":" + newParam->getName();
      iter = cfgParamsByString.find(stringID);

      // is there already this param?
      if( iter != cfgParamsByString.end() ) {
        // then get the values of the properties and override the old ones
        CFGParam* oldParam = iter->second;
        unsigned int i = 0;
        for(i = 0; i < oldParam->getNrOfPropertys(); ++i) {
          CFGProperty myProperty;
          myProperty.setParamId(newParam->getId());
          myProperty.setPropertyIndex(i);
          myProperty.setPropertyType(newParam->getPropertyTypeByIndex(i));
          if( newParam->getProperty(&myProperty) ) {
            if( myProperty.isValueSet() ) {
              myProperty.changeParamId(oldParam->getId());
              oldParam->setProperty(myProperty);
            }
          } //if
        } //for
        // delete the new param which is no more needed
        deleteParam(newParam);
      } else {
        string stringId = newParam->getGroup() + ":" + newParam->getName();
        cfgParamsById.insert(pair<cfgParamId, CFGParam*>(id, newParam));
        cfgParamsByString.insert(pair<string, CFGParam*>(stringId, newParam));
      }
      mutexCFGParams.unlock();
    }


    void CFGManager::deleteParam(CFGParam *param) {
      switch (param->getParamType()) {
      case doubleParam :
        delete (CFGParamDouble*)(param);
        break;
      case intParam :
        delete (CFGParamInt*)(param);
        break;
      case boolParam :
        delete (CFGParamBool*)(param);
        break;
      case stringParam :
        delete (CFGParamString*)(param);
        break;
      default:
        delete (param);
      } // switch
    }


    bool CFGManager::getParam(CFGParam **param,
                       const string &_group, const string &_name) const {
      utils::MutexLocker locker (&mutexCFGParams);
      mapStringToParam::const_iterator iter;
      string stringId = _group + ":" + _name;
      iter = cfgParamsByString.find(stringId);
      if( iter != cfgParamsByString.end() ) {
        *param = iter->second;
        //cout << "set param by string!" << endl;
        return true;
      } else {
        return false;
      }
    }


    bool CFGManager::getParam(CFGParam **param, const cfgParamId &_id) const {
      utils::MutexLocker locker (&mutexCFGParams);
      mapIdToParam::const_iterator iter;
      iter = cfgParamsById.find(_id);
      if( iter != cfgParamsById.end() ) {
        *param = iter->second;
        //cout << "set param by id!" << endl;
        return true;
      } else {
        return false;
      }
    }


    const cfgParamInfo CFGManager::getParamInfo(const CFGParam *param) const {
      utils::MutexLocker locker(&mutexCFGParams);
      cfgParamInfo paramInfo;
      if( param != NULL ) {
        paramInfo.id    = param->getId();
        paramInfo.group = param->getGroup();
        paramInfo.name  = param->getName();
        paramInfo.type  = param->getParamType();
      } else {
        paramInfo.id    = 0;
        paramInfo.group = "";
        paramInfo.name  = "";
        paramInfo.type  = noParam;
      }
      return paramInfo;
    }


    void CFGManager::addedCFGParam(const cfgParamId &_id) const {
      vector<CFGClient*>::const_iterator iter;
      mutexVecClients.lock();
      for(iter = vecClients.begin(); iter != vecClients.end(); ++iter) {
        (*iter)->cfgParamCreated(_id);
      }
      mutexVecClients.unlock();
    }


    void CFGManager::removedCFGParam(const cfgParamId &_id) const {
      vector<CFGClient*>::const_iterator iter;
      mutexVecClients.lock();
      for(iter = vecClients.begin(); iter != vecClients.end(); ++iter) {
        (*iter)->cfgParamRemoved(_id);
      }
      mutexVecClients.unlock();
    }


    bool CFGManager::fileExists(const string &strFilename) const {
      struct stat stFileInfo;
      bool blnReturn;
      int intStat;

      // Attempt to get the file attributes
      intStat = stat(strFilename.c_str(), &stFileInfo);
      if(intStat == 0) {
        // We were able to get the file attributes
        // so the file obviously exists.
        blnReturn = true;
      } else {
        // We were not able to get the file attributes.
        // This may mean that we don't have permission to
        // access the folder which contains this file. If you
        // need to do that level of checking, lookup the
        // return values of stat which will give you
        // more details on why stat failed.
        blnReturn = false;
      }

      return(blnReturn);
    }


    const cfgPropertyStruct CFGManager::getOrCreateProperty(const string &_group,
                                                            const string &_name,
                                                            bool val,
                                                            CFGClient *newClient) {
      cfgPropertyStruct newProp;
      newProp.propertyType = boolProperty;
      newProp.propertyIndex = 0;
      newProp.bValue = val;
      if(getPropertyValue(_group, _name, "value", &newProp.bValue)) {
        newProp.paramId = getParamId(_group, _name);
      } else {
        newProp.paramId = createParam(_group, _name, boolParam);
        setProperty(newProp);
      }
      if(newClient) {
        registerToParam(newProp.paramId, newClient);
      }
      return newProp;
    }


    const cfgPropertyStruct CFGManager::getOrCreateProperty(const string &_group,
                                                            const string &_name,
                                                            double val,
                                                            CFGClient *newClient) {
      cfgPropertyStruct newProp;
      newProp.propertyType = doubleProperty;
      newProp.propertyIndex = 0;
      newProp.dValue = val;
      if(getPropertyValue(_group, _name, "value", &newProp.dValue)) {
        newProp.paramId = getParamId(_group, _name);
      } else {
        newProp.paramId = createParam(_group, _name, doubleParam);
        setProperty(newProp);
      }
      if(newClient) {
        registerToParam(newProp.paramId, newClient);
      }
      return newProp;
    }


    const cfgPropertyStruct CFGManager::getOrCreateProperty(const string &_group,
                                                            const string &_name,
                                                            int val,
                                                            CFGClient *newClient) {
      cfgPropertyStruct newProp;
      newProp.propertyType = intProperty;
      newProp.propertyIndex = 0;
      newProp.iValue = val;
      if(getPropertyValue(_group, _name, "value", &newProp.iValue)) {
        newProp.paramId = getParamId(_group, _name);
      } else {
        newProp.paramId = createParam(_group, _name, intParam);
        setProperty(newProp);
      }
      if(newClient) {
        registerToParam(newProp.paramId, newClient);
      }
      return newProp;
    }


    const cfgPropertyStruct CFGManager::getOrCreateProperty(const string &_group,
                                                            const string &_name,
                                                            const string &val,
                                                            CFGClient *newClient) {
      cfgPropertyStruct newProp;
      newProp.propertyType = stringProperty;
      newProp.propertyIndex = 0;
      newProp.sValue = val;
      if(getPropertyValue(_group, _name, "value", &newProp.sValue)) {
        newProp.paramId = getParamId(_group, _name);
      } else {
        newProp.paramId = createParam(_group, _name, stringParam);
        setProperty(newProp);
      }
      if(newClient) {
        registerToParam(newProp.paramId, newClient);
      }
      return newProp;
    }

    const cfgPropertyStruct CFGManager::getOrCreateProperty(const string &_group,
                                                            const string &_name,
                                                            const char *val,
                                                            CFGClient *newClient) {
      return getOrCreateProperty(_group, _name, std::string(val), newClient);
    }

  } // end of namespace cfg_manager
} // end of namespace mars


DESTROY_LIB(mars::cfg_manager::CFGManager);
CREATE_LIB(mars::cfg_manager::CFGManager);
