/**
 * \file DataBrokerPlotterLib.cpp
 * \author Malte Langosz
 * \brief
 **/

#include "DataBrokerPlotter.hpp"
#include "DataBrokerPlotterLib.hpp"
#include <cstdio>
#include <iostream>

namespace data_broker_plotter2 {

  using namespace mars::cfg_manager;

  DataBrokerPlotterLib::DataBrokerPlotterLib(lib_manager::LibManager* theManager) :
    lib_manager::LibInterface(theManager), gui(NULL),
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

    gui->addGenericMenuAction("../Tools/Plotter", 1, this, 0);
    if(!cfg->getOrCreateProperty("Windows", "Plotter 1/hidden", true).bValue) {
      menuAction(1, 0);
    }

    startTimer(75);
  }


  DataBrokerPlotterLib::~DataBrokerPlotterLib() {
    if(libManager == NULL) return;

    std::map<std::string, DataBrokerPlotter*>::iterator itMap;
    std::map<std::string, DataBrokerPlotter*> pw = plotWindows;

    if(cfg) libManager->releaseLibrary("cfg_manager");
    if(gui) libManager->releaseLibrary("main_gui");
    if(dataBroker) libManager->releaseLibrary("data_broker");
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
        int i=1;
        sprintf(text, "Plotter %d", i);
        while(plotWindows.find(text) != plotWindows.end()) {
          sprintf(text, "Plotter %d", ++i);
        }
        name = text;
        DataBrokerPlotter *dataBrokerPlotter;
        dataBrokerPlotter = new DataBrokerPlotter(this, libManager, dataBroker,
                                                  cfg, name);
        gui->addDockWidget((void*)dataBrokerPlotter, 0);
        dataBrokerPlotter->show();
        plotWindows[name] = dataBrokerPlotter;
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
    }
    toDelete.clear();

    // update existing
    for(itMap=plotWindows.begin(); itMap!=plotWindows.end(); ++itMap) {
      itMap->second->update();
    }
  }

  void DataBrokerPlotterLib::destroyPlotWindow(DataBrokerPlotter *plotWindow) {
    //toDelete.push_back(plotWindow);
    auto itMap = plotWindows.find(plotWindow->getName());
    if(itMap != plotWindows.end()) {
      plotWindows.erase(itMap);
    }
  }

} // end of namespace: data_broker_plotter2

DESTROY_LIB(data_broker_plotter2::DataBrokerPlotterLib);
CREATE_LIB(data_broker_plotter2::DataBrokerPlotterLib);
