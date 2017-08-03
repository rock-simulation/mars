#include "DataBrokerPlotter.hpp"
#include "DataBrokerPlotterLib.hpp"
#include <mars/config_map_gui/DataWidget.h>
#include <mars/utils/misc.h>
#include<QVBoxLayout>
#include<QHBoxLayout>
#include<QSplitter>
#include<QPushButton>
#include <QFileDialog>
#include <cstdio>

namespace data_broker_plotter2 {

  enum { CALLBACK_OTHER=0, CALLBACK_NEW_STREAM=-1 };

  DataBrokerPlotter::DataBrokerPlotter(DataBrokerPlotterLib *_mainLib,
                               mars::data_broker::DataBrokerInterface *_dataBroker,
                               mars::cfg_manager::CFGManagerInterface *cfg,
                               std::string _name, QWidget *parent) :
    mars::main_gui::BaseWidget(parent, cfg, _name),
    dataBroker(_dataBroker), mainLib(_mainLib),
    name(_name), nextPlotId(1), updateMap(false), needReplot(false), inReceive(false), exit(false),
    threadRunning(false), simTime(0) {

    setStyleSheet("background-color:#eeeeee;");
    exportPath = cfg->getOrCreateProperty("Config", "config_path", string(".")).sValue;

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
    QWidget *w = new QWidget();
    QVBoxLayout *vLayout2 = new QVBoxLayout();
    vLayout2->addWidget(dw);
    QPushButton *button = new QPushButton("export plot");
    button->setMaximumWidth(500);
    w->setMaximumWidth(500);
    connect(button, SIGNAL(clicked()), this, SLOT(exportPlot()));
    vLayout2->addWidget(button);
    w->setLayout(vLayout2);
    splitter->addWidget(w);
    vLayout->addWidget(splitter);

    xRange = 10000;
    penSize = 1.0;
    dataUpdateRate = 40;
    map["Properties"]["X-Range in ms"] = xRange;
    map["Properties"]["Data Update Rate"] = dataUpdateRate;
    map["Properties"]["Pen Size"] = penSize;
    map["Properties"]["Filter"] = "*/Motors/:*root:";
    filter.push_back("*/Motors/");
    filter.push_back("*root");
    setLayout(vLayout);
    dw->setConfigMap("", map);

    dataBroker->registerSyncReceiver(this, "data_broker", "newStream",
                                     CALLBACK_NEW_STREAM);
    dataBroker->registerTimedReceiver(this, "mars_sim", "simTime",
                                      "mars_sim/simTimer", 0);
    std::vector<mars::data_broker::DataInfo> infoList;
    std::vector<mars::data_broker::DataInfo>::iterator it;
    infoList = dataBroker->getDataList();

    for(it=infoList.begin(); it!=infoList.end(); ++it) {
      if(it->groupName != "data_broker" &&
         it->groupName != "_MESSAGES_") {
        if(it->flags & mars::data_broker::DATA_PACKAGE_READ_FLAG) {
          // dataBroker->registerTimedReceiver(this, it->groupName, it->dataName,
          //                                   "mars_sim/simTimer", 50);
          mars::data_broker::DataPackage package = dataBroker->getDataPackage(it->dataId);
          receiveData(*it, package, 0);
        }
      }
    }
    start();
  }

  DataBrokerPlotter::~DataBrokerPlotter(void) {
    exit = true;
    dataLock.lock();
    dataBroker->unregisterSyncReceiver(this, "*", "*");
    dataBroker->unregisterTimedReceiver(this, "*", "*", "mars_sim/simTimer");
    dataLock.unlock();
    while(inReceive) {
      msleep(10);
    }
    delete qcPlot;
  }

  void DataBrokerPlotter::update() {
    plotLock.lock();
    if(updateMap) {
      dw->updateConfigMap("", map);
      updateMap = false;
    }

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
    plotLock.unlock();
  }

  void DataBrokerPlotter::receiveData(const mars::data_broker::DataInfo &info,
                                      const mars::data_broker::DataPackage &package,
                                      int callbackParam) {
    if(exit) return;
    inReceive = true;
    dataLock.lock();
    if(callbackParam == CALLBACK_NEW_STREAM) {
      mars::data_broker::DataInfo newInfo;
      package.get("groupName", &newInfo.groupName);
      package.get("dataName", &newInfo.dataName);
      package.get("dataId", (long*)&newInfo.dataId);
      package.get("flags", (int*)&newInfo.flags);
      if(newInfo.flags & mars::data_broker::DATA_PACKAGE_READ_FLAG) {
        if(newInfo.groupName != "data_broker" &&
           newInfo.groupName != "_MESSAGES_") {
          // dataBroker->registerTimedReceiver(this, newInfo.groupName, newInfo.dataName,
          //                                   "mars_sim/simTimer", 50);
          std::string label = newInfo.groupName + "/" + newInfo.dataName;
          pendingIDs[label] = newInfo;
        }
      }
    }
    else {
      std::string label = info.groupName + "/" + info.dataName;
      if(label == "mars_sim/simTime") {
        double v;
        package.get(0, &v);
        if(v < simTime) {
          for(auto it: plotMap) {
            it.second->xValues.clear();
            it.second->yValues.clear();
          }
        }
        simTime = v;
      }
      else {
        packageList.push_back({label, simTime, info, package});
      }
    }
    dataLock.unlock();
    inReceive = false;
  }

  void DataBrokerPlotter::createNewPlot(std::string label, const mars::data_broker::DataInfo &info) {
    Plot *newPlot = new Plot;

    newPlot->name = label;
    newPlot->gotData = 0;
    newPlot->curve = NULL;
    newPlot->dataInfo = info;

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
        plotLock.lock();
        configmaps::ConfigMap newMap;
        newMap["Properties"] = map["Properties"];
        map = newMap;
        map["Properties"]["Filter"] = value;
        plotLock.unlock();
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
            plotLock.lock();
            it.second->curve->setPen( QPen(c, penSize) );
            needReplot = true;
            plotLock.unlock();
          }
        }
      }
      else if(key.find("Data") != std::string::npos) {
        plotLock.lock();
        // todo: change already registerd data
        dataUpdateRate = atof(value.c_str());
        plotLock.unlock();
      }
      else {
        plotLock.lock();
        xRange = atoi(value.c_str());
        plotLock.unlock();
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
            plotLock.lock();
            mars::data_broker::DataInfo &info = it->second->dataInfo;
            if(registerMap.find(info.dataId) == registerMap.end()) {
              registerMap[info.dataId] = 1;
              dataBroker->registerTimedReceiver(this, info.groupName, info.dataName,
                                                "mars_sim/simTimer", dataUpdateRate);
            }
            else {
              registerMap[info.dataId] = registerMap[info.dataId]+1;
            }
            it->second->curve = qcPlot->addGraph();
            it->second->curve->setAntialiasedFill(false);
            QColor c(255*(double)it->second->options["color"]["r"],
                     255*(double)it->second->options["color"]["g"],
                     255*(double)it->second->options["color"]["b"]);
            it->second->curve->setPen( QPen(c, penSize) );
            it->second->curve->setLineStyle( QCPGraph::lsLine );
            it->second->options["show"] = true;
            needReplot = true;
            plotLock.unlock();
          }
        }
        else {
          if(it->second->curve) {
            mars::data_broker::DataInfo &info = it->second->dataInfo;
            plotLock.lock();
            int count = registerMap[info.dataId]-1;
            if(count == 0) {
              registerMap.erase(info.dataId);
              dataBroker->unregisterTimedReceiver(this, info.groupName, info.dataName,
                                                  "mars_sim/simTimer");
            }
            else {
              registerMap[info.dataId] = count;
            }
            qcPlot->removeGraph(it->second->curve);
            it->second->curve = NULL;
            it->second->options["show"] = false;
            needReplot = true;
            plotLock.unlock();
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
          plotLock.lock();
          it->second->curve->setPen( QPen(c, penSize) );
          needReplot = true;
          plotLock.unlock();
        }
      }
    }
    dataLock.unlock();
  }

  void DataBrokerPlotter::run() {
    threadRunning = true;
    double x, xmin;
    int ix;

    while(!exit) {
      // first handle panding dataPackages
      dataLock.lock();
      std::vector<PackageData> packageList_;
      packageList.swap(packageList_);

      while(!pendingIDs.empty()) {
        std::map<std::string, mars::data_broker::DataInfo>::iterator it = pendingIDs.begin();
        mars::data_broker::DataPackage package = dataBroker->getDataPackage(it->second.dataId);
        packageList_.push_back({it->first, simTime, it->second, package});
        pendingIDs.erase(it);
      }
      dataLock.unlock();

      plotLock.lock();

      for(auto p: packageList_) {
        mars::data_broker::DataInfo &info = p.di;
        mars::data_broker::DataPackage &package = p.dp;
        if(p.label == "mars_sim/simTime") {

        }
        else {
          for(size_t i=0; i<package.size(); ++i) {
            std::string label2 = p.label + "/" + package[i].getName();

            auto it = plotMap.find(label2);
            if(it == plotMap.end()) {
              createNewPlot(label2, info);
              it = plotMap.find(label2);
            }
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
            xmin = simTime-xRange;
            it->second->xValues.push_back(p.simTime);
            it->second->yValues.push_back(x);
            while(!it->second->xValues.empty() && it->second->xValues.front() < xmin) {
              it->second->xValues.pop_front();
              it->second->yValues.pop_front();
            }
            needReplot = true;
            it->second->gotData = true;
          }
        }
      }
      plotLock.unlock();
      msleep(10);
    }
    threadRunning = false;
  }

  void DataBrokerPlotter::exportPlot() {
    QString folder = QFileDialog::getExistingDirectory(NULL,
                                                       QObject::tr("Select Export Folder"),
                                                       exportPath.c_str());
    if(!folder.isNull()) {
      exportPath = folder.toStdString();
    }
    if(exportPath.back() != '/') exportPath += '/';
    plotLock.lock();
    for(auto p: plotMap) {
      if(!p.second->curve) continue;
      std::string filePath = mars::utils::replaceString(p.second->name, "/", "_");
      filePath = exportPath + filePath + ".csv";
      FILE *file = fopen(filePath.c_str(), "w");
      if(!file) {
        fprintf(stderr, "Error open File: %s\n", filePath.c_str());
        continue;
      }
      for(size_t i=0; i<p.second->xValues.size(); ++i) {
        fprintf(file, "%g %g\n", p.second->xValues[i], p.second->yValues[i]);
      }
      fclose(file);
    }
    plotLock.unlock();
  }

} // end of namespace: data_broker_plotter2
