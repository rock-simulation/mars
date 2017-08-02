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
    QVector<double> xValues;
    QVector<double> yValues;
    bool gotData, show;
    QMutex mutex;
    configmaps::ConfigMap options;
  };

  class PackageData {
  public:
    mars::data_broker::DataPackage dp;
    std::string label;
  };

  class DataBrokerPlotter : public mars::main_gui::BaseWidget,
                            public mars::data_broker::ReceiverInterface {
    Q_OBJECT;

  public:
    DataBrokerPlotter(DataBrokerPlotterLib *_mainLib,
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

  protected:
    void hideEvent(QHideEvent *event);

  private:
    mars::config_map_gui::DataWidget *dw;
    configmaps::ConfigMap map;
    mars::data_broker::DataBrokerInterface *dataBroker;
    DataBrokerPlotterLib *mainLib;
    QCustomPlot *qcPlot;
    QMutex dataLock, plotLock;
    std::string name;
    std::map<std::string, mars::data_broker::DataPackage> packageList;
    std::vector<std::string> filter;
    unsigned long xRange;

    void shiftDown( QRect &rect, int offset ) const;

    std::map<std::string, Plot*> plotMap;
    std::vector<Plot*> plots;
    std::map<std::string, unsigned long> pendingIDs;
    std::map<mars::cfg_manager::cfgParamId, Plot*> cfgParamIdToPlot;

    int nextPlotId;
    bool updateMap, needReplot;
    double penSize;
    std::default_random_engine generator;

    void createNewPlot(std::string label);

  };

} // end of namespace: data_broker_plotter2

#endif // DATA_BROKER_PLOTTER_HPP
