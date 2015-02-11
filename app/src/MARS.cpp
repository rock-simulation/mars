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

#include <lib_manager/LibManager.hpp>
#include <lib_manager/LibInterface.hpp>
#include <mars/main_gui/MainGUI.h>
#include <mars/interfaces/sim/SimulatorInterface.h>
#include <mars/interfaces/gui/MarsGuiInterface.h>
#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/utils/Thread.h>
#include <mars/utils/misc.h>
#include <mars/cfg_manager/CFGManagerInterface.h>

#include <QDir>

#ifdef WIN32
  #include <Windows.h>
#endif

#include <getopt.h>
#include <signal.h>

#ifndef DEFAULT_CONFIG_DIR
    #define DEFAULT_CONFIG_DIR "."
#endif

namespace mars {

  namespace app {

    mars::interfaces::ControlCenter* MARS::control = NULL;
    bool MARS::quit = false;

    void exit_main(int signal) {
#ifndef WIN32
      if(signal == SIGPIPE) {
        return;
      }
#endif
      MARS::quit = true;
      if (signal) {
        fprintf(stderr, "\nI think we exit with an error! Signal: %d\n", signal);
        MARS::control->sim->exitMars();
        exit(-1);
        //MARS::control->sim->exitMars();
        //Convention: print the signal type
      } else {
        MARS::control->sim->exitMars();
        fprintf(stderr, "\n################################\n");
        fprintf(stderr, "## everything closed fine ^-^ ##\n");
        fprintf(stderr, "################################\n\n");
      }
    }

    void handle_abort(int signal) {
      MARS::control->sim->handleError(interfaces::PHYSICS_DEBUG);
    }

    MARS::MARS() : configDir(DEFAULT_CONFIG_DIR),
                   libManager(new lib_manager::LibManager()),
                   marsGui(NULL), ownLibManager(true) {
      needQApp = true;
      graphicsTimer = NULL;
#ifdef WIN32
      // request a scheduler of 1ms
      timeBeginPeriod(1);
#endif //WIN32
    }

    MARS::MARS(lib_manager::LibManager *theManager) : configDir(DEFAULT_CONFIG_DIR),
                   libManager(theManager),
                   marsGui(NULL), ownLibManager(false) {
      needQApp = true;
      graphicsTimer = NULL;
#ifdef WIN32
      // request a scheduler of 1ms
      timeBeginPeriod(1);
#endif //WIN32
    }

    MARS::~MARS() {
      //! close simulation
      exit_main(0);

      if(graphicsTimer) delete graphicsTimer;

      libManager->releaseLibrary("mars_sim");
      if(marsGui) libManager->releaseLibrary("mars_gui");
      if(control->graphics) libManager->releaseLibrary("mars_graphics");
      libManager->releaseLibrary("main_gui");
      libManager->releaseLibrary("cfg_manager");

      if(ownLibManager) delete libManager;

#ifdef WIN32
      // end scheduler of 1ms
      timeEndPeriod(1);
#endif //WIN32

    }

    void MARS::start(int argc, char **argv, bool startThread) {

      // then check locals
#ifndef WIN32
      setenv("LC_ALL","C", 1);
      unsetenv("LANG");
      setlocale(LC_ALL,"C");
#endif

      // Test if current locale supports ENGLISH number interpretation
      struct lconv *locale = localeconv();
      fprintf(stderr, "Active locale (LC_ALL): ");
      if( *(locale->decimal_point) != '.') {
        fprintf(stderr, " [FAIL] Current locale conflicts with mars\n");
        exit(1);
      } else {
        fprintf(stderr, " [OK]\n");
      }

      // load the simulation core_libs:
      coreConfigFile = configDir+"/core_libs.txt";
      FILE *plugin_config;
      plugin_config = fopen(coreConfigFile.c_str() , "r");
      if(plugin_config) {
        fclose(plugin_config);
        libManager->loadConfigFile(coreConfigFile);
      } else {
        fprintf(stderr, "Loading default core libraries...\n");
        libManager->loadLibrary("data_broker");
        libManager->loadLibrary("cfg_manager");
        libManager->loadLibrary("main_gui");
        libManager->loadLibrary("mars_graphics");
        libManager->loadLibrary("mars_sim");
        libManager->loadLibrary("mars_scene_loader");
        libManager->loadLibrary("mars_gui");
        libManager->loadLibrary("mars_entity_factory");
        libManager->loadLibrary("mars_smurf");
        libManager->loadLibrary("mars_smurf_loader");
      }

      mars::cfg_manager::CFGManagerInterface *cfg;
      cfg = libManager->getLibraryAs<mars::cfg_manager::CFGManagerInterface>("cfg_manager");
      if(cfg) {
        cfg_manager::cfgPropertyStruct configPath;
        configPath = cfg->getOrCreateProperty("Config", "config_path",
                                              configDir);
        configPath.sValue = configDir;
        cfg->setProperty(configPath);
      }

      // then get the simulation
      mars::interfaces::SimulatorInterface *marsSim;
      marsSim = libManager->getLibraryAs<mars::interfaces::SimulatorInterface>("mars_sim");
      if(!marsSim) {
        fprintf(stderr, "main: error while casting simulation lib\n\n");
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

      mars::interfaces::GraphicsManagerInterface *marsGraphics = NULL;
      lib_manager::LibInterface *lib= libManager->getLibrary("mars_graphics");
      if(lib) {
        if( (marsGraphics = dynamic_cast<mars::interfaces::GraphicsManagerInterface*>(lib)) ) {
          // init osg
          //initialize graphicsFactory
          if(mainGui) {
            marsGraphics->initializeOSG(NULL);
            void *widget = marsGraphics->getQTWidget(1);
            if (widget && mainGui) {
              mainGui->mainWindow_p()->setCentralWidget((QWidget*)widget);
            }
            if(widget) ((QWidget*)widget)->show();
          }
          else {
            marsGraphics->initializeOSG(NULL, false);
          }
        }
      }

      // load the simulation other_libs:
      std::string otherConfigFile = configDir+"/other_libs.txt";
      plugin_config = fopen(otherConfigFile.c_str() , "r");
      if(plugin_config) {
        fclose(plugin_config);
        libManager->loadConfigFile(otherConfigFile);
      } else {
        fprintf(stderr, "Loading default additional libraries...\n");
        libManager->loadLibrary("connexion_plugin");
        libManager->loadLibrary("data_broker_gui");
        libManager->loadLibrary("cfg_manager_gui");
        libManager->loadLibrary("lib_manager_gui");
      }


      // if we have a main gui, show it
      if(mainGui) mainGui->show();

      control->sim->runSimulation(startThread);

      if(needQApp) {
        graphicsTimer = new mars::app::GraphicsTimer(marsGraphics,
                                                     control->sim);
        graphicsTimer->run();
      }
    }


    void MARS::readArguments(int argc, char **argv) {
      int c;
      int option_index = 0;

      int i;
      char** argv_copy = NULL;
      unsigned int arg_len = 0;

      //remember how many arguments were already processed by getopt()
      const int old_opterr = opterr;

      //do not print error messages for unkown arguments
      //(may be a valid option for mars_sim).
      opterr = 0;

      static struct option long_options[] = {
        {"config_dir", required_argument, 0, 'C'},
        {"noQApp",no_argument,0,'Q'},
        {0, 0, 0, 0}
      };

      // copy the argument vector in order to prevent messing around with the order
      // of the arguments by getopt
      argv_copy = (char**) malloc(argc*sizeof(char*));
      for (i = 0; i < argc; i++) {
        arg_len = strlen(argv[i]);
        argv_copy[i] = (char*) malloc((arg_len+1)*sizeof(char));
        memcpy(argv_copy[i],argv[i],arg_len);
        argv_copy[i][arg_len] = '\0';
      }

      // here just work with the copied argument vector ...
      while (1) {

#ifdef __linux__
        c = getopt_long(argc, argv, "C:Q", long_options, &option_index);
#else
        c = getopt_long(argc, argv_copy, "C:Q", long_options, &option_index);
#endif
        if (c == -1)
          break;
        switch (c) {
        case 'C':
          if( QDir(QString(optarg)).exists() )
            configDir = optarg;
          else
            printf("The given configuration Directory does not exists: %s\n",
                   optarg);
          break;
        case 'Q':
          needQApp = false;
          break;
        }
      }

      // clean up the copied argument vector
      for (i = 0; i < argc; i++) {
        free(argv_copy[i]);
      }
      free(argv_copy);

      //reset error message printing to original setting
      opterr = old_opterr;

      //reset index to read arguments again in other libraries (mars_sim).
      optind = 1;
      optarg = NULL;

      return;
    }

    int MARS::runWoQApp() {
      while(!quit) {
        if(control->sim->getAllowDraw() || !control->sim->getSyncGraphics()) {
          control->sim->finishedDraw();
        }
        //mars::utils::msleep(2);
      }
      return 0;
    }

  } // end of namespace app
} // end of namespace mars
