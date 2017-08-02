#include "DataBrokerPlotter.hpp"
#include "DataBrokerPlotterLib.hpp"
#include <mars/config_map_gui/DataWidget.h>
#include <mars/utils/misc.h>
#include<QVBoxLayout>
#include<QHBoxLayout>
#include<QSplitter>

namespace data_broker_plotter2 {

  enum { CALLBACK_OTHER=0, CALLBACK_NEW_STREAM=-1 };

  DataBrokerPlotter::DataBrokerPlotter(DataBrokerPlotterLib *_mainLib,
                               mars::data_broker::DataBrokerInterface *_dataBroker,
                               mars::cfg_manager::CFGManagerInterface *cfg,
                               std::string _name, QWidget *parent) :
    mars::main_gui::BaseWidget(parent, cfg, _name),
    dataBroker(_dataBroker), mainLib(_mainLib),
    name(_name), nextPlotId(1), updateMap(false), needReplot(false) {

    setStyleSheet("background-color:#eeeeee;");

    qcPlot = new QCustomPlot;

    qcPlot->xAxis2->setVisible(true);
    qcPlot->xAxis2->setTickLabels(false);
    qcPlot->yAxis2->setVisible(true);
    qcPlot->yAxis2->setTickLabels(false);
    qcPlot->setMinimumSize(QSize(100, 100));
    connect(qcPlot->xAxis, SIGNAL(rangeChanged(QCPRange)),
            qcPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(qcPlot->yAxis, SIGNAL(rangeChanged(QCPRange)),
            qcPlot->yAxis2, SLOT(setRange(QCPRange)));
    //qcPlot->setInteraction(QCustomPlot::iSelectPlottables);

    dw = new mars::config_map_gui::DataWidget(cfg, 0, true, false);
    std::vector<std::string> pattern {"*color"};
    dw->setColorPattern(pattern);
    dw->setMaximumWidth(500);
    connect(dw, SIGNAL(valueChanged(std::string, std::string)),
            this, SLOT(valueChanged(std::string, std::string)));

    QVBoxLayout *vLayout = new QVBoxLayout();
    QSplitter *splitter = new QSplitter();
    splitter->addWidget(qcPlot);
    splitter->addWidget(dw);
    vLayout->addWidget(splitter);

    xRange = 10000;
    penSize = 1.0;
    map["Properties"]["X-Range in ms"] = xRange;
    map["Properties"]["Pen Size"] = penSize;
    map["Properties"]["Filter"] = "*/Motors/:*root:";
    filter.push_back("*/Motors/");
    filter.push_back("*root");
    setLayout(vLayout);
    dw->setConfigMap("", map);

    dataBroker->registerSyncReceiver(this, "data_broker", "newStream",
                                     CALLBACK_NEW_STREAM);
    std::vector<mars::data_broker::DataInfo> infoList;
    std::vector<mars::data_broker::DataInfo>::iterator it;
    infoList = dataBroker->getDataList();

    for(it=infoList.begin(); it!=infoList.end(); ++it) {
      if(it->groupName != "data_broker" &&
         it->groupName != "_MESSAGES_") {
        if(it->flags & mars::data_broker::DATA_PACKAGE_READ_FLAG) {
          dataBroker->registerTimedReceiver(this, it->groupName, it->dataName,
                                            "mars_sim/simTimer", 20);
          mars::data_broker::DataPackage package = dataBroker->getDataPackage(it->dataId);
          receiveData(*it, package, 0);
        }
      }
    }
  }

  DataBrokerPlotter::~DataBrokerPlotter(void) {
    dataLock.lock();
    dataBroker->unregisterSyncReceiver(this, "*", "*");
    dataBroker->unregisterTimedReceiver(this, "*", "*", "mars_sim/simTimer");

    delete qcPlot;
  }

  void DataBrokerPlotter::update() {
    static double simTime = 0;
    std::vector<Plot*>::iterator it;
    Plot *p;
    double x;
    int ix;
    double xmin;
    double sTime;

    // first handle panding dataPackages
    dataLock.lock();
    while(!pendingIDs.empty()) {
      std::map<std::string, unsigned long>::iterator it = pendingIDs.begin();
      mars::data_broker::DataPackage package = dataBroker->getDataPackage(it->second);
      packageList.push_back((PackageData){package, it->first});
      pendingIDs.erase(it);
    }
    if(updateMap) {
      dw->updateConfigMap("", map);
      updateMap = false;
    }
    while(!packageList.empty()) {
      mars::data_broker::DataPackage &package = packageList.front().dp;
      if(packageList.front().label == "mars_sim/simTime") {
        package.get(0, &simTime);
      }
      else {
        for(size_t i=0; i<package.size(); ++i) {
          std::string label2 = packageList.front().label + "/" + package[i].getName();

          if(plotMap.find(label2) == plotMap.end()) {
            createNewPlot(label2);
          }
          auto it = plotMap.find(label2);
          if(package[i].type == mars::data_broker::DOUBLE_TYPE) {
            package.get(i, &x);
          }
          else if(package[i].type == mars::data_broker::INT_TYPE) {
            package.get(i, &ix);
            x = (double)ix;
          }
          else {
            continue;
          }
          double xmin = simTime-xRange;
          it->second->xValues.push_back(simTime);
          it->second->yValues.push_back(x);
          while(!it->second->xValues.empty() && it->second->xValues.front() < xmin) {
            it->second->xValues.pop_front();
            it->second->yValues.pop_front();
          }
          needReplot = true;
          it->second->gotData = true;
        }
      }
      packageList.pop_front();
    }
    dataLock.unlock();

    bool onlyEnlarge = false;
    for(auto it: plotMap) {
      if(it.second->curve) {
        if(it.second->gotData) {
          it.second->curve->setData(it.second->xValues, it.second->yValues);
          it.second->gotData = 0;
        }
        it.second->curve->rescaleAxes(onlyEnlarge);
        onlyEnlarge = true;
      }
    }
    if(needReplot) {
      qcPlot->replot();
      needReplot = false;
    }
  }

  void DataBrokerPlotter::receiveData(const mars::data_broker::DataInfo &info,
                                      const mars::data_broker::DataPackage &package,
                                      int callbackParam) {

    dataLock.lock();
    if(callbackParam == CALLBACK_NEW_STREAM) {
      mars::data_broker::DataInfo newInfo;
      package.get("groupName", &newInfo.groupName);
      package.get("dataName", &newInfo.dataName);
      package.get("dataId", (long*)&newInfo.dataId);
      package.get("flags", (int*)&newInfo.flags);
      dataBroker->registerTriggeredReceiver(this, newInfo.groupName, newInfo.dataName,
                                            "");
      if(newInfo.flags & mars::data_broker::DATA_PACKAGE_READ_FLAG) {
        if(newInfo.groupName != "data_broker" &&
           newInfo.groupName != "_MESSAGES_") {
          dataBroker->registerTimedReceiver(this, newInfo.groupName, newInfo.dataName,
                                            "mars_sim/simTimer", 20);
          std::string label = newInfo.groupName + "/" + newInfo.dataName;
          pendingIDs[label] = newInfo.dataId;
        }
      }
    }
    else {
      std::string label = info.groupName + "/" + info.dataName;
      packageList.push_back((PackageData){package, label});
    }
    dataLock.unlock();
  }

  void DataBrokerPlotter::createNewPlot(std::string label) {
    Plot *newPlot = new Plot;

    newPlot->name = label;
    newPlot->gotData = 0;
    newPlot->curve = NULL;

    configmaps::ConfigItem *item;
    std::uniform_real_distribution<double> distribution(0.0,1.0);
    
    newPlot->options["color"]["r"] = distribution(generator);
    newPlot->options["color"]["g"] = distribution(generator);
    newPlot->options["color"]["b"] = distribution(generator);
    newPlot->options["color"]["a"] = 1.0;
    newPlot->options["show"] = false;
    for(auto it: filter) {
      if(mars::utils::matchPattern(it, label)) {
        std::vector<std::string> arrString = mars::utils::explodeString('/', label);
        item = map[arrString[0]];
        arrString.erase(arrString.begin());
        for(auto it: arrString) item = (*item)[it];
        *item = newPlot->options;
        updateMap = true;
        break;
      }
    }
    plotMap[label] = newPlot;
  }

  void DataBrokerPlotter::cfgUpdateProperty(mars::cfg_manager::cfgPropertyStruct _property) {
    mars::main_gui::BaseWidget::cfgUpdateProperty(_property);
  }

  void DataBrokerPlotter::hideEvent(QHideEvent *event) {
    (void)event;

    mainLib->destroyPlotWindow(this);
  }

  void DataBrokerPlotter::valueChanged(std::string key, std::string value) {
    int color = 0;
    double dColor;
    //fprintf(stderr, "value changed %s %s\n", key.c_str(), value.c_str());
    if(key.find("Properties") == 3) {
      if(key.find("Filter") != std::string::npos) {
        configmaps::ConfigMap newMap;
        newMap["Properties"] = map["Properties"];
        map = newMap;
        map["Properties"]["Filter"] = value;
        dataLock.lock();
        filter = mars::utils::explodeString(':', value);
        if(value[value.size()-1] != ':') filter.pop_back();
        for(auto it: plotMap) {
          for(auto it2: filter) {
            if(mars::utils::matchPattern(it2, it.first)) {
              std::vector<std::string> arrString = mars::utils::explodeString('/', it.first);
              configmaps::ConfigItem *item = map[arrString[0]];
              arrString.erase(arrString.begin());
              for(auto it3: arrString) item = (*item)[it3];
              *item = it.second->options;
              break;
            }
          }
        }
        updateMap = true;
        dataLock.unlock();
      }
      else if(key.find("Pen") != std::string::npos) {
        penSize = atof(value.c_str());
        map["Properties"]["Pen Size"] = penSize;
        for(auto it: plotMap) {
          if(it.second->curve) {
            QColor c(255*(double)it.second->options["color"]["r"],
                     255*(double)it.second->options["color"]["g"],
                     255*(double)it.second->options["color"]["b"]);
            it.second->curve->setPen( QPen(c, penSize) );          
            needReplot = true;
          }
        }
      }
      else {
        xRange = atoi(value.c_str());
        map["Properties"]["X-Range in ms"] = xRange;
      }
      return;
    }
    if(key.substr(key.size()-5) == "/show") {
      key = key.substr(3, key.size()-8);
    }
    else {
      if(key[key.size()-1] == 'a') return;
      else if(key[key.size()-1] == 'r') color = 1;
      else if(key[key.size()-1] == 'g') color = 2;
      else if(key[key.size()-1] == 'b') color = 3;
      key = key.substr(3, key.size()-11);
      dColor = atof(value.c_str());
    }
    //fprintf(stderr, "value changed %s %d: %s %g\n", key.c_str(), color, value.c_str(), dColor);
    dataLock.lock();
    auto it = plotMap.find(key);
    if(it != plotMap.end()) {
      if(color == 0) { // changed show
        if(mars::utils::tolower(value) == "true" ||
           value == "1") {
          if(!it->second->curve) {
            it->second->curve = qcPlot->addGraph();
            it->second->curve->setAntialiasedFill(false);
            QColor c(255*(double)it->second->options["color"]["r"],
                     255*(double)it->second->options["color"]["g"],
                     255*(double)it->second->options["color"]["b"]);
            it->second->curve->setPen( QPen(c, penSize) );
            it->second->curve->setLineStyle( QCPGraph::lsLine );
            it->second->options["show"] = true;
            needReplot = true;
          }
        }
        else {
          if(it->second->curve) {
            qcPlot->removeGraph(it->second->curve);
            it->second->curve = NULL;
            it->second->options["show"] = false;
            needReplot = true;
          }
        }
      }
      else { // changed color
        if(color == 1) {
          it->second->options["color"]["r"] = dColor;
        }
        else if(color == 2) {
          it->second->options["color"]["g"] = dColor;
        }
        else if(color == 3) {
          it->second->options["color"]["b"] = dColor;
        }
        if(it->second->curve) {
          QColor c(255*(double)it->second->options["color"]["r"],
                   255*(double)it->second->options["color"]["g"],
                   255*(double)it->second->options["color"]["b"]);
          it->second->curve->setPen( QPen(c, penSize) );          
          needReplot = true;
        }
      }
    }
    dataLock.unlock();
  }

} // end of namespace: data_broker_plotter2
