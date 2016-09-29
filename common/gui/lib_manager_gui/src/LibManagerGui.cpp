/*
 *  Copyright 2013, DFKI GmbH Robotics Innovation Center
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
 * \file LibManagerGui.cpp
 * \author Lorenz Quack (lorenz.quack@dfki.de)
 * \brief GUI for the LibManager
 *
 * Version 0.1
 */


#include "LibManagerGui.h"
#include "LibManagerWidget.h"

#include <cstdio>
#include <cassert>

#include <QFileInfo>
#include <QDir>

namespace mars {
  namespace plugins {
    namespace lib_manager_gui {

      using namespace std;
      using cfg_manager::CFGManagerInterface;
      using main_gui::GuiInterface;

      enum MenuCallback {
        CALLBACK_INFO = 1,
        CALLBACK_LOAD,
      };

      LibManagerGui::LibManagerGui(lib_manager::LibManager *theManager)
        : lib_manager::LibInterface(theManager)
        , mars::main_gui::MenuInterface()
        , widget(NULL)
        , cfg(NULL)
        , gui(NULL) {

        setupGUI("../resources/");
        connect(&timer, SIGNAL(timeout()), this, SLOT(update()));
        timer.start(1000);
        // defer init because the LibManager fills the info object after
        // the constructor returns.
        QTimer::singleShot(1, this, SLOT(init()));
      }

      LibManagerGui::~LibManagerGui() {
        timer.stop();
        if(cfg)
          libManager->releaseLibrary(cfg->getLibName());
        if(gui)
          libManager->releaseLibrary(gui->getLibName());
      }

      void LibManagerGui::init() {
        // get the default lib directory by looking at our own LibInfo
        lib_manager::LibInfo info = libManager->getLibraryInfo(getLibName());
        QDir libDir =QFileInfo(QString::fromStdString(info.path)).absoluteDir();
        string libDirPath = libDir.absolutePath().toStdString();
        widget->setDefaultLibPath(libDirPath);
      }

      void LibManagerGui::load(std::string libPath) {
        libManager->loadLibrary(libPath);
      }

      void LibManagerGui::unload(std::string libName) {
        libManager->unloadLibrary(libName);
        std::list<std::string> libNameList;
        libManager->getAllLibraryNames(&libNameList);
        widget->clear();
        for(std::list<std::string>::iterator it = libNameList.begin();
            it != libNameList.end(); ++it) {
          lib_manager::LibInfo info = libManager->getLibraryInfo(*it);
          widget->updateLibInfo(info);
        }
      }

      void LibManagerGui::dumpTo(std::string filepath) {
        libManager->dumpTo(filepath);
      }

      void LibManagerGui::update() {
        if(!widget || !widget->isVisible())
          return;
        std::list<std::string> libNameList;
        libManager->getAllLibraryNames(&libNameList);
        for(std::list<std::string>::iterator it = libNameList.begin();
            it != libNameList.end(); ++it) {
          lib_manager::LibInfo info = libManager->getLibraryInfo(*it);
          widget->updateLibInfo(info);
        }
      }

      void LibManagerGui::setupGUI(std::string rPath) {
        if(!libManager)
          return;

        cfg = libManager->getLibraryAs<CFGManagerInterface>("cfg_manager");
        if(!cfg) {
          fprintf(stderr, "******* LibManagerGui: couldn't find cfg_manager\n");
          return;
        }

        gui = libManager->getLibraryAs<GuiInterface>("main_gui");
        if(!gui) {
          fprintf(stderr, "******* LibManagerGui: couldn't find main_gui\n");
          return;
        }

        gui->addGenericMenuAction("../File/Library Info...", CALLBACK_INFO,
                                  this);
        gui->addGenericMenuAction("../File/Load Library...", CALLBACK_LOAD,
                                  this);

        widget = new LibManagerWidget(NULL, cfg);
        connect(widget, SIGNAL(load(std::string)),
                this, SLOT(load(std::string)));
        connect(widget, SIGNAL(unload(std::string)),
                this, SLOT(unload(std::string)));
        connect(widget, SIGNAL(dump(std::string)),
                this, SLOT(dumpTo(std::string)));
      }

      void LibManagerGui::menuAction(int action, bool checked) {
        if(!widget)
          return;
        switch(action) {
        case CALLBACK_INFO:
          if(widget->isHidden()) {
            gui->addDockWidget(widget, 1);
            widget->show();
          } else {
            widget->hide();
            gui->removeDockWidget(widget, 1);
          }
          break;
        case CALLBACK_LOAD:
          widget->onLoad();
          break;
        }
      }

      void LibManagerGui::newLibLoaded(const std::string &libName) {
        if(!widget)
          return;
        lib_manager::LibInfo info = libManager->getLibraryInfo(libName);
        widget->updateLibInfo(info);
      }



    } // end of namespace lib_manager_gui
  } // end of namespace plugins
} // end of namespace mars

DESTROY_LIB(mars::plugins::lib_manager_gui::LibManagerGui);
CREATE_LIB(mars::plugins::lib_manager_gui::LibManagerGui);
