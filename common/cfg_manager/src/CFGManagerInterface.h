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
 * \file CFGManagerInterface.h
 * \author Michael Rohn
 * \brief 'CFGManager' is a class to handle the config file and to update values stored in MARS
 *
 * Version 0.1
 */

#ifndef CFG_MANAGER_INTERFACE_H
#define CFG_MANAGER_INTERFACE_H

#ifdef _PRINT_HEADER_
#warning "CFGManagerInterface.h"
#endif

#include "CFGDefs.h"
#include "CFGClient.h"

#include <lib_manager/LibManager.hpp>

#include <string>
#include <vector>

namespace mars {
  namespace cfg_manager {

    class CFGManagerInterface : public lib_manager::LibInterface {

    public:
      CFGManagerInterface(lib_manager::LibManager *theManager) :
        lib_manager::LibInterface(theManager) {}
      ~CFGManagerInterface() {}

      // LibInterface methods
      virtual int getLibVersion() const = 0;
      virtual const std::string getLibName() const = 0;
      virtual void createModuleInfo() {}

      // CFGInterface methods
      virtual cfgParamId createParam(const std::string &_group,
                                     const std::string &_name,
                                     const cfgParamType &_paramType) = 0;

      virtual bool loadConfig(const char *filename) = 0;
      virtual bool loadConfig(const char *filename, const char *group) = 0;

      virtual bool loadConfigFromString(const std::string &configString) = 0;

      virtual void writeConfig(const char *filename, const char *group,
                               const unsigned char saveOption = saveOnClose) const = 0;
      virtual const std::string writeConfigToString(const char *group = 0,
                                                    const unsigned char saveOption = saveOnClose) const = 0;

      virtual bool removeParam(const cfgParamId &_id) = 0;
      virtual bool removeParam(const std::string &_group,
                               const std::string &_name) = 0;

      //virtual bool setProperty(const CFGProperty _property) = 0;
      //virtual bool getProperty(CFGProperty &_property) const = 0;

      virtual bool setProperty(const cfgPropertyStruct &_propertyS) = 0;
      virtual bool getProperty(cfgPropertyStruct *_propertyS) const = 0;

      template <typename T>
      bool getPropertyValue(const std::string &_group, const std::string &_name,
                            const std::string &_propertyName, T *rValue) const {
        return getPropertyValue(getParamId(_group, _name), _propertyName, rValue);
      }
      
      virtual bool getPropertyValue(cfgParamId paramId,
                                    const std::string &_propertyName,
                                    double *rValue) const = 0;
      virtual bool getPropertyValue(cfgParamId paramId,
                                    const std::string &_propertyName,
                                    int *rValue) const = 0;
      virtual bool getPropertyValue(cfgParamId paramId,
                                    const std::string &_propertyName,
                                    bool *rValue) const = 0;
      virtual bool getPropertyValue(cfgParamId paramId,
                                    const std::string &_propertyName,
                                    std::string *rValue) const = 0;

      virtual bool setPropertyValue(const std::string &_group,
                                    const std::string &_name,
                                    const std::string &_propertyName,
                                    const double rValue) = 0;
      virtual bool setPropertyValue(const std::string &_group,
                                    const std::string &_name,
                                    const std::string &_propertyName,
                                    const int rValue) = 0;
      virtual bool setPropertyValue(const std::string &_group,
                                    const std::string &_name,
                                    const std::string &_propertyName,
                                    const bool rValue) = 0;
      virtual bool setPropertyValue(const std::string &_group,
                                    const std::string &_name,
                                    const std::string &_propertyName,
                                    const std::string &rValue) = 0;
      virtual bool setPropertyValue(const std::string &_group,
                                    const std::string &_name,
                                    const std::string &_propertyName,
                                    const char *rValue) = 0;

      virtual cfgParamId getParamId(const std::string &_group,
                                    const std::string &_name) const = 0;

      virtual const cfgParamInfo getParamInfo(const cfgParamId &_id) const = 0;
      virtual const cfgParamInfo getParamInfo(const std::string &_group,
                                              const std::string &_name) const = 0;

      virtual cfgParamId registerToParam(const std::string &_group,
                                         const std::string &_name,
                                         CFGClient *client) = 0;
      virtual bool registerToParam(const cfgParamId &_id, CFGClient *client) = 0;

      virtual bool unregisterFromParam(const std::string &_group,
                                       const std::string &_name,
                                       CFGClient *client) = 0;
      virtual bool unregisterFromParam(const cfgParamId &_id,
                                       CFGClient *client) = 0;

      virtual bool registerToCFG(CFGClient *client) = 0;
      virtual bool unregisterFromCFG(CFGClient *client) = 0;

      virtual bool getAllParams(std::vector<cfgParamInfo> *allParams) const = 0;

      // specific methods that make life easier
      virtual const cfgPropertyStruct getOrCreateProperty(const std::string &_group,
                                                          const std::string &_name,
                                                          bool val,
                                                          CFGClient *newClient = 0) = 0;
      virtual const cfgPropertyStruct getOrCreateProperty(const std::string &_group,
                                                          const std::string &_name,
                                                          double val,
                                                          CFGClient *newClient = 0) = 0;
      virtual const cfgPropertyStruct getOrCreateProperty(const std::string &_group,
                                                          const std::string &_name,
                                                          int val,
                                                          CFGClient *newClient = 0) = 0;
      virtual const cfgPropertyStruct getOrCreateProperty(const std::string &_group,
                                                          const std::string &_name,
                                                          const std::string &val,
                                                          CFGClient *newClient = 0) = 0;
      virtual const cfgPropertyStruct getOrCreateProperty(const std::string &_group,
                                                          const std::string &_name,
                                                          const char *val,
                                                          CFGClient *newClient = 0) = 0;

      virtual void setProperty(const std::string &_group,
			       const std::string &_name, bool val) = 0;
      virtual void setProperty(const std::string &_group,
			       const std::string &_name, double val) = 0;
      virtual void setProperty(const std::string &_group,
			       const std::string &_name, int val) = 0;
      virtual void setProperty(const std::string &_group,
			       const std::string &_name,
			       const std::string &val) = 0;
      virtual void setProperty(const std::string &_group,
			       const std::string &_name, const char *val) = 0;

    }; // end class CFGManagerInterface

  } // end namespace cfg_manager
} // end namespace mars

#endif /* CFG_MANAGER_INTERFACE_H */
