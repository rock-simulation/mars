/*
 *  Copyright 2013, DFKI GmbH Robotics Innovation Center
 *
 *  This file.is part of the MARS simulation framework.
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
 * \file.ObstacleGenerator.h
 * \author Kai von Szadkowski (kavo01@dfki.de)
 * \brief A
 *
 * Version 0.1
 */

#ifndef MARS_PLUGINS_OBSTACLE_GENERATOR_H
#define MARS_PLUGINS_OBSTACLE_GENERATOR_H

#ifdef _PRINT_HEADER_
  #warning "ObstacleGenerator.h"
#endif

// set define if you want to extend the gui
//#define PLUGIN_WITH_MARS_GUI
#include <mars/interfaces/sim/MarsPluginTemplate.h>
#include <mars/data_broker/ReceiverInterface.h>
#include <mars/cfg_manager/CFGManagerInterface.h>
#include <mars/interfaces/MARSDefs.h>
//#include <mars/common/utils/Vector.h> //we need this for positioning of the obstacle field

#include <string>
#include <math.h>

namespace mars {

  namespace lib_manager {
    class LibManager;
  }

  namespace plugins {
    namespace obstacle_generator {

      // inherit from MarsPluginTemplateGUI for extending the gui
      class ObstacleGenerator: public mars::interfaces::MarsPluginTemplate,
        public mars::data_broker::ReceiverInterface,
        // for gui
        // public mars::main_gui::MenuInterface,
        public mars::cfg_manager::CFGClient {

      public:
        ObstacleGenerator(mars::lib_manager::LibManager *theManager);
        ~ObstacleGenerator();

        // LibInterface methods
        int getLibVersion() const { return 1; }
        const std::string getLibName() const { return std::string("obstacle_generator"); }
        CREATE_MODULE_INFO();

        // MarsPlugin methods
        void init();
        void reset();
        void update(mars::interfaces::sReal time_ms);
        void createObstacleField();
        void clearObstacleField();
        void createObstacle(std::string name, double pos_x, double pos_y, double width, double length, double height);

        // DataBrokerReceiver methods
        virtual void receiveData(const data_broker::DataInfo &info,
                                 const data_broker::DataPackage &package,
                                 int callbackParam);
        // CFGClient methods
        virtual void cfgUpdateProperty(cfg_manager::cfgPropertyStruct _property);

        // MenuInterface methods
        //void menuAction(int action, bool checked = false);

        // ObstacleGenerator methods

      private:
        std::map<std::string, double> params;
        std::map<cfg_manager::cfgParamId, std::string> paramIds;
        std::vector<mars::interfaces::NodeId> oldNodeIDs;
        std::map<std::string, std::string> textures;
        std::map<std::string, bool> bool_params;
        std::map<cfg_manager::cfgParamId, std::string> bool_paramIds;
        double sigma;
        double random_normal_number(double mean, double std, double min, double max);
        double random_number(double min, double max);

      }; // end of class definition ObstacleGenerator

    } // end of namespace obstacle_generator
  } // end of namespace plugins
} // end of namespace mars

#endif // MARS_PLUGINS_OBSTACLE_GENERATOR_H
