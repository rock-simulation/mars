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
 * \file MainDataGui.cpp
 * \author Malte Roemmermann
 * \brief
 **/

#include "MainDataGui.h"

#include <mars/data_broker/DataBrokerInterface.h>

#include <cstdio>
#include <iostream>

namespace mars {

  namespace data_broker_gui {

    using namespace cfg_manager;

    MainDataGui::MainDataGui(lib_manager::LibManager *theManager) :
      lib_manager::LibInterface(theManager),
      gui(NULL), cfg(NULL), dataBroker(NULL), dataWidget(NULL) {

      // setup GUI with default path
      setupGUI("../resources/");
    }

    void MainDataGui::setupGUI(std::string path) {

      if(libManager == NULL) return;

      cfg = libManager->getLibraryAs<CFGManagerInterface>("cfg_manager");
      if(cfg) {
        cfg_manager::cfgPropertyStruct r_path;
        r_path = cfg->getOrCreateProperty("Preferences", "resources_path",
                                          std::string("../resources"));
        path = r_path.sValue;
      } else {
        fprintf(stderr, "******* data_broker_gui: couldn't find cfg_manager\n");
      }

      gui = libManager->getLibraryAs<main_gui::GuiInterface>("main_gui");
      if (gui == NULL)
        return;

      dataBroker = libManager->getLibraryAs<data_broker::DataBrokerInterface>("data_broker");

      std::string path2 = path;
      path2.append("/mars/data_broker_gui/resources/images/data_broker_symbol.png");
      gui->addGenericMenuAction("../Data/DataBroker", 1, this, 0,
                                path2, true);

      path2 = path;
      path2.append("/mars/data_broker_gui/resources/images/data_broker_connection_symbol.png");
      gui->addGenericMenuAction("../Data/DataBrokerConnections", 2, this, 0,
                                path2, true);

      dataWidget = NULL;//new DataWidget(dataBroker);
      dataConnWidget = NULL;
    }


    MainDataGui::~MainDataGui() {
      if(libManager == NULL) return;

      if(cfg) libManager->releaseLibrary("cfg_manager");
      if(gui) libManager->releaseLibrary("main_gui");
      if(dataBroker) libManager->releaseLibrary("data_broker");
      fprintf(stderr, "Delete MainDataGui\n");
    }


    void MainDataGui::menuAction(int action, bool checked) {
      (void)checked;

      if (gui == NULL)
        return;

      switch(action) {
      case 1:
        if(dataWidget == NULL) {
          dataWidget = new DataWidget(dataBroker, cfg);
          gui->addDockWidget((void*)dataWidget, 0);
          dataWidget->show();
        }
        else {
          gui->removeDockWidget((void*)dataWidget, 0);
          delete dataWidget;
          dataWidget = NULL;
        }
        break;
      case 2:
        if(dataConnWidget == NULL) {
          dataConnWidget = new DataConnWidget(dataBroker, cfg);
          gui->addDockWidget((void*)dataConnWidget, 0);
          dataConnWidget->show();
        }
        else {
          gui->removeDockWidget((void*)dataConnWidget, 0);
          delete dataConnWidget;
          dataConnWidget = NULL;
        }
        break;
      }
    }

    void MainDataGui::timerEvent(QTimerEvent *event) {
      (void)event;
    }

  } // end of namespace: data_broker_gui

} // end of namespace mars

DESTROY_LIB(mars::data_broker_gui::MainDataGui);
CREATE_LIB(mars::data_broker_gui::MainDataGui);
