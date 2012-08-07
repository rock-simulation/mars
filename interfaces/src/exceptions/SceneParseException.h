/*
 *  Copyright 2012, DFKI GmbH Robotics Innovation Center
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

#ifndef MARS_BASE_SCENE_PARSE_EXCEPTION
#define MARS_BASE_SCENE_PARSE_EXCEPTION

#include <exception>
#include <string>
#include <sstream>

namespace mars {
  namespace interfaces {

    class SceneParseException : public std::exception {
    public:
      SceneParseException(const std::string &info, int line=-1,
                          const std::string &file="") throw()
        : line(line), file(file) {

        std::stringstream s;
        s << "Error in scene file";
        
        //if a filename is given
        if(file.size()) {
          s << " \"" << file << "\"";
        }
        
        //if a line number is given
        if(line != -1) {
          s << " in line " << line;
        }

        s << ": " << info;
        message = s.str();
      }
      
      virtual ~SceneParseException() throw()
      {}
      
      virtual const char* what() {
        return message.c_str();
      }

      int getLine() {
        return line;
      }

      const char* getFile() {
        return file.c_str();
      }

      const char* getInfo() {
        return info.c_str();
      }

    protected:
      ///if known, the line where the error occured
      int line;

      ///if known, the filename of the scene file.
      std::string file;

      ///informative text to the user (without filename and line).
      std::string info;

      ///informative text including line and filename.
      std::string message;
    }; // end of class SceneParseException

  } // end of namespace interfaces
} // end of namespace mars

#endif /* MARS_BASE_SCENE_PARSE_EXCEPTION */
