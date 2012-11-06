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

#include "MARS.h"

#include <signal.h>
//#include "HandleFileNames.h"
#include <QApplication>
#include <QPlastiqueStyle>


#include <stdexcept>
#include <iostream>


namespace mars {

  namespace app {

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


void qtExitHandler(int sig) {
  qApp->quit();
  mars::app::exit_main(sig);
}

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
#else
  signal(SIGFPE, qtExitHandler);
  signal(SIGILL, qtExitHandler);
  signal(SIGSEGV, qtExitHandler);
  signal(SIGBREAK, qtExitHandler);
#endif
  signal(SIGABRT, qtExitHandler);
  signal(SIGTERM, qtExitHandler);
  signal(SIGINT, qtExitHandler);


  mars::app::MARS *simulation = new mars::app::MARS();

  // first setup qapp
  //QApplication *app = new QApplication(argc, argv);
  mars::app::MyApp *app = new mars::app::MyApp(argc, argv);
  app->setStyle(new QPlastiqueStyle);

  // for osx relase build:
  /*
    QDir dir(QApplication::applicationDirPath());
    dir.cdUp();
    dir.cd("PlugIns");
    QApplication::setLibraryPaths(QStringList(dir.absolutePath()));
  */

  //app->setWindowIcon(QIcon(QString::fromStdString(Pathes::getGuiPath()) + "images/mars_icon.ico"));

  simulation->start(argc, argv);

  int state = app->exec();

  delete simulation;
  return state;
}
