/**
 * \file DataBrokerPlotter.hpp
 * \author Malte Langosz
 * \brief
 **/

#ifndef DATA_BROKER_PLOTTER_HPP
#define DATA_BROKER_PLOTTER_HPP

#include "qcustomplot.h"
#include <QPainter>
#include <QCloseEvent>
#include <QMutex>

#include <mars/data_broker/ReceiverInterface.h>
#include <mars/data_broker/DataBrokerInterface.h>
#include <mars/main_gui/BaseWidget.h>
#include <mars/utils/Thread.h>
#include <configmaps/ConfigData.h>

// todo: add config_map_gui add correct layout
//       react on show hide
//       react on color change
//       create random start colors
//         -> replot

// next: add x range setting
//       scale y values (save scale factor to rescale)
//       add y offset (save offset factor for changing offset)

#include <vector>
#include <random>

using namespace std;

namespace mars {
  namespace config_map_gui {
    class DataWidget;
  }
}

namespace data_broker_plotter2 {

  class DataBrokerPlotterLib;

  class Plot {
  public:
    std::string name;
    QCPGraph *curve;
    mars::data_broker::DataInfo dataInfo;
    QVector<double> xValues;
    QVector<double> yValues;
    bool gotData, show;
    QMutex mutex;
    configmaps::ConfigMap options;
  };

  class PackageData {
  public:
    std::string label;
    double simTime;
    mars::data_broker::DataInfo di;
    mars::data_broker::DataPackage dp;
  };

  class DataBrokerPlotter : public mars::main_gui::BaseWidget,
                            public mars::data_broker::ReceiverInterface,
                            public mars::utils::Thread {
    Q_OBJECT;

  public:
    DataBrokerPlotter(DataBrokerPlotterLib *_mainLib,
                      lib_manager::LibManager* theManager,
                      mars::data_broker::DataBrokerInterface *_dataBroker,
                      mars::cfg_manager::CFGManagerInterface *cfg,
                      std::string _name, QWidget* parent = 0);
    ~DataBrokerPlotter();

    // DataBroker method
    void receiveData(const mars::data_broker::DataInfo &info,
                     const mars::data_broker::DataPackage &dataPackage,
                     int callbackParam);
    void cfgUpdateProperty(mars::cfg_manager::cfgPropertyStruct _property);
    void update();
    inline std::string getName() {return name;}

  public slots:
    void valueChanged(std::string key, std::string value);
    void checkChanged(std::string key, bool checked);
    void exportPlot();

  protected:
    void timerEvent(QTimerEvent *event);
    void closeEvent(QCloseEvent *event);
    void run();

  private:
    mars::config_map_gui::DataWidget *dw;
    configmaps::ConfigMap map, loadMap;
    lib_manager::LibManager* libManager;
    mars::data_broker::DataBrokerInterface *dataBroker;
    DataBrokerPlotterLib *mainLib;
    QCustomPlot *qcPlot;
    QMutex dataLock, plotLock;
    std::string name, configPath, exportPath;
    std::vector<PackageData> packageList;
    std::vector<std::string> filter;
    unsigned long xRange;
    int updateFilterTicks;

    std::map<unsigned long, int> registerMap;
    std::map<std::string, Plot*> plotMap;
    std::vector<Plot*> plots;
    std::map<std::string, mars::data_broker::DataInfo> pendingIDs;
    std::map<mars::cfg_manager::cfgParamId, Plot*> cfgParamIdToPlot;

    int nextPlotId;
    bool updateMap, needReplot, inReceive, exit, threadRunning;
    double penSize, dataUpdateRate, simTime;
    std::default_random_engine generator;

    void createNewPlot(std::string label, const mars::data_broker::DataInfo &info);
    void shiftDown( QRect &rect, int offset ) const;
    void showPlot(Plot* plot);
    void hidePlot(Plot* plot);
    void checkMap(std::string key, std::string path, configmaps::ConfigMap &map, bool checked);
    void checkSub(configmaps::ConfigMap &map, std::string path, bool checked);

  };

} // end of namespace: data_broker_plotter2

#endif // DATA_BROKER_PLOTTER_HPP
