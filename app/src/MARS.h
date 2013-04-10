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

#include <QApplication>
#include <iostream>
#include <string>

namespace mars {

  namespace lib_manager {
    class LibManager;
  }

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
      ~MARS();

      void readArguments(int argc, char **argv);
      void start(int argc, char **argv);
      int runWoQApp();
      inline lib_manager::LibManager* getLibManager() {return libManager;}

      static interfaces::ControlCenter *control;
      static bool quit;
      std::string configDir;
      std::string coreConfigFile;
      bool needQApp;

    private:
      lib_manager::LibManager *libManager;
      app::GraphicsTimer *graphicsTimer;
      interfaces::MarsGuiInterface *marsGui;


    };


    /**
     * This function provides a clean exit of the simulation
     * in case of a kill-signal.
     */

    class MyApp : public QApplication {
    public:
      MyApp(int &argc, char **argv) : QApplication(argc, argv) {}
      virtual bool notify( QObject *receiver, QEvent *event ) {
        try {
          return QApplication::notify(receiver, event);
        } catch (const std::exception &e) {
          std::cerr << e.what() << std::endl;
          throw(e);
        }
      }
    };

  } // end of namespace app
} // end of namespace mars

#endif // end of namespace MARS_H
