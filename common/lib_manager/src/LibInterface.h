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

/* We had to jump through some hoops to get the git version information
 * into the libInterface:
 * 1) If we set it directly in the constructor the linker seems to mess things
 *    up (has only a single code section of the ctor?).
 * 2) So we set the info in the create_c.
 * 3) create_c must therefore be a friend function of the LibInterface.
 * 4) create_c must therefore be forward declared.
 * 5) The LibInterface and LibManager must therefore be forward declared.
 * NOTE: because of step 2) the git info only becomes available after the
 *       classes constructor!
 */

namespace mars {
  namespace lib_manager {
    class LibInterface;
    class LibManager;
  }
}

extern "C" mars::lib_manager::LibInterface* create_c(mars::lib_manager::LibManager *theManager);

#ifdef GIT_INFO
#define EXPAND_STRING(x) #x
#define MACRO_TO_STRING(x) EXPAND_STRING(x)
#define GIT_INFO_STR MACRO_TO_STRING(GIT_INFO);
#else
#define GIT_INFO_STR "<no git info>";
#endif

#define DESTROY_LIB(theClass)                                           \
  extern "C" void *destroy_c(mars::lib_manager::LibInterface *sp) {     \
    delete (dynamic_cast<theClass*>(sp));                               \
    return 0;                                                           \
  }

#define CREATE_LIB(theClass)                                            \
  extern "C" mars::lib_manager::LibInterface* create_c(mars::lib_manager::LibManager *theManager) { \
    theClass *instance = new theClass(theManager);                      \
    instance->gitInfo = GIT_INFO_STR;                                   \
    return dynamic_cast<mars::lib_manager::LibInterface*>(instance);    \
  }

#define CREATE_LIB_CONFIG(theClass, configType)                         \
  extern "C" mars::lib_manager::LibInterface* config_create_c(mars::lib_manager::LibManager *theManager, configType *config) { \
    configType *_config = dynamic_cast<configType*>(config);            \
    if(_config == NULL) return 0;                                       \
    theClass *instance = new theClass(theManager, _config);             \
    instance->gitInfo = GIT_INFO_STR;                                   \
    return dynamic_cast<mars::lib_manager::LibInterface*>(instance);    \
  }

namespace mars {
  namespace lib_manager {

    /**
     * The interface to load libraries dynamically
     *
     */
    class LibInterface {
    public:
      LibInterface(LibManager *theManager)
        : libManager(theManager),
          gitInfo("<git info becomes available after ctor>")
      {}
      virtual ~LibInterface(void) {}
      virtual int getLibVersion() const = 0;
      virtual const std::string getLibName() const = 0;
      const char* getGitInfo() const
      { return gitInfo; }
      virtual void newLibLoaded(const std::string &libName) {}

    protected:
      LibManager *libManager;
      const char *gitInfo;
      friend mars::lib_manager::LibInterface* ::create_c(mars::lib_manager::LibManager *theManager);
      friend mars::lib_manager::LibInterface* ::config_create_c(mars::lib_manager::LibManager *theManager, configType *config);
    };

    typedef void *destroyLib(LibInterface *sp);
    typedef LibInterface* createLib(LibManager *theManager);
    typedef LibInterface* createLib2(LibManager *theManager, void *configuration);

  } // end of namespace lib_manager

} // end of namespace mars

#endif  /* LIB_INTERFACE_H */
