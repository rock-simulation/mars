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

/*
 *  utils.cpp
 *  Simulator
 *
 *  Created by Malte Roemmermann
 *
 */

#include "misc.h"

#include <cstdio>
#include <cmath>
#include <sstream>
#include <iostream>
#include <cassert>

#ifdef WIN32
#include <dirent.h>
#define MODUS
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#define MODUS ,0711
#endif

namespace mars {
  namespace utils {

    using namespace std;

#if 0
    void getAbsFromRel(const nodeStruct &node1, nodeStruct* node2){
      Vector t, t2, source;
      Quaternion q;
      q.setIdentity();
      
      t = node1.rot * node2->relative_pos;
      node2->pos = node1.pos + t;
      node2->rot = node1.rot * node2->relative_rot;
    }

    /* keep this in sync with the JointTypes struct. */
    static const char* sJointTypeStrings[NUMBER_OF_JOINT_TYPES] = {
      "undefined",
      "hinge",
      "hinge2",
      "slider",
      "ball",
      "universal",
      "fixed",
      "istruct-spine"
    };
    
    const char* getJointTypeString(JointType type)
    {
      if (type >= NUMBER_OF_JOINT_TYPES)
        {
          std::cerr << "getJointTypeString(JointType id): invalid joint type id " << (int)type << std::endl;
          //throw exception here?
          
          return NULL;
        }
      return sJointTypeStrings[type];
    }
    
    JointType getJointType(const std::string& text)
    {
      assert(sizeof(sJointTypeStrings) / sizeof(char*) == NUMBER_OF_JOINT_TYPES);
      
      //keep this in sync with the correct ids (from mars_core/src/MARSDefs.h)
      for (int i = 0; i < NUMBER_OF_JOINT_TYPES; ++i)
        {
          if (text == sJointTypeStrings[i])
            {
              return (JointType)i;
            }
        }
      
      //this type string might also be the type-id encoded in a decimal string.
      std::istringstream iss(text);
      int numberFromString;
      if ( !(iss >> numberFromString).fail() )
        {
          //conversion to integer was ok but is this a valid type id?
          if (numberFromString > 0 && numberFromString < NUMBER_OF_JOINT_TYPES)
            {
              //yes, it is.
              return (JointType)numberFromString;
            }
        }
      
      //string not found and conversion to integer not successful.
      std::cerr << __FILE__": Could not get joint type from string \""
                << text << "\"." << std::endl;
      return JOINT_TYPE_UNDEFINED;
    }
#endif

    void handleFilenamePrefix(std::string *file, const std::string &prefix) {
      std::string tmp = prefix;
      size_t pos;

      if(file->empty()) return;

      tmp.append("/");
      if((pos = file->rfind('/')) != std::string::npos) {
        *file= file->substr(pos+1);
      }
      tmp.append(*file);
      *file = tmp;
    }

    void removeFilenamePrefix(std::string *file) {
      size_t pos;

      if((pos = file->rfind('/')) != std::string::npos) {
        *file= file->substr(pos+1);
      }
    }

    void removeFilenameSuffix(std::string *file) {
      size_t pos;

      if((pos = file->rfind('.')) != std::string::npos) {
        *file= file->substr(0, pos+1);
      }
    }

    std::string getFilenameSuffix(const std::string &file) {
      size_t pos;

      if((pos = file.rfind('.')) != std::string::npos) {
        return file.substr(pos+1);
      }
      return "";
    }

    std::string getCurrentWorkingDir() {
      char buffer[512];
      getcwd(buffer, 512);
      std::string currentDirectory(buffer);
      if(buffer == NULL) {
        std::cerr << "misc:: error while getting current working dir"
                  << std::endl;
        return "";
      }
      return currentDirectory;
    }

    std::string getPathOfFile(const std::string &filename) {
      std::string path = ".";
      size_t pos;

      if((pos = filename.rfind('/')) != std::string::npos) {
        path = filename.substr(0, pos+1);
      }
      return path;
    }

    unsigned int createDirectory(const std::string &dir) {
      std::string s_tmpString = dir;
      DIR *p_Directory;

      p_Directory= opendir(dir.c_str());
      if (p_Directory!=NULL) {
        closedir(p_Directory);
        return 1;
      }

      int result=mkdir(dir.c_str()MODUS );
      if(result==-1) {
        fprintf(stderr, "misc:: could not create dir: %s\n", dir.c_str());
        return 0;
      }
      return 1;
    }
    
  } // end of namespace utils

} // end of namespace mars
