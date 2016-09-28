/*
 *  Copyright 2015, 2016, DFKI GmbH Robotics Innovation Center
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
 * \file EntityView.cpp
 * \author Malte (malte.langosz@dfki.de)
 * \brief A
 *
 * Version 0.1
 */


#include "EntityView.h"
#include <mars/data_broker/DataBrokerInterface.h>
#include <mars/data_broker/DataPackage.h>

namespace mars {
  namespace plugins {
    namespace EntityView {

      using namespace mars::utils;
      using namespace mars::interfaces;

      EntityView::EntityView(lib_manager::LibManager *theManager)
        : MarsPluginTemplateGUI(theManager, "entity_view") {
      }

      void EntityView::init() {
        view = NULL;
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

        gui->addGenericMenuAction("../Edit/", 0, NULL, 0, "", 0, -1); // separator
        gui->addGenericMenuAction("../Edit/Entity Viewer", 1, this);

      }

      void EntityView::reset() {
      }

      EntityView::~EntityView() {
      }


      void EntityView::update(sReal time_ms) {

        // control->motors->setMotorValue(id, value);
      }

      void EntityView::receiveData(const data_broker::DataInfo& info,
                                    const data_broker::DataPackage& package,
                                    int id) {
        // package.get("force1/x", force);
      }

      void EntityView::cfgUpdateProperty(cfg_manager::cfgPropertyStruct _property) {

        if(_property.paramId == example.paramId) {
          example.dValue = _property.dValue;
        }
      }

      void EntityView::menuAction (int action, bool checked) {
        if(!view) {
          view = new EntityViewMainWindow (control);
          gui->addDockWidget((void*)view, 0);
        }
        else {
          gui->removeDockWidget((void*)view, 0);
          delete view;
          view = NULL;
        }
      }

    } // end of namespace EntityView
  } // end of namespace plugins
} // end of namespace mars

DESTROY_LIB(mars::plugins::EntityView::EntityView);
CREATE_LIB(mars::plugins::EntityView::EntityView);
