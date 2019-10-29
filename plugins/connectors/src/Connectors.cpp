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
 * \file Connectors.cpp
 * \author Kai (kai.von-szadkowski@uni-bremen.de)
 * \brief Allows
 *
 * Version 0.1
 */


#include "Connectors.h"
#include <mars/data_broker/DataBrokerInterface.h>
#include <mars/data_broker/DataPackage.h>
#include <mars/interfaces/sim/EntityManagerInterface.h>
#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/sim/JointManagerInterface.h>
#include <mars/sim/SimEntity.h>
#include <mars/sim/SimJoint.h>
#include <mars/utils/mathUtils.h>

namespace mars {
  namespace plugins {
    namespace connectors {

      using namespace mars::utils;
      using namespace mars::interfaces;

      // To manage thread shared resources.
      std::atomic<bool> isDisconnectionTriggered(false);

      Connectors::Connectors(lib_manager::LibManager *theManager) :
        MarsPluginTemplateGUI(theManager, "Connectors"),
        mars::utils::Thread() {
      }

      void Connectors::init() {

        const std::map<unsigned long, sim::SimEntity*>* entities = control->entities->subscribeToEntityCreation(this);
        for (std::map<unsigned long, sim::SimEntity*>::const_iterator it = entities->begin(); it != entities->end(); ++it) {
          // Regsiter the entity.
          registerEntity(it->second);
        }

        if(control->cfg) {
          cfgautoconnect = control->cfg->getOrCreateProperty("Connectors", "autoconnect", false, this);
          cfgbreakable = control->cfg->getOrCreateProperty("Connectors", "breakable", false, this);
        }
        else {
          cfgautoconnect.bValue = false;
          cfgbreakable.bValue = false;
        }

        gui->addGenericMenuAction("../Control/", 0, NULL, 0, "", 0, -1); // separator
        gui->addGenericMenuAction("../Control/Connect available connectors", 1, this);
        gui->addGenericMenuAction("../Control/Disconnect all connectors", 2, this);
        //maleconnectors.clear();
        //femaleconnectors.clear();

        // Start thread loop.
        // This tread will diconnect connections when they are triggered by the user from the Control GUI.
        // At the same time, it prevents the plugin' update() method from invoking checkForPossibleConnections().
        start();

        LOG_INFO("Connectors Plugin: init was successful.");

      }

      void Connectors::run() {
        // This thread is an infinite loop that checks if connections need to be disconnected and proceesds in
        // disconnecting them if they do.
        // This logic essentially prevents the update() method from invoking checkForPossibleConnections().'
        while(true){

          // Has disconnection been triggered by the user via the Control GUI?
          if(isDisconnectionTriggered.load()){

            // If so, then disconnect all connections.
            for (std::map<std::string, std::string>::iterator it = connections.begin(); it!=connections.end(); ++it) {
               disconnect(it->first);
            }

            // Sleep for a bit to allow time for disconnects to physically detach by falling far enough so that a
            // reconnection is not just retriggered automatically if autoconnect is set to true.
            msleep(300);

            // Set atomic boolean to indicate that we are done with the disconnection.
            // The plugin's update() call can continue invoking checkForPossibleConnections().
            isDisconnectionTriggered = false;

          }else{

            // Sleep for a bit before checking again if a disconnection was triggered by the user.
            msleep(100);
          }
        }
      }

      void Connectors::connect(std::string male, std::string female) {
        fprintf(stderr, "Create connection: %s, %s\n", male.c_str(), female.c_str());
        interfaces::JointData jointdata;
        jointdata.nodeIndex1 = control->nodes->getID((std::string)(maleconnectors[male]["link"]));
        jointdata.nodeIndex2 = control->nodes->getID((std::string)(femaleconnectors[female]["link"]));
        jointdata.type = interfaces::JOINT_TYPE_FIXED;
        jointdata.name = "connector_"+male+"_"+female;
        unsigned long jointid = control->joints->addJoint(&jointdata);
        if (jointid > 0) {
          // TODO: maybe update NodeData here and set collision group differently to avoid collision problems
          maleconnectors[male]["jointid"] = jointid;
          maleconnectors[male]["partner"] = female;
          femaleconnectors[female]["jointid"] = jointid;
          femaleconnectors[female]["partner"] = male;
          connections[male] = female;
        }
      }

      // This ia a shared resources.
      // When invokating, make sure to wrap with !isDisconnectionTriggered.load().
      // The method itself is not wrapped because more often than not it is contained in a for loop
      // so it is best to wrap the for loop rather than every invokation of this method within the for loop.
      void Connectors::disconnect(std::string connector) {
        configmaps::ConfigMap* conmap = NULL;
        unsigned long jointid = 0;
        std::string partner = "";
        bool male = false;
        std::map<std::string, configmaps::ConfigMap>::iterator it = maleconnectors.find(connector);
        if (it != maleconnectors.end()) {
          conmap = &(it->second);
          jointid = (unsigned long)((*conmap)["jointid"]);
          partner = (std::string)((*conmap)["partner"]);
          male = true;
          connections.erase(connector);
        } else {
          it = femaleconnectors.find(connector);
          if (it != maleconnectors.end()) {
            conmap = &(it->second);
            jointid = (unsigned long)((*conmap)["jointid"]);
            partner = (std::string)((*conmap)["partner"]);
            male = false;
            connections.erase(partner);
          }
        }
        if (jointid != 0) {
          control->joints->removeJoint(jointid); // if no match is found, this tries to delete joint id 0
          (it->second)["jointid"] = 0;
          (it->second)["partner"] = "";
          if (male) {
            it = femaleconnectors.find(partner);
          } else {
            it = maleconnectors.find(partner);
          }
          // now reset partner
          (it->second)["jointid"] = 0;
          (it->second)["partner"] = "";
        }
      }

      void Connectors::registerEntity(sim::SimEntity* entity) {

        configmaps::ConfigMap entitymap = entity->getConfig();
        if (entitymap.hasKey("connectors")) {

          configmaps::ConfigMap tmpmap;

          // gather types
          configmaps::ConfigVector typevec = entitymap["connectors"]["types"];

          for (configmaps::ConfigVector::iterator it = typevec.begin(); it!= typevec.end(); ++it) {
            tmpmap = (*it);
            connectortypes[(*it)["name"]] = tmpmap; // the one last read in determines the type
          }

          // gather connectors
          configmaps::ConfigVector convec = entitymap["connectors"]["connectors"];
          for (configmaps::ConfigVector::iterator it = convec.begin(); it!= convec.end(); ++it) {
            tmpmap = (*it);
            tmpmap["nodeid"] = control->nodes->getID((*it)["link"]);
            if (((std::string)((*it)["gender"])).compare("male") == 0) { // male
              maleconnectors[(std::string)(tmpmap["name"])] =  tmpmap;
              fprintf(stderr, "Adding male connector: %s\n", ((std::string)(tmpmap["name"])).c_str(), (unsigned long)(tmpmap["nodeid"]));
            } else { // female
              femaleconnectors[(std::string)(tmpmap["name"])] = tmpmap;
              fprintf(stderr, "Adding female connector: %s\n", ((std::string)(tmpmap["name"])).c_str());
            }
          }
        }
      }

      void Connectors::reset() {
      }

      Connectors::~Connectors() {
      }

      bool Connectors::closeEnough(std::string malename, std::string femalename) {
        unsigned long maleid = maleconnectors[malename]["nodeid"];
        unsigned long femaleid = femaleconnectors[femalename]["nodeid"];
        utils::Vector malerot = control->nodes->getRotation(maleid)*Vector(1.0, 0.0, 0.0);
        utils::Vector femalerot = control->nodes->getRotation(femaleid)*Vector(1.0, 0.0, 0.0);
        sReal angle = utils::angleBetween(malerot, femalerot);

        //TODO: this is a hard-coded hack, should be defined in type
        sReal distance = utils::distanceBetween(control->nodes->getPosition(maleid),
          control->nodes->getPosition(femaleid));
        //fprintf(stderr, "distance/angle: %i, %i, %g/%g\n", maleid, femaleid, distance, angle);
        return (distance <= ((double)(connectortypes[maleconnectors[malename]["type"]]["distance"]))
          && angle < ((double)(connectortypes[maleconnectors[malename]["type"]]["angle"])));

      }

      void Connectors::checkForPossibleConnections(bool isforced) {
        // Connector properties.
        std::string malename, femalename, maletype, femaletype, maleautoconnect, femaleautoconnect;

        // Merge type to check against.
        std::string automatic ("automatic");

        // Nested for loop to check for every male-female connector mating combination.
        for (std::map<std::string, configmaps::ConfigMap>::iterator mit= maleconnectors.begin();
          mit!=maleconnectors.end(); ++mit) {

          // Get male connector properties.
          malename = (std::string)mit->second["name"];
          maletype = (std::string)mit->second["type"];
          maleautoconnect = (std::string)mit->second["mating"];

          for (std::map<std::string, configmaps::ConfigMap>::iterator fit= femaleconnectors.begin();
            fit!=femaleconnectors.end(); ++fit) {

            // Get female connector properties.
            femalename = (std::string)fit->second["name"];
            femaletype = (std::string)fit->second["type"];
            femaleautoconnect = (std::string)fit->second["mating"];

            // There are 2 cases that will trigger checking for connections:
            //  1. When it is forced from the Control GUI: Control > Connect available connectors
            //  2. During plugin update(): IF autoconnect is globally set to true OR IF male and female's mating properties are both set to automatic.
            if(isforced || (cfgautoconnect.bValue || (maleautoconnect.compare(automatic) == 0 && femaleautoconnect.compare(automatic) == 0))){

              // Check if connectors meet the mating requirements:
              //  1. They are of the  same type.
              //  2. They are close enough to each other (distance and angle) as per the set thresholds in the model's YML config file.
              //  3. They are not already connected to each other.
              if (maletype.compare(femaletype) == 0 && closeEnough(malename, femalename)
                && ((std::string)mit->second["partner"]).empty() && ((std::string)fit->second["partner"]).empty()) {

                  // All mating requirements have been met. Mate the connectors.
                  connect(malename, femalename);
              }
            }
          }
        }
      }

      void Connectors::update(sReal time_ms) {

        if(!isDisconnectionTriggered.load()){
            checkForPossibleConnections(false);
        }

        // the following is experimental and not working yet
        if (cfgbreakable.bValue) {
          if(!isDisconnectionTriggered.load()){
            for (std::map<std::string, std::string>::iterator it = connections.begin(); it!=connections.end(); ++it) {
              utils::Vector forcevec = control->joints->getSimJoint(maleconnectors[it->first]["jointid"])->getJointLoad();
              //fprintf(stderr, "JointLoad: %g\n", forcevec.norm());
              if (forcevec.norm() > (double)(connectortypes[maleconnectors[it->first]["type"]]["maxforce"])) {
                disconnect(it->first);
              }
            }
          }
        }
      }

      void Connectors::receiveData(const data_broker::DataInfo& info,
                                    const data_broker::DataPackage& package,
                                    int id) {
        // package.get("force1/x", force);
      }

      void Connectors::cfgUpdateProperty(cfg_manager::cfgPropertyStruct _property) {
        if(_property.paramId == cfgautoconnect.paramId) {
          cfgautoconnect.bValue = _property.bValue;
        } else if(_property.paramId == cfgbreakable.paramId) {
          cfgbreakable.bValue = _property.bValue;
        }
      }

      void Connectors::menuAction(int action, bool checked) {
        if(action == 1) {
          checkForPossibleConnections(true);

        } else if (action == 2) {
          // Use atomic boolean to flag the thread that it should disconnect connections.
          // Setting this atomic boolean to true will also prevent the plugin's update method from invoking checkForPossibleConnections().
          isDisconnectionTriggered = true;
        }
      }

    } // end of namespace connectors
  } // end of namespace plugins
} // end of namespace mars

DESTROY_LIB(mars::plugins::connectors::Connectors);
CREATE_LIB(mars::plugins::connectors::Connectors);
