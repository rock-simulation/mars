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
  #include <dlfcn.h>
#else
  #include <windows.h>
#endif

#include <iostream>
#include <cstdio>

namespace mars {
  namespace lib_manager {

    using namespace std;

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

      //fprintf(stderr, "\nDelete LibManager %d\n", (int)libMap.size());

      while(!finished) {
        finished = true;
        for(it=libMap.begin(); it!=libMap.end(); ++it) {
          if(it->second.useCount == 0) {
            fprintf(stderr, "\nLibManager: delete [%s] !", it->first.c_str() );
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
        for(it=libMap.begin(); it!=libMap.end(); ++it) {
          fprintf(stderr, "LibManager: [%s] not deleted correctly! "
                  "%d references remain.\n",
                  it->first.c_str(), it->second.useCount);
        }
      } else {
        fprintf(stderr, "LibManager: successfully deleted all libraries!");
      }
      fprintf(stderr, "\nDelete lib_manager\n");
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
      if(libMap.find(name) == libMap.end()) {
        libMap[name] = newLib;
      }
    }

    LibManager::ErrorNumber LibManager::loadLibrary(const string &libPath, 
                                                    void *config) {
      libStruct newLib;
      string name;

      newLib.destroy = 0;
      newLib.libInterface = 0;
      newLib.useCount = 0;

#ifdef WIN32
      HINSTANCE pl;
      size_t needed = ::mbstowcs(NULL, libPath.c_str(), libPath.length());

      // allocate
      std::wstring output;
      output.resize(needed);

      // real call
      ::mbstowcs(&output[0], libPath.c_str(), libPath.length());

      // You asked for a pointer
      //const wchar_t *pout = output.c_str();
      cout << "lib_manager: load library: " << libPath << endl;
      pl = LoadLibrary(libPath.c_str());

      if(!pl) {
        cout << "lib_manager: can not load library. Error code: " 
             << GetLastError() << endl;
      } else {
        //dlerror();
        createLib* tmp_con = (createLib*)GetProcAddress(pl, "create_c");
        newLib.destroy = (destroyLib*)GetProcAddress(pl, "destroy_c");
        if(!newLib.destroy) {
          cout << "lib_manager: can not load library symbol" << endl;
        } else {
          newLib.libInterface = tmp_con(this);
        }
      }
#else
      cout << "lib_manager: load plugin: " << libPath << endl;
      void *pl = dlopen(libPath.c_str(), RTLD_LAZY);
      if(!pl) {
        cout << "lib_manager: can not load library. Error code: " 
             << dlerror() << endl;
      } else {
        dlerror();
        createLib *tmp_con;
        createLib2 *tmp_con2;

        if(config != NULL) {
          tmp_con2 = (createLib2*)dlsym(pl, "config_create_c");
        } else {
          tmp_con = (createLib*)dlsym(pl, "create_c");
        }

        newLib.destroy = (destroyLib*)dlsym(pl, "destroy_c");
        if(dlerror()) {
          cout << "lib_manager: can not load library symbol" << endl;
        } else {
          if(config != NULL) {
            newLib.libInterface = tmp_con2(this, config);
          } else{
            newLib.libInterface = tmp_con(this);
          }
        }
      }
#endif
      if(newLib.libInterface) {
        name = newLib.libInterface->getLibName();
        if(libMap.find(name) == libMap.end()) {
          libMap[name] = newLib;
          return LIBMGR_NO_ERROR;
        } else newLib.destroy(newLib.libInterface);
        return LIBMGR_ERR_LIBNAME_EXISTS;
      }
      return LIBMGR_ERR_NOT_ABLE_TO_LOAD;
    }

    LibInterface* LibManager::getLibrary(const string &libName) {
      if(libMap.find(libName) == libMap.end()) {
        fprintf(stderr, "LibManager: could not find \"%s\"", libName.c_str());
        return 0;
      }

      libStruct *theLib = &(libMap[libName]);
      theLib->useCount++;
      return theLib->libInterface;
    }

    LibManager::ErrorNumber LibManager::unloadLibrary(const string &libName) {
      if(libMap.find(libName) == libMap.end()) {
        return LIBMGR_ERR_NO_LIBRARY;
      }

      libStruct *theLib = &(libMap[libName]);
      theLib->useCount--;
      if(theLib->useCount <= 0) {
        fprintf(stderr, "\nLibManager: unload delete [%s]", libName.c_str());
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

  } // end of namespace lib_manager
} // end of namespace mars
