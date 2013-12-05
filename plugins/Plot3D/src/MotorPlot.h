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
 * \file MotorPlot.h
 * \author Malte (malte.langosz@dfki.de)
 * \brief A
 *
 * Version 0.1
 */

#ifndef MARS_PLUGINS_MOTOR_PLOT_H
#define MARS_PLUGINS_MOTOR_PLOT_H

#ifdef _PRINT_HEADER_
  #warning "Plot3D.h"
#endif

// set define if you want to extend the gui
#define PLUGIN_WITH_MARS_GUI
#include <mars/interfaces/MARSDefs.h>
#include <mars/data_broker/ReceiverInterface.h>
#include <mars/interfaces/graphics/GraphicsUpdateInterface.h>
#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/utils/Vector.h>
#include <mars/data_broker/DataPackageMapping.h>

#include <string>

#include <osg_plot/Plot.h>
#include <osg_plot/Curve.h>
#include <osg/AutoTransform>

namespace mars {

  namespace plugins {
    namespace Plot3D {

      class MotorPlot: public mars::data_broker::ReceiverInterface,
                       public mars::interfaces::GraphicsUpdateInterface {

        public:
        MotorPlot(interfaces::ControlCenter *c, unsigned long motorID);
        ~MotorPlot();


        // DataBrokerReceiver methods
        virtual void receiveData(const data_broker::DataInfo &info,
                                 const data_broker::DataPackage &package,
                                 int callbackParam);

        virtual void preGraphicsUpdate(void);

      private:
        interfaces::ControlCenter *c;
        osg::ref_ptr<osg_plot::Plot> plot;
        osg_plot::Curve* curve;
        osg::ref_ptr<osg::Group> scene;
        osg::ref_ptr<osg::MatrixTransform> scaleTransform;
        osg::ref_ptr<osg::AutoTransform> posTransform;

        unsigned long motorID;
        utils::Vector plotPos;
        double time, current;
        int haveNewData;
        data_broker::DataPackageMapping dbTimeMapping;
        data_broker::DataPackageMapping dbMotorMapping;
        data_broker::DataPackageMapping dbNodeMapping;

      }; // end of class definition MotorPlot

    } // end of namespace Plot3D
  } // end of namespace plugins
} // end of namespace mars

#endif // MARS_PLUGINS_PLOT3D_H
