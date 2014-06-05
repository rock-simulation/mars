/*
 *  Copyright 2014, DFKI GmbH Robotics Innovation Center
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
 * \file GeneralPlot.cpp
 * \author Malte (malte.langosz@dfki.de)
 * \brief A
 *
 * Version 0.1
 */


#include "GeneralPlot.h"
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

      GeneralPlot::GeneralPlot(ControlCenter *c)
        : c(c) {

        posTransform = new osg::AutoTransform();
        posTransform->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_SCREEN);
        scaleTransform = new osg::MatrixTransform;
        scaleTransform->setMatrix(osg::Matrix::scale(100., 100., 100.)*
                                  osg::Matrix::translate(-0.1, 0.0, 0.0));
        posTransform->addChild(scaleTransform);
        posTransform->setPosition(osg::Vec3(200.0f, 200.0f, 0.0f));

        hudID = c->graphics->addHUDOSGNode(posTransform.get());

        /*
        scene = static_cast<osg::Group*>(c->graphics->getScene());

        scene->addChild(posTransform);
        */

        plot = new osg_plot::Plot();
        plot->setCullingActive(false);

        scaleTransform->addChild(plot.get());

        c->graphics->addGraphicsUpdateInterface(this);
      }

      void GeneralPlot::addCurve(std::string name) {
        curveVector.push_back(plot->createCurve());
        curveVector.back()->setTitle(name);
        dataVector.push_back(CurveData());
      }

      void GeneralPlot::setYBounds(int curve, double yMin, double yMax) {
        if(curve >= 0 && curve < curveVector.size()) {
            curveVector[curve]->setYBounds(yMin, yMax);
          }
      }

      void GeneralPlot::setPosition(double x, double y, double z) {
        posTransform->setPosition(osg::Vec3(x, y, z));
      }

      void GeneralPlot::setScale(double x, double y) {
        scaleTransform->setMatrix(osg::Matrix::scale(x, y, 1.)*
                                  osg::Matrix::translate(-0.1, 0.0, 0.0));
      }

      void GeneralPlot::addData(int curve, double time, double data) {
        if(curve >= 0 && curve < curveVector.size()) {
          dataVector[curve].time.push_back(time);
          dataVector[curve].data.push_back(data);
        }
      }


      GeneralPlot::~GeneralPlot() {
        c->graphics->removeGraphicsUpdateInterface(this);
        //scene->removeChild(posTransform);
      }


      void GeneralPlot::preGraphicsUpdate() {
        mutex.lock();
        bool updateData = false;
        for(unsigned int i=0; i<dataVector.size(); ++i) {
          std::list<double>::iterator itTime, itData;
          for(itTime=dataVector[i].time.begin(),
                itData=dataVector[i].data.begin();
              itTime!=dataVector[i].time.end(); ++itTime, ++itData) {
            curveVector[i]->appendData(*itTime*0.001, *itData);
            updateData = true;
          }
          dataVector[i].time.clear();
          dataVector[i].data.clear();
        }
        if(updateData) {
          plot->update();
        }
        mutex.unlock();
      }
  
    } // end of namespace Plot3D
  } // end of namespace plugins
} // end of namespace mars
