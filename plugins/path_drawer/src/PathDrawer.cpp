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
 * \file PathDrawer.cpp
 * \author Alexander (alexander.dettmann@dfki.de)
 * \brief draws
 *
 * Version 0.1
 */


#include "PathDrawer.h"
#include <mars/data_broker/DataBrokerInterface.h>
#include <mars/data_broker/DataPackage.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>
#include <osg_lines/LinesFactory.h>
 
#include <fstream>

namespace mars {
  namespace plugins {
    namespace path_drawer {

      using namespace mars::utils;
      using namespace mars::interfaces;
      using namespace std;

      PathDrawer::PathDrawer(lib_manager::LibManager *theManager)
        : MarsPluginTemplate(theManager, "PathDrawer") {
      }
  
      void PathDrawer::init() {

        // read the points from an obj file and pass them to the OSG Line Interface
        osg_lines::LinesFactory lF;
        osg_lines::Lines *l = lF.createLines();
        ifstream file;
        file.open ("blender_export_waypoints.obj");
        if (file.is_open()){
          string line;
          while ( getline (file, line) ) {
            //printf("%s\n", line.c_str());
            if(int start = line.find("v ", 0, 2) != string::npos){
              double v[3];
              start += 1;
              int end_first = line.find(" ", start) + 1;
              int end_second = line.find(" ", end_first) + 1;
              v[0] = (sReal) atof((line.substr(start, end_first-start)).c_str());
              v[1] = (sReal) atof((line.substr(end_first, end_second-end_first)).c_str());
              v[2] = (sReal) atof((line.substr(end_second).c_str()));
              l->appendData(osg_lines::Vector(v[0], v[1], v[2]));
            }
          }
          file.close();
        }else{
          printf("[PathDrawer] Error: Unable to open file\n"); 
        }

        l->setColor(osg_lines::Color(0.0, 1.0, 0.0, 1.0));
        l->setLineWidth(4);
        control->graphics->addOSGNode(l->getOSGNode());
        control->graphics->hideCoords();
      }

      void PathDrawer::reset() {
      }

      PathDrawer::~PathDrawer() {
      }


      void PathDrawer::update(sReal time_ms) {

        // control->motors->setMotorValue(id, value);
      }

      void PathDrawer::receiveData(const data_broker::DataInfo& info,
                                    const data_broker::DataPackage& package,
                                    int id) {
        // package.get("force1/x", force);
      }
  
      void PathDrawer::cfgUpdateProperty(cfg_manager::cfgPropertyStruct _property) {

        if(_property.paramId == example.paramId) {
          example.dValue = _property.dValue;
        }
      }

    } // end of namespace path_drawer
  } // end of namespace plugins
} // end of namespace mars

DESTROY_LIB(mars::plugins::path_drawer::PathDrawer);
CREATE_LIB(mars::plugins::path_drawer::PathDrawer);
