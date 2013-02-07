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
#include <vector>
#include <sstream>
#include <iostream>
#include <cassert>

#ifdef WIN32
#include <dirent.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#endif

namespace mars {
  namespace utils {

    using namespace std;

    bool matchPattern(const std::string &pattern, const std::string &str) {
      size_t pos1 = 0, pos2 = 0;

      // chop up pattern
      std::vector<std::string> patternList;
      while(1) {
        pos2 = pattern.find("*", pos1);
        if(pos2 == pattern.npos) {
          if(pos1 != pattern.length())
            patternList.push_back(pattern.substr(pos1));
          break;
        }
        patternList.push_back(pattern.substr(pos1, pos2 - pos1));
        pos1 = pos2 + 1;
      }

      // no wildcards. do direct test
      if(patternList.empty())
        return pattern == str;

      // special case the first pattern because it must match at pos == 0
      std::vector<std::string>::iterator patternListIt = patternList.begin();
      int result = str.compare(0, patternListIt->length(), *patternListIt);
      if(result != 0)
        return false;
      pos1 = patternListIt->length();
      ++patternListIt;
      // do the matching
      for(/*nothing*/; patternListIt != patternList.end(); ++patternListIt) {
        pos1 = str.find(*patternListIt, pos1);
        if(pos1 == str.npos)
          return false;
        pos1 += patternListIt->length();
      }
      return true;
    }

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
        *file= file->substr(0, pos);
      }
    }

    std::string getFilenameSuffix(const std::string &file) {
      size_t pos;

      if((pos = file.rfind('.')) != std::string::npos) {
        return file.substr(pos);
      }
      return "";
    }

    std::string getCurrentWorkingDir() {
      const int BUFFER_SIZE = 512;
      char buffer[BUFFER_SIZE];
      if(!getcwd(buffer, BUFFER_SIZE)) {
        std::cerr << "misc:: error while getting current working dir"
                  << std::endl;
        return "";
      }
      return std::string(buffer);
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

      p_Directory = opendir(dir.c_str());
      if(p_Directory != NULL) {
        closedir(p_Directory);
        return 1;
      }

#ifdef WIN32
      int result = mkdir(dir.c_str());
#else
      int result = mkdir(dir.c_str(), 0755);
#endif
      if(result == -1) {
        fprintf(stderr, "misc:: could not create dir: %s\n", dir.c_str());
        return 0;
      }
      return 1;
    }
    
  } // end of namespace utils

} // end of namespace mars
