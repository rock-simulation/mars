#include "DataBrokerPlotter.h"
#include "DataBrokerPlotterLib.h"

#include<QVBoxLayout>

namespace data_broker_plotter {
  
  DataBrokerPlotter::DataBrokerPlotter(DataBrokerPlotterLib *_mainLib,
                               mars::data_broker::DataBrokerInterface *_dataBroker,
                               mars::cfg_manager::CFGManagerInterface *cfg,
                               std::string _name, QWidget *parent) : 
    mars::main_gui::BaseWidget(parent, cfg, _name),
    dataBroker(_dataBroker), mainLib(_mainLib),
    name(_name), nextPlotId(1) {
  
    setStyleSheet("background-color:#eeeeee;");

    colors[0] = QColor(255, 0, 0);
    colors[1] = QColor(0, 255, 0);
    colors[2] = QColor(0, 0, 255);
    colors[3] = QColor(255, 155, 0);
    colors[4] = QColor(0, 255, 255);
    colors[5] = QColor(255, 0, 255);
    colors[6] = QColor(127, 0, 255);
    colors[7] = QColor(255, 0, 127);

    qcPlot = new QCustomPlot;

    qcPlot->xAxis2->setVisible(true);
    qcPlot->xAxis2->setTickLabels(false);
    qcPlot->yAxis2->setVisible(true);
    qcPlot->yAxis2->setTickLabels(false);
    connect(qcPlot->xAxis, SIGNAL(rangeChanged(QCPRange)),
            qcPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(qcPlot->yAxis, SIGNAL(rangeChanged(QCPRange)),
            qcPlot->yAxis2, SLOT(setRange(QCPRange)));
    //qcPlot->setInteraction(QCustomPlot::iSelectPlottables);

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addWidget(qcPlot);
    setLayout(vLayout);
    createNewPlot();

  }

  DataBrokerPlotter::~DataBrokerPlotter(void) {
    dataLock.lock();
    dataBroker->unregisterSyncReceiver(this, "*", "*");

    delete qcPlot;
  }

  void DataBrokerPlotter::update() {
    std::vector<Plot*>::iterator it;
    Plot *p;
    double x;
    int ix;
    double xmin;
    double sTime, xRange;

    // first handle panding dataPackages
    dataLock.lock();
    while(!packageList.empty()) {
      mars::data_broker::DataPackage &dataPackage = packageList.front().dp;
      int callbackParam = packageList.front().callbackParam;
      for(it=plots.begin(); it!=plots.end(); ++it) {
        if((*it)->dpId == callbackParam/10) {
          p = *it;

          if(dataPackage[0].type == mars::data_broker::DOUBLE_TYPE)
            dataPackage.get(0, &x);
          else if(dataPackage[0].type == mars::data_broker::INT_TYPE) {
            dataPackage.get(0, &ix);
            x = (double)ix;
          }

          if(callbackParam % 10) {
            x = x*p->yScale.dValue+p->yOffset.dValue;
            p->yValues.push_back(x);
            p->gotNewData |= 1;
          }
          else {
            p->xValues.push_back(x);
            if((xRange = fabs(p->xRange.dValue)) < 0.000001)
              xRange = fabs(plots[0]->xRange.dValue);
            if(xRange > 0.0000001) {
              xmin = x-xRange;
              while(!p->xValues.empty() && p->xValues.front() < xmin) {
                p->xValues.pop_front();
                p->yValues.pop_front();
              }
            }
            p->gotNewData |= 2;
          }
          /*
          if((sTime = fabs(p->sTime.dValue)) < 0.000001) {
            sTime = fabs(plots[0]->sTime.dValue);
          }
          if(sTime > 0.00000001) {
            if(x-p->xValues.back() < sTime) {
              dataLock.unlock();
              return;
            }
          }
          */

        
          if(!p->gotData) {
            p->gotData = true;
            createNewPlot();
          }

          break;
        }
      }
      packageList.pop_front();
    }

    bool onlyEnlarge = false;
    for(it=plots.begin(); it!=plots.end(); ++it) {
      if((*it)->gotNewData == 3) {
        (*it)->curve->setData((*it)->xValues, (*it)->yValues);
        (*it)->curve->rescaleAxes(onlyEnlarge);
        onlyEnlarge = true;
        (*it)->gotNewData = 0;
      }
    }
    qcPlot->replot();
    dataLock.unlock();
  }

  void DataBrokerPlotter::receiveData(const mars::data_broker::DataInfo &info,
                                  const mars::data_broker::DataPackage &dataPackage,
                                  int callbackParam) {

    dataLock.lock();

    packageList.push_back((PackageData){dataPackage, callbackParam});

    dataLock.unlock();
  }
  
  void DataBrokerPlotter::createNewPlot() {
    char text[50];
    // create first curve
    std::string cfgName = name;
    std::string tmpString;
    Plot *newPlot = new Plot;
    QCPGraph *newCurve = qcPlot->addGraph();

    newCurve->setAntialiasedFill(false);

    newPlot->dpId = nextPlotId++;
    sprintf(text, "graph%02d", newPlot->dpId);
    newPlot->name = std::string(text);

    cfgName.append("/");
    cfgName.append(newPlot->name);
    cfgName.append("/");

    newCurve->setPen( QPen(colors[(newPlot->dpId-1) % 8], 0.5) );
    newCurve->setLineStyle( QCPGraph::lsLine );

    newPlot->curve = newCurve;
    
    plots.push_back(newPlot);

    // add datapackage for first plot
    mars::data_broker::DataPackage dbPackageX;// = new mars::data_broker::DataPackage;
    mars::data_broker::DataPackage dbPackageY;// = new mars::data_broker::DataPackage;
    dbPackageX.add("xvalue", (double)0.0);
    dbPackageY.add("yvalue", (double)0.0);
    //dbPackage->add("yvalue", (double)0.0);

    tmpString = name;
    tmpString.append("/");
    tmpString.append(text);
    tmpString.append("/xvalue");
    dataBroker->pushData("data_broker_plotter", tmpString,
                         dbPackageX, this,
                         mars::data_broker::DATA_PACKAGE_READ_WRITE_FLAG);
    dataBroker->registerSyncReceiver(this, "data_broker_plotter",
                                     tmpString, newPlot->dpId*10);
    tmpString = name;
    tmpString.append("/");
    tmpString.append(text);
    tmpString.append("/yvalue");
    dataBroker->pushData("data_broker_plotter", tmpString,
                         dbPackageY, this,
                         mars::data_broker::DATA_PACKAGE_READ_WRITE_FLAG);
    dataBroker->registerSyncReceiver(this, "data_broker_plotter",
                                     tmpString, newPlot->dpId*10+1);
    newPlot->gotData = 0;
    
    tmpString = cfgName;
    tmpString.append("sTime");
    newPlot->sTime = cfg->getOrCreateProperty("DataBrokerPlotter",
                                              tmpString.c_str(),
                                              (double)0.0, this);
    tmpString = cfgName;
    tmpString.append("xRange");
    newPlot->xRange = cfg->getOrCreateProperty("DataBrokerPlotter",
                                               tmpString.c_str(),
                                               (double)0.0, this);
    tmpString = cfgName;
    tmpString.append("yScale");
    newPlot->yScale = cfg->getOrCreateProperty("DataBrokerPlotter",
                                               tmpString.c_str(),
                                               (double)1.0, this);
    tmpString = cfgName;
    tmpString.append("yOffset");
    newPlot->yOffset = cfg->getOrCreateProperty("DataBrokerPlotter",
                                                tmpString.c_str(),
                                                (double)0.0, this);

    cfgParamIdToPlot[newPlot->sTime.paramId] = newPlot;
    cfgParamIdToPlot[newPlot->xRange.paramId] = newPlot;
    cfgParamIdToPlot[newPlot->yScale.paramId] = newPlot;
    cfgParamIdToPlot[newPlot->yOffset.paramId] = newPlot;
    newPlot->cfgParamIdProp[newPlot->sTime.paramId] = &newPlot->sTime;
    newPlot->cfgParamIdProp[newPlot->xRange.paramId] = &newPlot->xRange;
    newPlot->cfgParamIdProp[newPlot->yScale.paramId] = &newPlot->yScale;
    newPlot->cfgParamIdProp[newPlot->yOffset.paramId] = &newPlot->yOffset;
  }

  void DataBrokerPlotter::cfgUpdateProperty(mars::cfg_manager::cfgPropertyStruct _property) {
    mars::main_gui::BaseWidget::cfgUpdateProperty(_property);
    
    // get plot
    dataLock.lock();
    std::map<mars::cfg_manager::cfgParamId, Plot*>::iterator it;
    it = cfgParamIdToPlot.find(_property.paramId);
    if(it == cfgParamIdToPlot.end()) {
      dataLock.unlock();
      return;
    }
    Plot *p = it->second;
    std::map<mars::cfg_manager::cfgParamId, mars::cfg_manager::cfgPropertyStruct*>::iterator it2;
    it2 = p->cfgParamIdProp.find(_property.paramId);
    if(it2 == p->cfgParamIdProp.end()) {
      dataLock.unlock();
      return;
    }
    it2->second->dValue = _property.dValue;
    dataLock.unlock();
  }

  void DataBrokerPlotter::hideEvent(QHideEvent *event) {
    (void)event;

    //mainLib->destroyPlotWindow(this);
  }

} // end of namespace: data_broker_plotter
