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

      Connectors::Connectors(lib_manager::LibManager *theManager)
        : MarsPluginTemplateGUI(theManager, "Connectors") {
      }

      void Connectors::init() {
        const std::map<unsigned long, sim::SimEntity*>* entities = control->entities->subscribeToEntityCreation(this);
        for (std::map<unsigned long, sim::SimEntity*>::const_iterator it = entities->begin(); it != entities->end(); ++it) {
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
              fprintf(stderr, "Adding male connector: %s\n", ((std::string)(tmpmap["name"])).c_str(),
            (unsigned long)(tmpmap["nodeid"]));
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

      bool Connectors::mated(std::string malename, std::string femalename) {
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

      void Connectors::checkForPossibleConnections() {
        std::string malename, femalename, maletype, femaletype;
        for (std::map<std::string, configmaps::ConfigMap>::iterator mit= maleconnectors.begin();
          mit!=maleconnectors.end(); ++mit) {
          malename = (std::string)mit->second["name"];
          maletype = (std::string)mit->second["type"];
          for (std::map<std::string, configmaps::ConfigMap>::iterator fit= femaleconnectors.begin();
            fit!=femaleconnectors.end(); ++fit) {
            femalename = (std::string)fit->second["name"];
            femaletype = (std::string)fit->second["type"];
            if (maletype.compare(femaletype) == 0 && mated(malename, femalename)
          && ((std::string)mit->second["partner"]).empty() && ((std::string)fit->second["partner"]).empty()) {
              connect(malename, femalename);
              }
            }
        }
      }


      void Connectors::update(sReal time_ms) {
        if (cfgautoconnect.bValue) {
          checkForPossibleConnections();
        }
        // the following is experimental and not working yet
        if (cfgbreakable.bValue) {
          for (std::map<std::string, std::string>::iterator it = connections.begin(); it!=connections.end(); ++it) {
            utils::Vector forcevec = control->joints->getSimJoint(maleconnectors[it->first]["jointid"])->getJointLoad();
            //fprintf(stderr, "JointLoad: %g\n", forcevec.norm());
            if (forcevec.norm() > (double)(connectortypes[maleconnectors[it->first]["type"]]["maxforce"])) {
              disconnect(it->first);
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
          checkForPossibleConnections();
        } else if (action == 2) {
          for (std::map<std::string, std::string>::iterator it = connections.begin(); it!=connections.end(); ++it) {
            disconnect(it->first);
          }
        }
      }

    } // end of namespace connectors
  } // end of namespace plugins
} // end of namespace mars

DESTROY_LIB(mars::plugins::connectors::Connectors);
CREATE_LIB(mars::plugins::connectors::Connectors);
