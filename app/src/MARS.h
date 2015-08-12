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

/**
 * \file MARS.h
 * \author Malte Roemmermann
 *
 */

#ifndef MARS_H
#define MARS_H


#ifdef _PRINT_HEADER_
  #warning "MARS.h"
#endif

#include <iostream>
#include <string>

namespace lib_manager {
  class LibManager;
}

namespace mars {


  namespace interfaces {
    class ControlCenter;
  }

  namespace interfaces {
    class MarsGuiInterface;
  }

  namespace app {
    class GraphicsTimer;

    void exit_main(int signal);
    void handle_abort(int signal);

    class MARS {
    public:
      MARS();
      MARS(lib_manager::LibManager *theManager);
      ~MARS();

      void readArguments(int argc, char **argv);
      void init();
      void start(int argc, char **argv, bool startThread = true,
                 bool handleLibraryLoading = true);
      int runWoQApp();
      inline lib_manager::LibManager* getLibManager() {return libManager;}

      static interfaces::ControlCenter *control;
      static bool quit;
      std::string configDir;
      std::string coreConfigFile;
      bool needQApp, noGUI;

    private:
      lib_manager::LibManager *libManager;
      app::GraphicsTimer *graphicsTimer;
      interfaces::MarsGuiInterface *marsGui;
      bool ownLibManager;
      bool argConfDir;
      bool initialized;
    };

  } // end of namespace app
} // end of namespace mars

#endif // end of namespace MARS_H
