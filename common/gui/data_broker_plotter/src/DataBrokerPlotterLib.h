/**
 * \file DataBrokerPlotterLib.h
 * \author Malte Römmermann
 * \brief 
 **/

#ifndef DATA_BROKER_PLOTTER_LIB_H
#define DATA_BROKER_PLOTTER_LIB_H

#ifdef _PRINT_HEADER_
#warning "DataBrokerPlotterLib.h"
#endif

#include <mars/main_gui/GuiInterface.h>
#include <mars/main_gui/MenuInterface.h>
#include <mars/lib_manager/LibInterface.h>
#include <mars/data_broker/ReceiverInterface.h>
#include <mars/data_broker/DataBrokerInterface.h>
#include <mars/cfg_manager/CFGManagerInterface.h>
#include <mars/cfg_manager/CFGClient.h>
#include <string>

#include <QObject>
#include <QMutex>
#include <QTimerEvent>

namespace data_broker_plotter {

  class DataBrokerPlotter;

  class DataBrokerPlotterLib : public QObject,
                           public mars::lib_manager::LibInterface,
                           public mars::main_gui::MenuInterface {

    Q_OBJECT

    public:
    DataBrokerPlotterLib(mars::lib_manager::LibManager* theManager);
    void setupGUI(std::string rPath = std::string("."));
   
    virtual ~DataBrokerPlotterLib(void);
  
    // MenuInterface methods
    virtual void menuAction(int action, bool checked = false);

    // LibInterface methods
    int getLibVersion() const {return 1;}
    const std::string getLibName() const {
      return std::string("data_broker_plotter");
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

} // end of namespace: data_broker_plotter

#endif // DATA_BROKER_PLOTTER_LIB_H
