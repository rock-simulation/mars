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
 * \file GeneralPlot.h
 * \author Malte (malte.langosz@dfki.de)
 * \brief A
 *
 * Version 0.1
 */

#ifndef MARS_PLUGINS_GENERAL_PLOT_H
#define MARS_PLUGINS_GENERAL_PLOT_H

#ifdef _PRINT_HEADER_
  #warning "Plot3D.h"
#endif

// set define if you want to extend the gui
#include <mars/interfaces/MARSDefs.h>
#include <mars/interfaces/graphics/GraphicsUpdateInterface.h>
#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/utils/Vector.h>
#include <mars/utils/Mutex.h>

#include <string>

#include <osg_plot/Plot.h>
#include <osg_plot/Curve.h>
#include <osg/AutoTransform>

namespace mars {

  namespace plugins {
    namespace Plot3D {

      class GeneralPlot: public mars::interfaces::GraphicsUpdateInterface {

        class CurveData {
        public:
          std::list<double> time;
          std::list<double> data;
        };

        public:
        GeneralPlot(interfaces::ControlCenter *c);
        ~GeneralPlot();

        virtual void preGraphicsUpdate(void);
        void addCurve(std::string name);
        void addData(int curve, double time, double data);
        void setPosition(double x, double y, double z);
        void setScale(double x, double y);
        void setYBounds(int curve, double yMin, double yMax);

      private:
        interfaces::ControlCenter *c;
        osg::ref_ptr<osg_plot::Plot> plot;
        std::vector<osg_plot::Curve*> curveVector;
        osg::ref_ptr<osg::Group> scene;
        osg::ref_ptr<osg::MatrixTransform> scaleTransform;
        osg::ref_ptr<osg::AutoTransform> posTransform;

        unsigned long motorID, hudID;
        utils::Vector plotPos;
        utils::Mutex mutex;
        std::vector<CurveData> dataVector;

      }; // end of class definition GeneralPlot

    } // end of namespace Plot3D
  } // end of namespace plugins
} // end of namespace mars

#endif // MARS_PLUGINS_GENERAL_PLOT_H
