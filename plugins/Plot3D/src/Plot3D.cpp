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
        // package.get("force1/x", force);
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
