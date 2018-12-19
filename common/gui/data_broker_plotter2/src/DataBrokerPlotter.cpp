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
#include <cstdlib>

namespace data_broker_plotter2 {

  using namespace configmaps;

  enum { CALLBACK_OTHER=0, CALLBACK_NEW_STREAM=-1 };

  DataBrokerPlotter::DataBrokerPlotter(DataBrokerPlotterLib *_mainLib,
                                       lib_manager::LibManager* theManager,
                                       mars::data_broker::DataBrokerInterface *_dataBroker,
                                       mars::cfg_manager::CFGManagerInterface *cfg,
                                       std::string _name, QWidget *parent) :
    mars::main_gui::BaseWidget(parent, cfg, _name),
    libManager(theManager), dataBroker(_dataBroker), mainLib(_mainLib),
    name(_name), nextPlotId(1), updateMap(false), needReplot(false), inReceive(false), exit(false),
    threadRunning(false), simTime(0) {

    setStyleSheet("background-color:#eeeeee;");
    configPath = cfg->getOrCreateProperty("Config", "config_path", string(".")).sValue;
    exportPath = configPath;

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

    dw = new mars::config_map_gui::DataWidget(NULL, 0, true, false);
    std::vector<std::string> pattern {"*color"};
    dw->setColorPattern(pattern);
    pattern[0] = "*";
    dw->setCheckablePattern(pattern);
    dw->setMaximumWidth(500);
    connect(dw, SIGNAL(valueChanged(std::string, std::string)),
            this, SLOT(valueChanged(std::string, std::string)));
    connect(dw, SIGNAL(checkChanged(std::string, bool)),
            this, SLOT(checkChanged(std::string, bool)));

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
    updateFilterTicks = 0;

    map["Properties"]["X-Range in ms"] = 10000UL;
    map["Properties"]["Data Update Rate"] = 40.0;
    map["Properties"]["Pen Size"] = 1.0;
    map["Properties"]["Filter"] = "*/Motors/:*root:";
    if(mars::utils::pathExists(configPath+"/dbplotter2.yml")) {
      configmaps::ConfigMap m = configmaps::ConfigMap::fromYamlFile(configPath+"/dbplotter2.yml");
      if(m.hasKey(name)) {
        loadMap = m[name];
        map["Properties"].appendMap(loadMap["Properties"]);
      }
    }
    xRange = map["Properties"]["X-Range in ms"];
    dataUpdateRate = map["Properties"]["Data Update Rate"];
    penSize = map["Properties"]["Pen Size"];
    filter = mars::utils::explodeString(':', map["Properties"]["Filter"]);
    setLayout(vLayout);
    dw->setConfigMap("", map);
    libManager->acquireLibrary("data_broker");
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
    startTimer(200);
  }

  DataBrokerPlotter::~DataBrokerPlotter(void) {
    fprintf(stderr, "close: %s\n", name.c_str());
    exit = true;
    // todo: handle different windows in plot file
    configmaps::ConfigMap outMap;
    if(mars::utils::pathExists(configPath+"/dbplotter2.yml")) {
      outMap = configmaps::ConfigMap::fromYamlFile(configPath+"/dbplotter2.yml");
    }
    outMap[name] = map;
    outMap.toYamlFile(configPath+"/dbplotter2.yml");
    dataLock.lock();
    dataBroker->unregisterSyncReceiver(this, "*", "*");
    dataBroker->unregisterTimedReceiver(this, "*", "*", "mars_sim/simTimer");
    libManager->releaseLibrary("data_broker");
    dataLock.unlock();
    while(inReceive || threadRunning) {
      msleep(10);
    }
  }

  void DataBrokerPlotter::update() {
    plotLock.lock();
    if(updateMap) {
      dw->setConfigMap("", map);
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

    ConfigItem *item;
    std::uniform_real_distribution<double> distribution(0.0,1.0);
    bool haveOptions = false;
    {
      std::vector<std::string> arrString = mars::utils::explodeString('/', label);
      ConfigItem itemWrapper(loadMap);
      item = itemWrapper;
      size_t i=0;
      while(i<arrString.size() && item->hasKey(arrString[i])) {
        item = (*item)[arrString[i]];
        ++i;
      }
      if(i == arrString.size()) {
        newPlot->options = *item;
        newPlot->options["show"] = (bool)newPlot->options["show"];
        haveOptions = true;
      }
    }
    if(!haveOptions) {
      newPlot->options["color"]["r"] = distribution(generator);
      newPlot->options["color"]["g"] = distribution(generator);
      newPlot->options["color"]["b"] = distribution(generator);
      newPlot->options["color"]["a"] = 1.0;
      newPlot->options["show"] = false;
    }
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
    if((bool)newPlot->options["show"]) {
      showPlot(newPlot);
    }
  }

  void DataBrokerPlotter::cfgUpdateProperty(mars::cfg_manager::cfgPropertyStruct _property) {
    mars::main_gui::BaseWidget::cfgUpdateProperty(_property);
  }

  // void DataBrokerPlotter::hideEvent(QHideEvent *event) {
  //   (void)event;

  //   mainLib->destroyPlotWindow(this);
  // }

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
        updateFilterTicks = 1;
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
            showPlot(it->second);
            plotLock.unlock();
          }
        }
        else {
          if(it->second->curve) {
            plotLock.lock();
            hidePlot(it->second);
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
      // update map
      std::vector<std::string> arrString = mars::utils::explodeString('/', key);
      ConfigMap *item = &map;
      size_t i=0;
      while(i<arrString.size() && item->hasKey(arrString[i])) {
        item = (*item)[arrString[i]];
        ++i;
      }
      if(i == arrString.size()) {
        *item = it->second->options;
      }
    }
    dataLock.unlock();
  }

  void DataBrokerPlotter::checkChanged(std::string key, bool checked) {
    std::string path;
    key = key.substr(3);
    checkMap(key, path, map, checked);
    plotLock.lock();
    dw->updateConfigMap("", map);
    plotLock.unlock();
  }

  void DataBrokerPlotter::checkSub(ConfigMap &map, std::string path, bool checked) {
    std::string path2 = path;
    if(path2.size() > 1) path2 += "/";
    ConfigMap::iterator it;
    for(it=map.begin(); it!=map.end(); ++it) {
      if(it->first != "color" && it->second.isMap()) {
        // todo: toggle checkbox four group in config_map_gui
        std::string gPath = "../";
        gPath += path2 + it->first;
        dw->setGroupChecked(gPath, checked);
        checkSub(it->second, path2+it->first, checked);
      }
      if(it->first == "show") {
        dataLock.lock();
        it->second = checked;
        auto it2 = plotMap.find(path);
        if(it2 != plotMap.end()) {
          if(checked) {
            if(!it2->second->curve) {
              plotLock.lock();
              showPlot(it2->second);
              plotLock.unlock();
            }
          }
          else {
            if(it2->second->curve) {
              plotLock.lock();
              hidePlot(it2->second);
              plotLock.unlock();
            }
          }
        }
        dataLock.unlock();
      }
    }
  }

  void DataBrokerPlotter::checkMap(std::string key, std::string path, ConfigMap &map, bool checked) {
    std::string compare;
    ConfigMap::iterator it;
    for(it=map.begin(); it!=map.end(); ++it) {
      compare = path;
      if(compare.size() > 0) {
        compare += "/";
      }
      compare += it->first;
      if(key.size() >= compare.size()) {
        if(compare == key.substr(0, compare.size())) {
          if(key.size() == compare.size()) {
            // search for all shows in subtree
            checkSub(it->second, compare, checked);
          }
          if(it->second.isMap()) {
            checkMap(key, compare, it->second, checked);
          }
        }
      }
    }
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
    configmaps::ConfigMap infoMap;
    if(!folder.isNull()) {
      exportPath = folder.toStdString();
    }
    if(exportPath.back() != '/') exportPath += '/';
    plotLock.lock();
    for(auto p: plotMap) {
      if(!p.second->curve) continue;
      infoMap[p.second->name] = p.second->options;
      std::string filePath = mars::utils::replaceString(p.second->name, "/", "_");
      infoMap[p.second->name]["file"] = filePath;
      filePath = exportPath + filePath + ".csv";
      FILE *file = fopen(filePath.c_str(), "w");
      if(!file) {
        fprintf(stderr, "Error open File: %s\n", filePath.c_str());
        continue;
      }
      for(long i=0; i<p.second->xValues.size(); ++i) {
        fprintf(file, "%g %g\n", p.second->xValues[i], p.second->yValues[i]);
      }
      fclose(file);
    }
    infoMap.toYamlFile(exportPath+"config.yml");
    std::string resourcesPath = cfg->getOrCreateProperty("Preferences", "resources_path",
                                                          string(".")).sValue;
    std::string copyPath = resourcesPath + "/data_broker_plotter2/plot.py";
    std::string cmd = "cp " + copyPath + " " + exportPath;
    system(cmd.c_str());
    copyPath = resourcesPath + "/data_broker_plotter2/gui.py";
    cmd = "cp " + copyPath + " " + exportPath;
    system(cmd.c_str());
    plotLock.unlock();
  }

  void DataBrokerPlotter::closeEvent(QCloseEvent *e) {
    mainLib->destroyPlotWindow(this);
  }

  void DataBrokerPlotter::showPlot(Plot* plot) {
    mars::data_broker::DataInfo &info = plot->dataInfo;
    if(registerMap.find(info.dataId) == registerMap.end()) {
      registerMap[info.dataId] = 1;
      dataBroker->registerTimedReceiver(this, info.groupName, info.dataName,
                                        "mars_sim/simTimer", dataUpdateRate);
    }
    else {
      registerMap[info.dataId] = registerMap[info.dataId]+1;
    }
    plot->curve = qcPlot->addGraph();
    plot->curve->setAntialiasedFill(false);
    QColor c(255*(double)plot->options["color"]["r"],
             255*(double)plot->options["color"]["g"],
             255*(double)plot->options["color"]["b"]);
    plot->curve->setPen( QPen(c, penSize) );
    plot->curve->setLineStyle( QCPGraph::lsLine );
    plot->options["show"] = true;
    needReplot = true;
  }

  void DataBrokerPlotter::hidePlot(Plot* plot) {
    mars::data_broker::DataInfo &info = plot->dataInfo;
    int count = registerMap[info.dataId]-1;
    if(count == 0) {
      registerMap.erase(info.dataId);
      dataBroker->unregisterTimedReceiver(this, info.groupName, info.dataName,
                                          "mars_sim/simTimer");
    }
    else {
      registerMap[info.dataId] = count;
    }
    qcPlot->removeGraph(plot->curve);
    plot->curve = NULL;
    plot->options["show"] = false;
    needReplot = true;
  }

  void DataBrokerPlotter::timerEvent(QTimerEvent *event) {
    if(updateFilterTicks > 10) {
      updateFilterTicks = 0;
      plotLock.lock();
      std::string value = map["Properties"]["Filter"];
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
    else if(updateFilterTicks) {
      ++updateFilterTicks;
    }
  }

} // end of namespace: data_broker_plotter2
