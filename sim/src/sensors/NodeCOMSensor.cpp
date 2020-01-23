/*
 *  Copyright 2011, 2012, DFKI GmbH Robotics Innovation Center
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

/*
 *  NodeCOMSensor.cpp
 *  QTVersion
 *
 *  Created by Malte Römmerann
 *
 */

#include "NodeCOMSensor.h"
#include <mars/interfaces/sim/NodeManagerInterface.h>

#include <cstdio>
#include <cstdlib>

namespace mars {
  namespace sim {

    using namespace utils;
    using namespace interfaces;

    BaseSensor* NodeCOMSensor::instanciate(ControlCenter *control,
                                           BaseConfig *config ){
  
      IDListConfig *cfg = dynamic_cast<IDListConfig*>(config);
      assert(cfg);
      return new NodeCOMSensor(control,*cfg);
    }

    NodeCOMSensor::NodeCOMSensor(ControlCenter* control, IDListConfig config):
      NodeArraySensor(control, config, false, false) {
      typeName = "NodeCOM";
      linkId = INVALID_ID;
      if (config.config.hasKey("link")){
        linkId = control->nodes->getID((std::string) config.config["link"]);
      }
      if (control->dataBroker) {
        setupDataPackageMapping();
        control->dataBroker->registerTimedProducer(this, "mars_sim", "Sensors",
                                                   "mars_sim/simTimer", 0);
      }
    }

    NodeCOMSensor::~NodeCOMSensor(){
      if (control->dataBroker) {
        control->dataBroker->unregisterTimedProducer(this, "mars_sim", "Sensors",
                                                     "mars_sim/simTimer");
      }
    }

    // this function should be overwritten by the special sensor to
    int NodeCOMSensor::getAsciiData(char* data) const {
      Vector center = control->nodes->getCenterOfMass(config.ids);
      if (linkId != INVALID_ID) {
        center = center - control->nodes->getPosition(linkId);
        center = control->nodes->getRotation(linkId).inverse() * center;
      }

      sprintf(data, " %6.2f %6.2f %6.2f", center.x(), center.y(), center.z());
      return 21;
    }

    int NodeCOMSensor::getSensorData(sReal** data) const {
      Vector center = control->nodes->getCenterOfMass(config.ids);
      if (linkId != INVALID_ID) {
        center = center -control->nodes->getPosition(linkId);
        center = control->nodes->getRotation(linkId).inverse() * center;
      }

      *data = (sReal*)calloc(3, sizeof(sReal));
      (*data)[0] = center.x();
      (*data)[1] = center.y();
      (*data)[2] = center.z();
      return 3;
    }

    /* this procedure is alled every time_step and sends the data to the DataBroker */
    void NodeCOMSensor::produceData(const data_broker::DataInfo &info,
                               data_broker::DataPackage *dbPackage,
                               int callbackParam) {

      com = control->nodes->getCenterOfMass(config.ids);
      if (linkId != INVALID_ID) {
        com = com - control->nodes->getPosition(linkId);
        com = control->nodes->getRotation(linkId).inverse() * com;
      }
      dbPackageMapping.writePackage(dbPackage);
    }

    /* this procedure defines how the data is send to the DataBroker */
    void NodeCOMSensor::setupDataPackageMapping() {
      dbPackageMapping.clear();

      dbPackageMapping.add(config.name+"/com/x", &com.x());
      dbPackageMapping.add(config.name+"/com/y", &com.y());
      dbPackageMapping.add(config.name+"/com/z", &com.z());
    }

  } // end of namespace sim
} // end of namespace mars
