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
 * \file LibInterface.cpp
 * \author Malte Roemmermann
 * \brief "LibInterface" is an interface to load dynamically libraries
 *
 */

#include "LibManager.h"

#ifndef WIN32
#  include <dlfcn.h>
#  define LibHandle void*
#else
#  include <windows.h>
#  define LibHandle HINSTANCE
#endif

#include <cstdio>

namespace mars {
  namespace lib_manager {

    using namespace std;

    // forward declarations
    static LibHandle intern_loadLib(const string &libPath);
    template <typename T>
    static T getFunc(LibHandle libHandle, const string &name);


    LibManager::LibManager() {
      errMessage[0] = "no error";
      errMessage[1] = "no library with given name loaded";
      errMessage[2] = "library name already exists";
      errMessage[3] = "not able to load library";  
      errMessage[4] = "library is still in use";  
    }

    LibManager::~LibManager() {
      std::map<std::string, libStruct>::iterator it;
      bool finished = false;

      while(!finished) {
        finished = true;
        for(it = libMap.begin(); it != libMap.end(); ++it) {
          if(it->second.useCount == 0) {
            fprintf(stderr, "LibManager: delete [%s] !\n", it->first.c_str() );
            if(it->second.destroy) {
              it->second.destroy(it->second.libInterface);
            }
            libMap.erase(it);
            finished = false;
            break;
          }
        }
      }
      if(libMap.size() > 0) {
        for(it = libMap.begin(); it != libMap.end(); ++it) {
          fprintf(stderr, "LibManager: [%s] not deleted correctly! "
                  "%d references remain.\n"
                  "      NOTE: The semantics of the LibManager has changed. To correctly\n"
                  "            dispose of a library acquired by a call to getLibrary(libName)\n"
                  "            you should now call releaseLibrary(libName) instead\n"
                  "            of unloadLibrary(libName).\n",
                  it->first.c_str(), it->second.useCount);
        }
      } else {
        fprintf(stderr, "LibManager: successfully deleted all libraries!\n");
      }
      fprintf(stderr, "Delete lib_manager\n");
    }

    void LibManager::addLibrary(LibInterface *_lib) {
      if(!_lib) {
        return;
      }
      libStruct newLib;
      string name = _lib->getLibName();

      newLib.destroy = 0;
      newLib.libInterface = _lib;
      newLib.useCount = 1;
      newLib.wasUnloaded = false;
      if(libMap.find(name) == libMap.end()) {
        libMap[name] = newLib;
      }
    }

    LibManager::ErrorNumber LibManager::loadLibrary(const string &libPath, 
                                                    void *config) {
      fprintf(stderr, "lib_manager: load plugin: %s\n", libPath.c_str());

      libStruct newLib;
      newLib.destroy = 0;
      newLib.libInterface = 0;
      newLib.useCount = 0;
      newLib.wasUnloaded = false;

      LibHandle pl = intern_loadLib(libPath);

      if(pl) {
        newLib.destroy = getFunc<destroyLib*>(pl, "destroy_c");
        if(newLib.destroy) {
          if(!config) {
            createLib *tmp_con = getFunc<createLib*>(pl, "create_c");
            if(tmp_con)
              newLib.libInterface = tmp_con(this);
          } else {
            createLib2 *tmp_con2 = getFunc<createLib2*>(pl, "config_create_c");
            if(tmp_con2)
              newLib.libInterface = tmp_con2(this, config);
          }
        }
      }

      if(!newLib.libInterface)
        return LIBMGR_ERR_NOT_ABLE_TO_LOAD;

      string name = newLib.libInterface->getLibName();

      if(libMap.find(name) != libMap.end()) {
        newLib.destroy(newLib.libInterface);
        return LIBMGR_ERR_LIBNAME_EXISTS;
      }

      libMap[name] = newLib;
      // notify all Libs of newly loaded lib
      for(map<string, libStruct>::iterator it = libMap.begin();
          it != libMap.end(); ++it) {
        // not notify the new lib about itself
        if(it->first != name)
          it->second.libInterface->newLibLoaded(name);
      }
      return LIBMGR_NO_ERROR;
    }

    LibInterface* LibManager::acquireLibrary(const string &libName) {
      if(libMap.find(libName) == libMap.end()) {
        fprintf(stderr, "LibManager: could not find \"%s\"\n", libName.c_str());
        return 0;
      }

      libStruct *theLib = &(libMap[libName]);
      theLib->useCount++;
      return theLib->libInterface;
    }

    LibManager::ErrorNumber LibManager::releaseLibrary(const string &libName) {
      if(libMap.find(libName) == libMap.end()) {
        return LIBMGR_ERR_NO_LIBRARY;
      }
      libStruct *theLib = &(libMap[libName]);
      theLib->useCount--;
      if(theLib->useCount <= 0 && theLib->wasUnloaded) {
        unloadLibrary(libName);
      }
      return LIBMGR_NO_ERROR;
    }

    LibManager::ErrorNumber LibManager::unloadLibrary(const string &libName) {
      if(libMap.find(libName) == libMap.end()) {
        return LIBMGR_ERR_NO_LIBRARY;
      }

      libStruct *theLib = &(libMap[libName]);
      theLib->wasUnloaded = true;
      if(theLib->useCount <= 0) {
        fprintf(stderr, "LibManager: unload delete [%s]\n", libName.c_str());
        if(theLib->destroy) {
          theLib->destroy(theLib->libInterface);
        }
        libMap.erase(libName);
        return LIBMGR_NO_ERROR;
      }
      return LIBMGR_ERR_LIB_IN_USE;
    }

    void LibManager::loadConfigFile(const std::string &config_file) {
      char plugin_chars[255];
      std::string plugin_path;
      FILE *plugin_config;

      plugin_config = fopen(config_file.c_str() , "r");

      if(plugin_config) {
        while(fgets(plugin_chars, 255, plugin_config)) {
          plugin_path = plugin_chars;
          // strip whitespaces from start and end of line
          size_t pos1 = plugin_path.find_first_not_of(" \t\n\r");
          size_t pos2 = plugin_path.find_last_not_of(" \t\n\r");
          if(pos1 == string::npos || pos2 == string::npos) {
            continue;
          }
          plugin_path = plugin_path.substr(pos1, pos2 - pos1 + 1);
          // ignore lines that start with #
          if(plugin_path[0] != '#') {
            loadLibrary(plugin_path);
          }
        }
        fclose(plugin_config);
      }
    }


    void LibManager::getAllLibraries(std::list<LibInterface*> *libList) {
      std::map<std::string, libStruct>::iterator it;
      for(it = libMap.begin(); it != libMap.end(); ++it) {
        it->second.useCount++;
        libList->push_back(it->second.libInterface);
      }
    }
    
    void LibManager::getAllLibraryNames(std::list<std::string> *libNameList) const {
      std::map<std::string, libStruct>::const_iterator it;
      for(it = libMap.begin(); it != libMap.end(); ++it) {
        libNameList->push_back(it->first);
      }
    }


    ////////////////////
    // Helper Functions
    ////////////////////

    static LibHandle intern_loadLib(const std::string &libPath) {
      LibHandle libHandle;
      string errorMsg;
#ifdef WIN32
      libHandle = LoadLibrary(libPath.c_str());
      if(!libHandle) {
        // retrive the error message
        {
          LPTSTR lpErrorText = NULL;
          ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
                          FORMAT_MESSAGE_ALLOCATE_BUFFER,
                          0, GetLastError(), 0,
                          (LPTSTR)&lpErrorText, MAX_PATH, 0);
          errorMsg = lpErrorText;
          ::LocalFree(lpErrorText);
        }
      }
#else
      libHandle = dlopen(libPath.c_str(), RTLD_LAZY);
      if(!libHandle) {
        errorMsg = dlerror();
      }
#endif
      if(!libHandle) {
        fprintf(stderr, "ERROR: lib_manager cannot load library:\n       %s\n",
                errorMsg.c_str());
      }
      return libHandle;
    }

    template <typename T>
    static T getFunc(LibHandle libHandle, const std::string &name) {
      T func = NULL;
      string err;
#ifdef WIN32
      func = reinterpret_cast<T>(GetProcAddress(libHandle, name.c_str()));
#else
      func = reinterpret_cast<T>(dlsym(libHandle, name.c_str()));
      if(!func)
        err = dlerror();
#endif
      if(!func) {
        fprintf(stderr, 
                "ERROR: lib_manager cannot load library symbol \"%s\"\n"
                "       %s\n", name.c_str(), err.c_str());
      }
      return func;
    }

  } // end of namespace lib_manager
} // end of namespace mars
