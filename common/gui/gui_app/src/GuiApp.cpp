/*
 *  Copyright 2015, DFKI GmbH Robotics Innovation Center
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
 * \file GuiApp.cpp
 * \author Malte Langosz
 *
 */

#include "GuiApp.hpp"

#include <lib_manager/LibManager.hpp>
#include <mars/main_gui/MainGUI.h>
#ifdef WIN32
#include <Windows.h>
#endif

#include <getopt.h>
#include <signal.h>
#include <locale.h>

namespace gui_app {

  void exit_main(int signal) {
#ifndef WIN32
    if(signal == SIGPIPE) {
      return;
    }
#endif
    if (signal) {
      fprintf(stderr, "\nI think we exit with an error! Signal: %d\n", signal);
      exit(-1);
    } else {
      fprintf(stderr, "\n################################\n");
      fprintf(stderr, "## everything closed fine ^-^ ##\n");
      fprintf(stderr, "################################\n\n");
    }
  }

  GuiApp::GuiApp() : libManager(new lib_manager::LibManager()), configDir(".") {
#ifdef WIN32
    // request a scheduler of 1ms
    timeBeginPeriod(1);
#endif //WIN32
  }

  GuiApp::~GuiApp() {

    //! close simulation
    exit_main(0);

    libManager->releaseLibrary("cfg_manager");
    libManager->releaseLibrary("lib_manager_gui");
    libManager->releaseLibrary("main_gui");

    libManager->clearLibraries();
    delete libManager;


#ifdef WIN32
    // end scheduler of 1ms
    timeEndPeriod(1);
#endif //WIN32

  }

  void GuiApp::init(const std::vector<std::string> &arguments) {
    mars::main_gui::MainGUI *mainGui;

    // then check locals
    setlocale(LC_ALL,"C");

    cfg = libManager->getLibraryAs<mars::cfg_manager::CFGManagerInterface>("cfg_manager", true);
    if(!cfg) {
      fprintf(stderr, "can not load needed library \"cfg_manager\".\n");
      exit(-1);
    }

    // pipe arguments into cfg_manager
    char label[55];
    for(size_t i=0; i<arguments.size(); ++i) {
      size_t f = arguments[i].find("=");
      if(f != std::string::npos) {
        cfg->getOrCreateProperty("Config", arguments[i].substr(0, f),
                                 arguments[i].substr(f+1));
      }
      else {
        sprintf(label, "arg%zu", i);
        cfg->getOrCreateProperty("Config", label, arguments[i]);
      }
    }
    configDir = cfg->getOrCreateProperty("Config", "config_path",
                                         configDir).sValue;
    std::string coreConfigFile = configDir+"/libs.txt";

    std::string p = configDir+"/preferences.yml";
    FILE *f = fopen(p.c_str(), "r");
    if(f) {
      fclose(f);
      cfg->loadConfig(p.c_str());
    }
    mainGui = libManager->getLibraryAs<mars::main_gui::MainGUI>("main_gui",
                                                                true);

    if(!mainGui) {
      fprintf(stderr, "can not load needed library \"main_gui\".\n");
      exit(-1);
    }

    libManager->getLibraryAs<void>("lib_manager_gui", true);

    // load the simulation core_libs:
    libManager->loadConfigFile(coreConfigFile);

    mainGui->show();
  }

} // end of namespace gui_app
