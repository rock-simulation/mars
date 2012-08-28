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
 * \file MARS.cpp
 * \author Malte Roemmermann
 *
 */

#include "MARS.h"

#include "GraphicsTimer.h"

#include <mars/lib_manager/LibManager.h>
#include <mars/lib_manager/LibInterface.h>
#include <mars/main_gui/MainGUI.h>
#include <mars/interfaces/sim/SimulatorInterface.h>
#include <mars/interfaces/gui/MarsGuiInterface.h>
#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/utils/Thread.h>
#include <mars/cfg_manager/CFGManagerInterface.h>

#include <QDir>

#ifdef WIN32
  #include <Windows.h>
#endif

#include <getopt.h>
#include <signal.h>

namespace mars {

  namespace app {

    mars::interfaces::ControlCenter* MARS::control = NULL;

    void exit_main(int signal) {
#ifndef WIN32
      if(signal == SIGPIPE) {
        return;
      }
#endif
      if (signal) {
        fprintf(stderr, "I think we exit with an error!");
        MARS::control->sim->exitMars();
        //Convention: print the signal type
      } else {
        MARS::control->sim->exitMars();
        fprintf(stderr, "\n\n################################");
        fprintf(stderr, "\n## everything closed fine ^-^ ##");
        fprintf(stderr, "\n################################\n\n");
      }
    }

    void handle_abort(int signal) {
      MARS::control->sim->handleError(1);
    }

    MARS::MARS() : configDir("."),
                   libManager(new lib_manager::LibManager()),
                   marsGui(NULL) {
      graphicsTimer = NULL;
#ifdef WIN32
      // request a scheduler of 1ms
      timeBeginPeriod(1);
#endif //WIN32
      coreConfigFile = configDir+"/core_libs.txt";
    }

    MARS::~MARS() {
      //! close simulation
      //fprintf(stderr, "\n want to exit\n");
      //fprintf(stderr, "\n want to exit\n");
      exit_main(0);

      if(graphicsTimer) delete graphicsTimer;

      libManager->unloadLibrary("mars_sim");
      if(marsGui) libManager->unloadLibrary("mars_gui");
      if(control->graphics) libManager->unloadLibrary("mars_graphics");
      libManager->unloadLibrary("main_gui");

      delete libManager;

#ifdef WIN32
      // end scheduler of 1ms
      timeEndPeriod(1);
#endif //WIN32

    }

    void MARS::start(int argc, char **argv) {
      readArguments(argc, argv);

      // then check locals
      setlocale(LC_ALL,"C");

      // Test if current locale supports ENGLISH number interpretation
      struct lconv *locale = localeconv();
      fprintf(stderr, "Active locale (LC_ALL): ");
      if( *(locale->decimal_point) != '.') {
        fprintf(stderr, " [FAIL] n");
        fprintf(stderr, "Current locale conflicts with mars\n");
        exit(1);
      }
      //fprintf(stderr, " [OK]\n");
    
      // load the simulation core_libs:
      libManager->loadConfigFile(coreConfigFile);

      mars::cfg_manager::CFGManagerInterface *cfg;
      cfg = libManager->getLibraryAs<mars::cfg_manager::CFGManagerInterface>("cfg_manager");
      if(cfg) {
        cfg_manager::cfgPropertyStruct configPath;
        cfg->getOrCreateProperty("Config", "config_path", configDir);
        configPath.sValue = configDir;
        cfg->setProperty(configPath);
      }

      // then get the simulation
      mars::interfaces::SimulatorInterface *marsSim;
      marsSim = libManager->getLibraryAs<mars::interfaces::SimulatorInterface>("mars_sim");
      if(!marsSim) {
        fprintf(stderr, "\nmain: error while casting simulation lib\n\n");
        exit(2);
      }
      control = marsSim->getControlCenter();
      // then read the simulation arguments
      control->sim->readArguments(argc, argv);

      marsGui = libManager->getLibraryAs<mars::interfaces::MarsGuiInterface>("mars_gui");
      if(marsGui) {
        marsGui->setupGui();
      }

      mars::main_gui::MainGUI *mainGui = NULL;
      mainGui = libManager->getLibraryAs<mars::main_gui::MainGUI>("main_gui");

      control->sim->runSimulation();

      mars::interfaces::GraphicsManagerInterface *marsGraphics = NULL;
      mars::lib_manager::LibInterface *lib= libManager->getLibrary("mars_graphics");
      if(lib) {
        if( (marsGraphics = dynamic_cast<mars::interfaces::GraphicsManagerInterface*>(lib)) ) {
          // init osg
          //initialize graphicsFactory
          marsGraphics->initializeOSG(NULL);
          void *widget = marsGraphics->getQTWidget(1);
          if (widget && mainGui) {
            mainGui->mainWindow_p()->setCentralWidget((QWidget*)widget);
          }
          if(widget) ((QWidget*)widget)->show();
        }
      }

      libManager->loadConfigFile(configDir+"/other_libs.txt");

      // if we have a main gui, show it
      if(mainGui) mainGui->show();

      graphicsTimer = new mars::app::GraphicsTimer(marsGraphics, control->sim);
      graphicsTimer->run();
    }


    void MARS::readArguments(int argc, char **argv) {
      int c;
      int option_index = 0;

      //remember how many arguments were already processed by getopt()
      const int old_opterr = opterr;

      //do not print error messages for unkown arguments
      //(may be a valid option for mars_sim).
      opterr = 0;

      static struct option long_options[] = {
        {"config_dir", required_argument, 0, 'C'},
        {0, 0, 0, 0}
      };

      while (1) {
        c = getopt_long(argc, argv, "C:", long_options, &option_index);
        if (c == -1)
          break;
        switch (c) {
        case 'C':
          if( QDir(QString(optarg)).exists() ) configDir = optarg;
          else printf("\nThe given configuration Directory does not exists: %s", optarg);
          break;
        }
      }

      //reset error message printing to original setting
      opterr = old_opterr;

      //reset index to read arguments again in other libraries (mars_sim).
      optind = 1;

      return;
    }

  } // end of namespace app
} // end of namespace mars
