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
 * \file MotorPlot.cpp
 * \author Malte (malte.langosz@dfki.de)
 * \brief A
 *
 * Version 0.1
 */


#include "MotorPlot.h"
#include <mars/data_broker/DataBrokerInterface.h>
#include <mars/data_broker/DataPackage.h>
#include <mars/interfaces/sim/MotorManagerInterface.h>
#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/sim/JointManagerInterface.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>

namespace mars {
  namespace plugins {
    namespace Plot3D {

      using namespace mars::utils;
      using namespace mars::interfaces;

      MotorPlot::MotorPlot(ControlCenter *c, unsigned long motorID)
        : c(c), motorID(motorID) {

        posTransform = new osg::AutoTransform();
        posTransform->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_SCREEN);
        scaleTransform = new osg::MatrixTransform;
        scaleTransform->setMatrix(osg::Matrix::scale(0.2, 0.12, 0.2)*
                                  osg::Matrix::translate(-0.1, 0.0, 0.0));
        posTransform->addChild(scaleTransform);

        scene = static_cast<osg::Group*>(c->graphics->getScene());

        scene->addChild(posTransform);

        plot = new osg_plot::Plot();
        plot->setCullingActive(false);
        curve = plot->createCurve();
        curve->setTitle("current");
        haveNewData = false;

        scaleTransform->addChild(plot.get());

        c->graphics->addGraphicsUpdateInterface(this);

        c->dataBroker->registerTimedReceiver(this, "mars_sim", "simTime",
                                             "mars_sim/simTimer", 10, 1);

        MotorData motorData = c->motors->getFullMotor(motorID);
        std::string groupName, dataName;
        c->motors->getDataBrokerNames(motorID, &groupName, &dataName);
        c->dataBroker->registerTimedReceiver(this, groupName, dataName,
                                             "mars_sim/simTimer", 10, 2);

        c->joints->getDataBrokerNames(motorData.jointIndex,
                                      &groupName, &dataName);
        c->dataBroker->registerTimedReceiver(this, groupName, dataName,
                                             "mars_sim/simTimer", 10, 3);

        dbTimeMapping.add("simTime", &time);
        dbMotorMapping.add("current", &current);
        dbNodeMapping.add("anchor/x", &plotPos.x());
        dbNodeMapping.add("anchor/y", &plotPos.y());
        dbNodeMapping.add("anchor/z", &plotPos.z());
      }


      MotorPlot::~MotorPlot() {
        c->graphics->removeGraphicsUpdateInterface(this);
        c->dataBroker->unregisterTimedReceiver(this, "*", "*", "mars_sim/simTimer");
        scene->removeChild(posTransform);
      }


      void MotorPlot::preGraphicsUpdate() {
        if(haveNewData == 7) {
          curve->appendData(time*0.001, current);
          plot->update();
          posTransform->setPosition(osg::Vec3(plotPos.x(), plotPos.y(), plotPos.z()+0.2));
          haveNewData = 0;
        }
      }

      void MotorPlot::receiveData(const data_broker::DataInfo& info,
                               const data_broker::DataPackage& package,
                               int id) {
        if(id==1) {
          dbTimeMapping.readPackage(package);
          haveNewData |= 1;
        }
        else if(id == 2) {
          dbMotorMapping.readPackage(package);
          haveNewData |= 2;
        }
        else {
          dbNodeMapping.readPackage(package);
          haveNewData |= 4;
        }
      }
  
    } // end of namespace Plot3D
  } // end of namespace plugins
} // end of namespace mars
