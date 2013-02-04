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
 * \file LibManagerGui.h
 * \author Lorenz Quack (lorenz.quack@dfki.de)
 * \brief GUI for the LibManager
 *
 * Version 0.1
 */

#ifndef MARS_PLUGINS_LIB_MANAGER_GUI_H
#define MARS_PLUGINS_LIB_MANAGER_GUI_H

#ifdef _PRINT_HEADER_
#warning "LibManagerGui.h"
#endif

#include <mars/main_gui/MenuInterface.h>
#include <mars/main_gui/GuiInterface.h>
#include <mars/lib_manager/LibInterface.h>
#include <mars/cfg_manager/CFGManagerInterface.h>

#include <QObject>
#include <QTimer>

#include <string>

namespace mars {

  namespace lib_manager {
    class LibManager;
  }

  namespace plugins {
    namespace lib_manager_gui {

      class LibManagerWidget;

      class LibManagerGui : public QObject,
                            public mars::lib_manager::LibInterface,
                            public mars::main_gui::MenuInterface {

        Q_OBJECT;

      public:
        LibManagerGui(mars::lib_manager::LibManager *theManager);
        ~LibManagerGui();

        // LibInterface methods
        int getLibVersion() const
        { return 1; }
        const std::string getLibName() const
        { return std::string("lib_manager_gui"); }
        void newLibLoaded(const std::string &libName);
        CREATE_MODULE_INFO();

        // MenuInterface methods
        void menuAction(int action, bool checked = false);

        // LibManagerGui methods
        void setupGUI(std::string rPath);

      private slots:
        void init();
        void load(std::string libPath);
        void unload(std::string libName);
        void dumpTo(std::string filepath);
        void update();

      private:
        LibManagerWidget *widget;
        cfg_manager::CFGManagerInterface *cfg;
        main_gui::GuiInterface *gui;
        QTimer timer;

      }; // end of class definition LibManagerGui

    } // end of namespace lib_manager_gui
  } // end of namespace plugins
} // end of namespace mars

#endif // MARS_PLUGINS_LIB_MANAGER_GUI_H
