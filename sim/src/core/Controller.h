/*
 *  Copyright 2011, 2012, DFKI GmbH Robotics Innovation Center
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
 * \file Controller.h
 * \author Malte Roemmermann
 * \brief "Controller" provides an architecture to log all relevant data from
 *        the simulation in file, GUI or any other receiver
 */

#ifndef CONTROLLER_H
#define CONTROLLER_H

#ifdef _PRINT_HEADER_
  #warning "Controller.h"
#endif

#include "SimMotor.h"

#ifdef WIN32
#include <windows.h>
#include <winsock.h>
#include <io.h>
#else
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdlib.h>
#include <netinet/in.h>
#endif
#include <stdio.h>

#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/utils/Thread.h>
#include <mars/interfaces/sensor_bases.h>
#include <mars/interfaces/ControllerData.h>
#include <mars/interfaces/sim/ControllerInterface.h>

namespace mars {
  namespace sim {

    /**
     * The class to log all data.
     *
     */
    class Controller : public utils::Thread {
    public:
      Controller(interfaces::sReal rate,
                 const std::vector<SimMotor*> &motors,
                 const std::vector<interfaces::BaseSensor*> &sensors,
                 const std::vector<interfaces::NodeData*> &sNodes,
                 interfaces::ControlCenter *control, int portn=1500);
      virtual ~Controller(void);
      virtual void update(interfaces::sReal time_ms);
      virtual std::list<interfaces::sReal> getSensorValues(void);

      void handleError(void);
      void setID(unsigned long id);
      unsigned long getID(void) const;
      const interfaces::ControllerData getSController() const;
      void getCoreExchange(interfaces::core_objects_exchange *obj) const;
      void resetData(void);
      void setDylibPath(const std::string &dylib_path);

      void setAutoMode(bool mode);
      void setIP(const std::string &ip);
      void setPort(int port);
      bool getAutoMode(void) const;
      const std::string getIP(void) const;
      int getPort(void) const;
      void connect(void);
      void disconnect(void);

#ifdef WIN32
      static bool sock_init;
#endif

    protected:
#ifdef WIN32
      SOCKET server;
      SOCKET conn;
      HINSTANCE dy;
#else
      int server;
      int conn;
      void *dy;
#endif
      interfaces::ControllerData sController;
      interfaces::ControllerInterface *dylibController;
      interfaces::sReal count_ms;
      bool auto_connect;
      int connected;
      int nport;
      int sock_state;
      struct sockaddr_in *myServAddr;
      std::string hostname;
      interfaces::ControlCenter *control;
      bool running;
      std::vector<SimMotor*> motors;
      std::vector<interfaces::BaseSensor*> sensors;
      std::vector<interfaces::NodeData*> sNodes;
      int initServer(int port);
      void getClient(void);
      int openClient(const char *host, int port);
      int connectClient(void);
      int getSReal(const char *data, interfaces::sReal *value) const;
      int getChar(const char *data, char *c) const;
      void run(void);
    };

  } // end of namespace sim
} // end of namespace mars

#endif  // CONTROLLER_H
