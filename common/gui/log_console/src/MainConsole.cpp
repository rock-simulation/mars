/*
 *  Copyright 2011, 2012, 2016 DFKI GmbH Robotics Innovation Center
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
 * \file Console.cpp
 * \author Malte Langosz
 * \brief "Console" is a template for the widget interface of the MARS GUI
 **/

#include "MainConsole.h"
#include <cstdio>
#include <iostream>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif


namespace mars {

  namespace log_console {

    MainConsole::MainConsole(lib_manager::LibManager *theManager) :
      lib_manager::LibInterface(theManager), gui(NULL),
      consoleWidget(NULL), cfg(NULL), set_window_prop(false) {

      setupGUI();
      dataBroker = theManager->getLibraryAs<data_broker::DataBrokerInterface>("data_broker");
      if(dataBroker) {
        dataBroker->registerSyncReceiver(this, "_MESSAGES_", "fatal", 
                                         data_broker::DB_MESSAGE_TYPE_FATAL);
        dataBroker->registerSyncReceiver(this, "_MESSAGES_", "error",
                                         data_broker::DB_MESSAGE_TYPE_ERROR);
        dataBroker->registerSyncReceiver(this, "_MESSAGES_", "warning",
                                         data_broker::DB_MESSAGE_TYPE_WARNING);
        dataBroker->registerSyncReceiver(this, "_MESSAGES_", "info",
                                         data_broker::DB_MESSAGE_TYPE_INFO);
        dataBroker->registerSyncReceiver(this, "_MESSAGES_", "debug",
                                         data_broker::DB_MESSAGE_TYPE_DEBUG);
      }
    }


    void MainConsole::setupGUI() {
      if(libManager == NULL) return;

      std::string path;

      cfg = libManager->getLibraryAs<cfg_manager::CFGManagerInterface>("cfg_manager");
      if(cfg) {
        cfg_manager::cfgPropertyStruct r_path;
        r_path = cfg->getOrCreateProperty("Preferences", "resources_path",
                                          std::string("."));
        path = r_path.sValue;
        showOnStdError = cfg->getOrCreateProperty("log_console",
                                                  "showOnStdError",
                                                  false, this);
        maxMessages = cfg->getOrCreateProperty("log_console",
                                               "maxLines",
                                               -1, this);
      }


      gui = libManager->getLibraryAs<main_gui::GuiInterface>("main_gui");
      if(gui) {
        path.append("/mars/log_console/resources/images/terminal.png");
        gui->addGenericMenuAction("../Tools/Console", 1,
                                  dynamic_cast<main_gui::MenuInterface*>(this),
                                  0, path, true);
	
        consoleWidget = new ConsoleGUI(NULL, cfg);
        consoleWidget->setMinimumSize(300, 200);
        connect(consoleWidget, SIGNAL(messageTypeChanged(int, bool)),
                this, SLOT(onMessageTypeChanged(int, bool)));
        if(!consoleWidget->getHiddenCloseState()){
          gui->addDockWidget((void*)consoleWidget, 1);
        }
        startTimer(20);
      
        //if(cfg) setupCFG();
      }
    }

    MainConsole::~MainConsole() {
      consoleWidget = NULL;
      if(dataBroker) libManager->releaseLibrary("data_broker");
      if(gui) libManager->releaseLibrary("main_gui");
      if(cfg) libManager->releaseLibrary("cfg_manager");
    }

    void MainConsole::menuAction(int action, bool checked) {
      if (gui == NULL || consoleWidget == NULL)
        return;

      switch(action) {
      case 1:
        if(consoleWidget->isHidden()) {
          gui->addDockWidget((void*)consoleWidget, 1);
        }
        else {
          gui->removeDockWidget((void*)consoleWidget, 1);
        }
        break;
      }
      (void)checked;
    }

    void MainConsole::add(const std::string &message, 
                          data_broker::MessageType type) {
      // output to the console widget, if it is loaded
      bool showInWidget = consoleWidget;

      //showing messages on stderr is nice because you can read
      //the error/debug messages even if the simulator crashes.

      if (showInWidget) {
        int wait = 100;
        con_data da;
        da.type = type;
        while(wait < 20) {
          consoleLock.lock();
          if(messages.size() < MAX_DE)
            wait = 20;
          consoleLock.unlock();
          if(wait++ < 20) {
#ifdef WIN32
            Sleep(10);
#else
            usleep(10000);
#endif
          }
        }
        consoleLock.lock();
        da.message = message;
        messages.push_back(da);
        consoleLock.unlock();
      }
      if (showOnStdError.bValue)
        {
          // output to stderr
          switch(type) {
          case data_broker::DB_MESSAGE_TYPE_FATAL:
          case data_broker::DB_MESSAGE_TYPE_ERROR:
#ifndef WIN32
            fprintf(stderr, "\033[1;31merror: %s\033[0m\n", message.c_str());
#else
            fprintf(stderr, "error: %s\n", message.c_str());
#endif
            break;
          case data_broker::DB_MESSAGE_TYPE_WARNING:
#ifndef WIN32
            fprintf(stderr, "\033[0;32mmessage: %s\033[0m\n", message.c_str());
#else
            fprintf(stderr, "message: %s\n", message.c_str());
#endif
            break;
          case data_broker::DB_MESSAGE_TYPE_INFO:
          case data_broker::DB_MESSAGE_TYPE_DEBUG:
#ifndef WIN32
            fprintf(stderr, "\033[1;34mwarning: %s\033[0m\n", message.c_str());
#else
            fprintf(stderr, "warning: %s\n", message.c_str());
#endif
            break;
          default:
            fprintf(stderr, "???: %s\n", message.c_str());
            break;
          }
        }
    }

    void MainConsole::addError(const std::string &my_error, ...) {
      va_list args;
      va_start(args, my_error);
      char message[255];
      vsprintf(message, my_error.data(), args);
      va_end(args);
      add(message, data_broker::DB_MESSAGE_TYPE_ERROR);  
    }

    void MainConsole::addWarning(const std::string &my_warning, ...) {
      va_list args;
      va_start(args, my_warning);
      char message[255];
      vsprintf(message, my_warning.data(), args);
      va_end(args);
      add(message, data_broker::DB_MESSAGE_TYPE_WARNING);
    }

    void MainConsole::addMessage(const std::string &my_message, ...) {
      va_list args;
      va_start(args, my_message);
      char message[255];
      vsprintf(message, my_message.data(), args);
      va_end(args);
      add(message, data_broker::DB_MESSAGE_TYPE_INFO);
    }

    void MainConsole::timerEvent(QTimerEvent* event) {
      (void)event;
      if (consoleWidget == NULL)
        return;
      consoleLock.lock();
      while(messages.size() > 0) {
        if(messages[0].type == data_broker::DB_MESSAGE_TYPE_ERROR)
          consoleWidget->setTextColor(QColor(212, 148, 90));
        else if(messages[0].type == data_broker::DB_MESSAGE_TYPE_WARNING)
          consoleWidget->setTextColor(QColor(90, 148, 212));
        else
          consoleWidget->setTextColor(QColor(90, 200, 70));
    
        consoleWidget->append(QString(messages[0].message.data()));
        messages.erase(messages.begin());
      }
      consoleLock.unlock();
    }

    void MainConsole::onMessageTypeChanged(int buttonId, bool state) {
      const char *dataNames[5] = {"fatal", "error", "warning", "info", "debug"};
      if(state == false) {
        dataBroker->unregisterSyncReceiver(this, "_MESSAGES_", 
                                           dataNames[buttonId]);
      } else if(state == true) {
        dataBroker->registerSyncReceiver(this, "_MESSAGES_", 
                                         dataNames[buttonId], buttonId);
      }
    }

    int MainConsole::getLibVersion() const {
      return 1;
    }

    const std::string MainConsole::getLibName() const {
      return "log_console";
    }

    void MainConsole::cfgUpdateProperty(cfg_manager::cfgPropertyStruct _property) {
      if(_property.paramId == showOnStdError.paramId) {
        showOnStdError.bValue = _property.bValue;
      }

      else if(_property.paramId == maxMessages.paramId) {
        maxMessages.iValue = _property.iValue;
        consoleWidget->setMaxLines(maxMessages.iValue);
      }
    }

    void MainConsole::receiveData(const data_broker::DataInfo &info,
                                  const data_broker::DataPackage &package,
                                  int callbackParam) {
      add(package[0].s, static_cast<data_broker::MessageType>(callbackParam));
    }

  } // end of namespace log_console

} // end of namespace mars


DESTROY_LIB(mars::log_console::MainConsole);
CREATE_LIB(mars::log_console::MainConsole);
