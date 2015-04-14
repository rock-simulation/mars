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
 * \file __classname__.cpp
 * \author __author__ (__email__)
 * \brief __description__
 *
 * Version 0.1
 */


#include "__classname__.h"
#include <mars/data_broker/DataBrokerInterface.h>
#include <mars/data_broker/DataPackage.h>

namespace mars {
  namespace plugins {
    namespace __namespace__ {

      using namespace mars::utils;
      using namespace mars::interfaces;

      __classname__::__classname__(lib_manager::LibManager *theManager)
        : MarsPluginTemplateGUI(theManager, "__classname__")
      {
	plugin_win = new __classname___MainWin ();
      }
  
      void __classname__::init() {
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

        gui->addGenericMenuAction("../__classname__/entry", 1, this);

      }

      void __classname__::reset() {
      }

      __classname__::~__classname__() {
      }


      void __classname__::update(sReal time_ms) {

        // control->motors->setMotorValue(id, value);
      }

      void __classname__::receiveData(const data_broker::DataInfo& info,
                                    const data_broker::DataPackage& package,
                                    int id) {
        // package.get("force1/x", force);
      }
  
      void __classname__::cfgUpdateProperty(cfg_manager::cfgPropertyStruct _property) {

        if(_property.paramId == example.paramId) {
          example.dValue = _property.dValue;
        }
      }
      
      void __classname__::menuAction (int action, bool checked)
      {
	plugin_win->show ();
      }

    } // end of namespace __namespace__
  } // end of namespace plugins
} // end of namespace mars

DESTROY_LIB(mars::plugins::__namespace__::__classname__);
CREATE_LIB(mars::plugins::__namespace__::__classname__);