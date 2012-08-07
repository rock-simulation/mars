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
 * \file LibInterface.h
 * \author Malte Roemmermann
 * \brief "LibInterface" is an interface to load dynamically libraries
 *
 */

#ifndef LIB_INTERFACE_H
#define LIB_INTERFACE_H

#ifdef _PRINT_HEADER_
  #warning "LibInterface.h"
#endif

#include <string>

#define DESTROY_LIB(theClass)                                   \
  extern "C" void *destroy_c(mars::lib_manager::LibInterface *sp) {     \
    delete (dynamic_cast<theClass*>(sp));                       \
    return 0;              \
  }

#define CREATE_LIB(theClass)                                            \
  extern "C" mars::lib_manager::LibInterface* create_c(mars::lib_manager::LibManager *theManager) { \
    theClass *instance = new theClass(theManager);                     \
    return dynamic_cast<mars::lib_manager::LibInterface*>(instance);   \
  }

#define CREATE_LIB_CONFIG(theClass,configType)                        \
  extern "C" mars::lib_manager::LibInterface* config_create_c(mars::lib_manager::LibManager *theManager, configType *config) { \
   configType *_config = dynamic_cast<configType*>(config); \
   if(_config == NULL) return 0;                                       \
   theClass *instance = new theClass(theManager, _config);            \
   return dynamic_cast<mars::lib_manager::LibInterface*>(instance);         \
 }



namespace mars {
  namespace lib_manager {

  class LibManager;

  /**
   * The interface to load libraries dynamically
   *
   */
  class LibInterface {
    public:
      LibInterface(LibManager *theManager) : libManager(theManager) {}
      virtual ~LibInterface(void) {}
      virtual int getLibVersion() const = 0;
      virtual const std::string getLibName() const = 0;

    protected:
      LibManager *libManager;
  };

  typedef void *destroyLib(LibInterface *sp);
  typedef LibInterface* createLib(LibManager *theManager);
  typedef LibInterface* createLib2(LibManager *theManager, void *configuration);

  } // end of namespace lib_manager

} // end of namespace mars

#endif  /* LIB_INTERFACE_H */
