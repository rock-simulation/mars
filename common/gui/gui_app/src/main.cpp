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

#include "GuiApp.hpp"
#include "MyApp.hpp"

#include <signal.h>

#include <stdexcept>

void qtExitHandler(int sig) {
  qApp->quit();
  gui_app::exit_main(sig);
}

void ignoreSignal(int sig)
{ (void)(sig); }

/**
 * The main function, that starts the GUI and init the physical environment.
 *Convention that start the simulation
 */
int main(int argc, char *argv[]) {

  //  Q_INIT_RESOURCE(resources);
#ifndef WIN32
  signal(SIGQUIT, qtExitHandler);
  signal(SIGPIPE, qtExitHandler);
  //signal(SIGKILL, qtExitHandler);
  signal(SIGUSR1, ignoreSignal);
#else
  signal(SIGFPE, qtExitHandler);
  signal(SIGILL, qtExitHandler);
  signal(SIGSEGV, qtExitHandler);
  signal(SIGBREAK, qtExitHandler);
#endif
  signal(SIGABRT, qtExitHandler);
  signal(SIGTERM, qtExitHandler);
  signal(SIGINT, qtExitHandler);

  std::vector<std::string> arguments;
  for(int i=0; i<argc; ++i) {
    arguments.push_back(argv[i]);
  }
  gui_app::GuiApp *gui = new gui_app::GuiApp();

  gui_app::MyApp *app = new gui_app::MyApp(argc, argv);

  gui->init(arguments);

  int state;
  state = app->exec();

  delete gui;
  return state;
}
