/**
 * \file DataBrokerPlotterLib.cpp
 * \author Malte Römmermann
 * \brief
 **/

#include "DataBrokerPlotter.h"
#include "DataBrokerPlotterLib.h"
#include <cstdio>
#include <iostream>

namespace data_broker_plotter {

  using namespace mars::cfg_manager;

  DataBrokerPlotterLib::DataBrokerPlotterLib(mars::lib_manager::LibManager* theManager) :
    mars::lib_manager::LibInterface(theManager), gui(NULL),
    cfg(NULL), dataBroker(NULL), num(0) {

    // setup GUI with default path
    setupGUI("../resources/");
  }

  void DataBrokerPlotterLib::setupGUI(std::string rPath) {

    if(libManager == NULL) return;

    cfg = libManager->getLibraryAs<CFGManagerInterface>("cfg_manager");

    if(cfg) {
      cfgPropertyStruct path;
      path = cfg->getOrCreateProperty("Preferences", "resources_path", rPath);
      rPath = path.sValue;

      path = cfg->getOrCreateProperty("Config", "config_path", string("."));
      configPath = path.sValue;
    } else {
      fprintf(stderr, "******* gui_cfg: couldn't find cfg_manager\n");
    }

    gui = libManager->getLibraryAs<mars::main_gui::GuiInterface>("main_gui");

    if (gui == NULL)
      return;

    dataBroker = libManager->getLibraryAs<mars::data_broker::DataBrokerInterface>("data_broker");

    //path.append("/data_broker_widget/resources/images/data_broker_symbol.png");
    std::string tmpString = configPath;
    tmpString.append("/");
    tmpString.append("DataBrokerPlotter.yaml");
    cfg->loadConfig(tmpString.c_str());

    gui->addGenericMenuAction("../DataBroker/DataBrokerPlotter", 1, this, 0);

    startTimer(20);
  }


  DataBrokerPlotterLib::~DataBrokerPlotterLib() {
    if(libManager == NULL) return;

    std::map<std::string, DataBrokerPlotter*>::iterator itMap;

    // deleting plot windows existing
    for(itMap=plotWindows.begin(); itMap!=plotWindows.end(); ++itMap) {
      gui->removeDockWidget((void*)itMap->second, 0);
      delete itMap->second;
    }

    std::string tmpString = configPath + std::string("/DataBrokerPlotter.yaml");
    cfg->writeConfig(tmpString.c_str(), "DataBrokerPlotter");

    if(cfg) libManager->releaseLibrary("cfg_manager");
    if(gui) libManager->releaseLibrary("main_gui");
    if(dataBroker) libManager->releaseLibrary("data_broker");
    fprintf(stderr, "Delete DataBrokerPlotterLib\n");
  }


  void DataBrokerPlotterLib::menuAction(int action, bool checked) {
    (void)checked;

    if (gui == NULL)
      return;

    switch(action) {
    case 1:
      {
        // search for new name
        std::map<std::string, DataBrokerPlotter*>::iterator it;
        char text[25];
        std::string name;
        do {
          sprintf(text, "plot%d", ++num);
          name = std::string(text);
          it = plotWindows.find(name);
        } while(it != plotWindows.end());
        if(it == plotWindows.end()) {
          DataBrokerPlotter *dataBrokerPlotter;
          dataBrokerPlotter = new DataBrokerPlotter(this, dataBroker, cfg, name);
          gui->addDockWidget((void*)dataBrokerPlotter, 0);
          dataBrokerPlotter->show();
          plotWindows[name] = dataBrokerPlotter;
        }
      }
      break;
    }
  }

  void DataBrokerPlotterLib::timerEvent(QTimerEvent* event) {
    (void)event;
    std::map<std::string, DataBrokerPlotter*>::iterator itMap;

    // handle delete
    std::list<DataBrokerPlotter*>::iterator it;
    for(it=toDelete.begin(); it!=toDelete.end(); ++it) {
      itMap = plotWindows.find((*it)->getName());
      if(itMap != plotWindows.end()) {
        plotWindows.erase(itMap);
      }

      gui->removeDockWidget((void*)(*it), 0);
      delete (*it);
    }
    toDelete.clear();

    // update existing
    for(itMap=plotWindows.begin(); itMap!=plotWindows.end(); ++itMap) {
      itMap->second->update();
    }
  }

  void DataBrokerPlotterLib::destroyPlotWindow(DataBrokerPlotter *plotWindow) {
    toDelete.push_back(plotWindow);
  }

} // end of namespace: data_broker_plotter

DESTROY_LIB(data_broker_plotter::DataBrokerPlotterLib);
CREATE_LIB(data_broker_plotter::DataBrokerPlotterLib);
