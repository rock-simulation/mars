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
 * \file MY_APP.h
 * \author Malte Langosz
 *
 */

#ifndef MY_APP_H
#define MY_APP_H

#ifndef NO_GUI
#include <QApplication>
#endif

namespace mars {

  namespace app {

    /**
     * This function provides a clean exit of the simulation
     * in case of a kill-signal.
     */

#ifdef NO_GUI
    class MyApp {
    public:
      MyApp(int &argc, char **argv) {}
      int exec() {
        abort();
        return 0;
      }
      void processEvents() {
        abort();
      }
    };
#else
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
#endif

  } // end of namespace app
} // end of namespace mars

#endif // end of namespace MARS_H
