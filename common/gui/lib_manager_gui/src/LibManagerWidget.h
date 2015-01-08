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
 * \file LibManagerWidget.h
 * \author Lorenz Quack (lorenz.quack@dfki.de)
 * \brief GUI for the LibManager
 *
 * Version 0.1
 */

#ifndef MARS_PLUGINS_LIB_MANAGER_WIDGET_H
#define MARS_PLUGINS_LIB_MANAGER_WIDGET_H

#ifdef _PRINT_HEADER_
  #warning "LibManagerWidget.h"
#endif

#include <mars/main_gui/BaseWidget.h>
#include <mars/cfg_manager/CFGManagerInterface.h>

#include <string>
#include <map>

#include <QTableWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
class QWidget;

namespace mars {

  namespace plugins {
    namespace lib_manager_gui {

      class LibManagerWidget : public mars::main_gui::BaseWidget {
        
        Q_OBJECT;
        
      public:
        LibManagerWidget(QWidget *parent,
                         cfg_manager::CFGManagerInterface *cfg);
        ~LibManagerWidget();

        // LibManagerWidget methods
        void updateLibInfo(const lib_manager::LibInfo &info);
        void clear();
        void setDefaultLibPath(const std::string &path);

      private slots:
        void onLoad();
        void onUnload();
        void onDump();

      signals:
        void load(std::string libPath);
        void unload(std::string libName);
        void dump(std::string dumppath);

      private:
        QTableWidget *table;
        QPushButton *loadButton;
        QPushButton *unloadButton;
        QPushButton *dumpButton;
        QString defaultLibPath;

        friend class LibManagerGui; // needed for access to onLoad.

      }; // end of class definition LibManagerWidget

    } // end of namespace lib_manager_gui
  } // end of namespace plugins
} // end of namespace mars

#endif // MARS_PLUGINS_LIB_MANAGER_WIDGET_H
