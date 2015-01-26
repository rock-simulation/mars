/**
 * \file DataBrokerPlotter.h
 * \author Malte Langosz
 * \brief 
 **/

#ifndef DATA_BROKER_PLOTTER_H
#define DATA_BROKER_PLOTTER_H

#ifdef _PRINT_HEADER_
#warning "DataBrokerPlotter.h"
#endif

#include "qcustomplot.h"
#include <QPainter>
#include <QCloseEvent>
#include <QMutex>

#include <mars/data_broker/ReceiverInterface.h>
#include <mars/data_broker/DataBrokerInterface.h>
#include <mars/main_gui/BaseWidget.h>
#include <vector>

using namespace std;

namespace data_broker_plotter {

  class DataBrokerPlotterLib;

  class Plot {
  public:
    std::string name;
    QCPGraph *curve;
    QVector<double> xValues;
    QVector<double> yValues;
    mars::data_broker::DataPackage dpPackage;
    int dpId, gotNewData;
    bool gotData;
    QMutex mutex;
    mars::cfg_manager::cfgPropertyStruct xRange, yScale, sTime, yOffset;
    std::map<mars::cfg_manager::cfgParamId, mars::cfg_manager::cfgPropertyStruct*> cfgParamIdProp;
  };

  class PackageData {
  public:
    mars::data_broker::DataPackage dp;
    int callbackParam;
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

  protected:
    void hideEvent(QHideEvent *event);

  private:
    mars::data_broker::DataBrokerInterface *dataBroker;
    DataBrokerPlotterLib *mainLib;
    QCustomPlot *qcPlot;
    QMutex dataLock;
    std::string name;
    std::list<PackageData> packageList;

    void shiftDown( QRect &rect, int offset ) const;

    std::vector<Plot*> plots;
    std::map<mars::cfg_manager::cfgParamId, Plot*> cfgParamIdToPlot;

    int nextPlotId;

    void createNewPlot();
    QColor colors[8];

  };

} // end of namespace: data_broker_plotter

#endif // DATA_BROKER_PLOTTER_H
