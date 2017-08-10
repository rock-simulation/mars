/*
 *  Copyright 2017, DFKI GmbH Robotics Innovation Center
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
 * \file DataBrokerPlotterLib.hpp
 * \author Malte Langosz
 * \brief 
 **/

#ifndef DATA_BROKER_PLOTTER_LIB_HPP
#define DATA_BROKER_PLOTTER_LIB_HPP

#ifdef _PRINT_HEADER_
#warning "DataBrokerPlotterLib.h"
#endif

#include <mars/main_gui/GuiInterface.h>
#include <mars/main_gui/MenuInterface.h>
#include <lib_manager/LibInterface.hpp>
#include <mars/data_broker/ReceiverInterface.h>
#include <mars/data_broker/DataBrokerInterface.h>
#include <mars/cfg_manager/CFGManagerInterface.h>
#include <mars/cfg_manager/CFGClient.h>
#include <string>

#include <QObject>
#include <QMutex>
#include <QTimerEvent>

namespace data_broker_plotter2 {

  class DataBrokerPlotter;

  class DataBrokerPlotterLib : public QObject,
                           public lib_manager::LibInterface,
                           public mars::main_gui::MenuInterface {

    Q_OBJECT

    public:
    DataBrokerPlotterLib(lib_manager::LibManager* theManager);
    void setupGUI(std::string rPath = std::string("."));
   
    virtual ~DataBrokerPlotterLib(void);
  
    // MenuInterface methods
    virtual void menuAction(int action, bool checked = false);

    // LibInterface methods
    int getLibVersion() const {return 1;}
    const std::string getLibName() const {
      return std::string("data_broker_plotter2");
    }
    CREATE_MODULE_INFO();

    // DataBroker method
    void receiveData(const mars::data_broker::DataInfo &info,
                     const mars::data_broker::DataPackage &dataPackage,
                     int callbackParam);

    void destroyPlotWindow(DataBrokerPlotter*);

  private:
    mars::main_gui::GuiInterface* gui;
    mars::cfg_manager::CFGManagerInterface *cfg;
    mars::data_broker::DataBrokerInterface *dataBroker;
    std::string configPath;
    std::map<std::string, DataBrokerPlotter*> plotWindows;
    std::list<DataBrokerPlotter*> toDelete;
    int num;

  protected slots:
    void timerEvent(QTimerEvent* event);

  };

} // end of namespace: data_broker_plotter2

#endif // DATA_BROKER_PLOTTER_LIB_HPP
