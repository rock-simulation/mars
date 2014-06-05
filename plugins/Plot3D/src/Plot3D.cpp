/*
 *  Copyright 2013, DFKI GmbH Robotics Innovation Center
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
 * \file Plot3D.cpp
 * \author Malte (malte.langosz@dfki.de)
 * \brief A
 *
 * Version 0.1
 */


#include "Plot3D.h"
#include <mars/data_broker/DataBrokerInterface.h>
#include <mars/data_broker/DataPackage.h>

#include <mars/interfaces/graphics/GraphicsManagerInterface.h>
#include <mars/interfaces/sim/NodeManagerInterface.h>

namespace mars {
  namespace plugins {
    namespace Plot3D {

      using namespace mars::utils;
      using namespace mars::interfaces;

      Plot3D::Plot3D(lib_manager::LibManager *theManager)
        : MarsPluginTemplateGUI(theManager, "Plot3D"), myWidget(NULL) {
      }

      void Plot3D::init() {
        if(gui) {
          //std::string tmp = resourcesPath + "/mars/plugins/connexion_plugin/connexion.png";
          gui->addGenericMenuAction("../Windows/MotorPlot", 1, this);
        }

        // Load a scene file:
        // control->sim->loadScene("some_file.scn");

        // Register for node information:
        /*
          std::string groupName, dataName;
          control->nodes->getDataBrokerNames(id, &groupName, &dataName);
          control->dataBroker->registerTimedReceiver(this, groupName, dataName, "mars_sim/simTimer", 10, 0);
        */

        /* get or create cfg_param
           example = control->cfg->getOrCreateProperty("plugin", "example",
           0.0, this);
        */

        // Create a nonphysical box:

        // Create a camera fixed on the box:

        // Create a HUD texture element:

        //gui->addGenericMenuAction("../Plot3D/entry", 1, this);

        mars::utils::ConfigMap map;
        map = mars::utils::ConfigMap::fromYamlFile("Plot3DConfig.yml", true);
        mars::utils::ConfigVector::iterator it, it2;
        int dataIndex = 2;
        if(map.find("Plots") != map.end()) {
          ConfigVector::iterator it = map["Plots"].begin();
          for(; it!=map["Plots"].end(); ++it) {
            double posX = 0, posY = 0, posZ = 0;
            double scaleX = 100, scaleY = 100;
            if(it->children.find("posX") != it->children.end()) {
              posX = it->children["posX"][0];
            }
            if(it->children.find("posY") != it->children.end()) {
              posY = it->children["posY"][0];
            }
            if(it->children.find("scaleX") != it->children.end()) {
              scaleX = it->children["scaleX"][0];
            }
            if(it->children.find("scaleY") != it->children.end()) {
              scaleY = it->children["scaleY"][0];
            }
            if(it->children.find("Curves") != it->children.end()) {
              fprintf(stderr, "Plot3D::init(): create new plot.   ........ \n");
              GeneralPlot *plot = new GeneralPlot(control);
              plot->setPosition(posX, posY, posZ);
              plot->setScale(scaleX, scaleY);
              int curveIndex = 0;
              for(it2=it->children["Curves"].begin();
                  it2!=it->children["Curves"].end(); ++it2) {
                plot->addCurve((std::string)it2->children["name"][0]);
                if(it2->children.find("yMin") != it2->children.end()) {
                  if(it2->children.find("yMax") != it2->children.end()) {
                    double yMin = it2->children["yMin"][0];
                    double yMax = it2->children["yMax"][0];
                    plot->setYBounds(curveIndex, yMin, yMax);
                    fprintf(stderr, "have curve limits %g %g\n", yMin, yMax);
                  }
                }
                std::string cDataName = (std::string)it2->children["dbItemName"][0];

                if(it2->children.find("NodeName") != it2->children.end()) {
                  std::string nodeName = (std::string)it2->children["NodeName"][0];
                  std::string groupName, dataName;
                  NodeId id = control->nodes->getID(nodeName);
                  control->nodes->getDataBrokerNames(id, &groupName,
                                                     &dataName);
                  control->dataBroker->registerTimedReceiver(this, groupName,
                                                             dataName,
                                                             "mars_sim/simTimer",
                                                             10, dataIndex);
                }
                else {
                  std::string groupName, dataName;
                  groupName = (std::string)it2->children["dbGroupName"][0];
                  dataName = (std::string)it2->children["dbDataName"][0];
                  control->dataBroker->registerTimedReceiver(this, groupName,
                                                             dataName,
                                                             "mars_sim/simTimer",
                                                             10, dataIndex);
                }
                std::string groupName, dataName, itemName;
                if(it2->children.find("timeGroup") != it2->children.end()) {
                  groupName = (std::string)it2->children["timeGroup"][0];
                  dataName = (std::string)it2->children["timeData"][0];
                  itemName = (std::string)it2->children["timeItem"][0];
                }
                else {
                  groupName = "mars_sim";
                  dataName = "simTime";
                  itemName = "simTime";
                }
                std::string timeString = groupName+dataName+itemName;
                int index = dataIndex++;
                if(timeStringMap.find(timeString) == timeStringMap.end()) {
                  control->dataBroker->registerTimedReceiver(this, groupName,
                                                             dataName,
                                                             "mars_sim/simTimer",
                                                             10, dataIndex);
                  timeStringMap[timeString] = dataIndex;
                  timeMap[dataIndex] = TimeMap();
                  timeMap[dataIndex].dataName = itemName;
                  timeMap[dataIndex++].curveList.push_back(index);
                }
                else {
                  timeMap[timeStringMap[timeString]].curveList.push_back(index);
                }

                generalPlotMap[index] = (PlotMapData){plot, curveIndex, 0.0, 0.0, false, false, cDataName};
                ++curveIndex;
              }

            }
          }
        }

        control->graphics->hideCoords();
      }

      void Plot3D::reset() {
      }

      Plot3D::~Plot3D() {
      }


      void Plot3D::preGraphicsUpdate() {
      }

      void Plot3D::update(sReal time_ms) {
      }

      void Plot3D::receiveData(const data_broker::DataInfo& info,
                               const data_broker::DataPackage& package,
                               int id) {
        if(timeMap.find(id) != timeMap.end()) {
          TimeMap &tm = timeMap[id];
          double time;
          package.get(tm.dataName, &time);
          std::list<int>::iterator it = tm.curveList.begin();
          for(; it!=tm.curveList.end(); ++it) {
            PlotMapData &cd = generalPlotMap[*it];
            cd.time = time;
            if(cd.newData) {
              cd.plot->addData(cd.curveIndex, cd.time, cd.data);
              cd.newTime = false;
              cd.newData = false;
            }
            else {
              cd.newTime = true;
            }
          }
        }
        else {
          //dbNodeMapping.readPackage(package);
          PlotMapData &cd = generalPlotMap[id];
          package.get(cd.dataName, &cd.data);
          if(cd.newTime) {
            cd.plot->addData(cd.curveIndex, cd.time, cd.data);
            cd.newTime = false;
            cd.newData = false;
          }
          else {
            cd.newData = true;
          }
        }
      }

      void Plot3D::cfgUpdateProperty(cfg_manager::cfgPropertyStruct _property) {

        if(_property.paramId == example.paramId) {
          example.dValue = _property.dValue;
        }
      }

      void Plot3D::menuAction(int action, bool checked) {
        switch (action) {
        case 1:
          if (!myWidget) {
            myWidget = new MotorPlotConfig(control);
            //      control->gui->addDockWidget((void*)myWidget);
            myWidget->show();
            //myWidget->setGeometry(40, 40, 200, 200);
            connect(myWidget, SIGNAL(hideSignal()), this, SLOT(hideWidget()));
            connect(myWidget, SIGNAL(closeSignal()), this, SLOT(closeWidget()));

            connect(myWidget, SIGNAL(motorSelected(unsigned long)),
                    this, SLOT(motorSelected(unsigned long)));
            connect(myWidget, SIGNAL(addPlot()),
                    this, SLOT(addPlot()));
            connect(myWidget, SIGNAL(removePlot()),
                    this, SLOT(removePlot()));
          }
          else {
            closeWidget();//myWidget->hide();
          }
          break;
        }
      }

      void Plot3D::hideWidget(void) {
        //if (myWidget) myWidget->close();
      }

      void Plot3D::closeWidget(void) {
        if (myWidget) {
          delete myWidget;
          //    control->gui->removeDockWidget((void*)myWidget);
          myWidget = NULL;
        }
      }

      void Plot3D::motorSelected(unsigned long id) {
        motorID = id;
      }

      void Plot3D::addPlot() {
        if(plotMap.find(motorID) == plotMap.end()) {
          plotMap[motorID] = new MotorPlot(control, motorID);
        }
      }

      void Plot3D::removePlot() {
        if(plotMap.find(motorID) != plotMap.end()) {
          delete plotMap[motorID];
          plotMap.erase(motorID);
        }
      }

    } // end of namespace Plot3D
  } // end of namespace plugins
} // end of namespace mars

DESTROY_LIB(mars::plugins::Plot3D::Plot3D);
CREATE_LIB(mars::plugins::Plot3D::Plot3D);
