/*
 *  Copyright 2012, DFKI GmbH Robotics Innovation Center
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
 * \file MantisPlugin.cpp
 * \author Lorenz Quack (lorenz.quack@dfki.de)
 * \brief A MARS plugin to test mantis configurations.
 *
 * Version 0.1
 */


#include "MantisPlugin.h"
#include <mars/data_broker/DataBrokerInterface.h>
#include <mars/data_broker/DataPackage.h>

#include <QString>
#include <QFileDialog>

namespace mars {
  namespace plugins {
    namespace mantis_plugin {

      using namespace mars::interfaces;
      using namespace mars::utils;
      using namespace mars::sim;

      enum MenuAction {
        ACTION_SAVE,
        ACTION_SAVE_AS,
      };

      MantisPlugin::MantisPlugin(lib_manager::LibManager *theManager)
        : MarsPluginTemplate(theManager, "MantisPlugin") {
        gui = theManager->getLibraryAs<main_gui::GuiInterface>("main_gui");
      }
  
      void MantisPlugin::init() {
        gui->addGenericMenuAction("../Mantis/Save Current Configuration",
                                  ACTION_SAVE, this);
        gui->addGenericMenuAction("../Mantis/Save Current Configuration As...",
                                  ACTION_SAVE_AS, this);
        // Load a scene file:
        // control->sim->loadScene("some_file.scn");

        // Register for node information:
        /*
          std::string groupName, dataName;
          control->nodes->getDataBrokerNames(id, &groupName, &dataName);
          control->dataBroker->registerTimedReceiver(this, groupName, dataName, "mars_sim/simTimer", 10, 0);
        */

        // Create a nonphysical box:

        // Create a camera fixed on the box:

        // Create a HUD texture element:

      }

      void MantisPlugin::reset() {
    
      }

      MantisPlugin::~MantisPlugin() {
      }


      void MantisPlugin::update(sReal time_ms) {

        // control->motors->setMotorValue(id, value);
      }

      void MantisPlugin::receiveData(const data_broker::DataInfo& info,
                                    const data_broker::DataPackage& package,
                                    int id) {
        // package.get("force1/x", force);
      }

      void MantisPlugin::saveConfiguration(std::string filename) const {
        if(filename == "")
          filename = "mantis_config.txt";
        FILE *f = fopen(filename.c_str(), "w");
        fprintf(f, "# MANTIS Plugin configuration file\n");
        fprintf(f, control->cfg->writeConfigToString("Constraints",
                                                     mars::cfg_manager::userSave).c_str());
        std::vector<interfaces::core_objects_exchange> motorList;
        control->motors->getListMotors(&motorList);
        std::vector<interfaces::core_objects_exchange>::const_iterator motorListIt;
        unsigned long id;
        double position, torque;
        fprintf(f, "\n\nMotors:\n");
        for(motorListIt = motorList.begin();
            motorListIt != motorList.end(); ++motorListIt) {
          id = motorListIt->index;
          position = control->motors->getActualPosition(id);
          torque = control->motors->getTorque(id);
          fprintf(f,
                  "  - name: %s\n"
                  "    position: %g\n"
                  "    torque: %g\n",
                  motorListIt->name.c_str(), position, torque);
        }

        fclose(f);
      }

      void MantisPlugin::saveConfigurationAs() const {
        QString filename;
        filename = QFileDialog::getSaveFileName(NULL,
                                                "Save MANTIS configuarion...");
        if(!filename.isEmpty())
          saveConfiguration(filename.toStdString());
      }

      void MantisPlugin::menuAction(int action, bool checked) {
        switch(action) {
        case ACTION_SAVE:
          saveConfiguration();
          break;
        case ACTION_SAVE_AS:
          saveConfigurationAs();
          break;
        default:
          LOG_WARN("received unknown menu action callback: %d", action);
          break;
        }
      }
  
    } // end of namespace mantis_plugin
  } // end of namespace plugins
} // end of namespace mars

DESTROY_LIB(mars::plugins::mantis_plugin::MantisPlugin);
CREATE_LIB(mars::plugins::mantis_plugin::MantisPlugin);
