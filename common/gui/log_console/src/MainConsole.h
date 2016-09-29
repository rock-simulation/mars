/*
 *  Copyright 2011, 2012, 2016, DFKI GmbH Robotics Innovation Center
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
 * \file Console.h
 * \author Malte Langosz
 * \brief "Console" is a template for the widget interface of the MARS GUI
 **/

#ifndef MAIN_CONSOLE_H
#define MAIN_CONSOLE_H

#ifdef _PRINT_HEADER_
#warning "MainConsole.h"
#endif

#include "ConsoleGUI.h"
#include "ConsoleInterface.h"

#include <lib_manager/LibInterface.hpp>
#include <mars/data_broker/ReceiverInterface.h>
#include <mars/data_broker/DataBrokerInterface.h>
#include <mars/cfg_manager/CFGManagerInterface.h>
#include <mars/cfg_manager/CFGClient.h>
#include <mars/main_gui/GuiInterface.h>
#include <mars/main_gui/MenuInterface.h>

#include <string>

#include <QMutex>
#include <QTimerEvent>


namespace mars {

  namespace log_console {

#define MAX_DE 20

    struct con_data {
      std::string message;
      data_broker::MessageType type;
    };

    class MainConsole : public QObject, public lib_manager::LibInterface,
                        public main_gui::MenuInterface, 
                        public ConsoleInterface, 
                        public cfg_manager::CFGClient, 
                        public data_broker::ReceiverInterface {
      Q_OBJECT;

    public:
      MainConsole(lib_manager::LibManager *theManager);
      void setupGUI();
  
      // ConsoleInterface methods
      virtual ~MainConsole(void);
      virtual void add(const std::string &msg, data_broker::MessageType type);
      virtual void addError(const std::string &my_error, ...);
      virtual void addWarning(const std::string &my_warning, ...);
      virtual void addMessage(const std::string &my_message, ...);
  
      // MenuInterface methods
      virtual void menuAction(int action, bool checked = false);

      // LibInterface methods
      int getLibVersion() const;
      const std::string getLibName() const;
      CREATE_MODULE_INFO();

      // CFGClient
      virtual void cfgUpdateProperty(cfg_manager::cfgPropertyStruct _property);

      // data_broker::ReceiverInterface
      virtual void receiveData(const data_broker::DataInfo &info,
                               const data_broker::DataPackage &package,
                               int callbackParam);

    private:
      main_gui::GuiInterface *gui;
      data_broker::DataBrokerInterface *dataBroker;
      ConsoleGUI *consoleWidget;
      QMutex consoleLock;
      std::vector<con_data> messages;
      // geometry config
      cfg_manager::CFGManagerInterface *cfg;
      cfg_manager::cfgPropertyStruct showOnStdError, maxMessages;
      void setupCFG(void);
      bool set_window_prop;
      int ignore_next_resize;

    protected slots:
      void timerEvent(QTimerEvent *event);
      void onMessageTypeChanged(int buttonId, bool state);

    }; // end of class MainConsole

  } // end of namespace log_console

} // end of namespace mars

#endif // MAIN_CONSOLE_H
