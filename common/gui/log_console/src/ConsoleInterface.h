/*
 *  Copyright 2011, 2012 DFKI GmbH Robotics Innovation Center
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
 * \file ConsoleInterface.h
 * \author Malte Roemmermann
 * \brief "ConsoleInterface" is an interface to a plugin to report errors and
 *    warnings
 *
 * Version 0.1 (23.01.09): 
 *               - 
 */

#ifndef CONSOLE_INTERFACE_H
#define CONSOLE_INTERFACE_H

#ifdef _PRINT_HEADER_
  #warning "ConsoleInterface.h"
#endif

#include <string>


namespace mars {

  /**
   * The Interface for a plugin to report errors and warnings.
   *
   */
  namespace log_console {

    class ConsoleInterface {
    public:
      ConsoleInterface(void) {}
      virtual ~ConsoleInterface(void) {}
      virtual void addError(const std::string &my_error, ...) = 0;
      virtual void addWarning(const std::string &my_warning, ...) = 0;
      virtual void addMessage(const std::string &my_message, ...) = 0;
    }; // end of class ConsoleInterface

  } // end of namespace log_console

} // end of namespace mars

#endif // CONSOLE_INTERFACE_H
