/*
 *  Copyright 2016, DFKI GmbH Robotics Innovation Center
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
 * \file PythonMars.h
 * \author Malte (malte.langosz@dfki.de)
 * \brief
 *
 * Version 0.1
 */

#ifndef MARS_PLUGINS_PYTHONMARS_H
#define MARS_PLUGINS_PYTHONMARS_H

#ifdef _PRINT_HEADER_
  #warning "PythonMars.h"
#endif

// set define if you want to extend the gui
#define PLUGIN_WITH_MARS_GUI
#include <mars/interfaces/sim/MarsPluginTemplate.h>
#include <mars/interfaces/MARSDefs.h>
#include <mars/data_broker/ReceiverInterface.h>
#include <mars/cfg_manager/CFGManagerInterface.h>
#include <mars/utils/Mutex.h>
#include <osg_points/Points.hpp>
#include <osg_points/PointsFactory.hpp>
#include <osg_lines/Lines.h>
#include <osg_lines/LinesFactory.h>
#include <mars/osg_material_manager/OsgMaterialManager.h>
#include "PythonInterpreter.hpp"

#include <string>

namespace mars {

  namespace plugins {
    namespace PythonMars {

      struct PointStruct {
        osg_points::Points *p;
        double *data, *pydata;
        int size;
      };

      struct LineStruct {
        osg_lines::Lines *l;
        std::vector<osg_lines::Vector> toAppend;
      };

      struct CameraStruct {
        unsigned long id;
        double *data, *pydata;
        int size;
      };

      // inherit from MarsPluginTemplateGUI for extending the gui
      class PythonMars: public mars::interfaces::MarsPluginTemplateGUI,
        public mars::data_broker::ReceiverInterface,
        // for gui
        public mars::main_gui::MenuInterface,
        public mars::cfg_manager::CFGClient {

      public:
        PythonMars(lib_manager::LibManager *theManager);
        ~PythonMars();

        // LibInterface methods
        int getLibVersion() const
        { return 1; }
        const std::string getLibName() const
        { return std::string("PythonMars"); }
        CREATE_MODULE_INFO();

        // MarsPlugin methods
        void init();
        void reset();
        void update(mars::interfaces::sReal time_ms);

        void interpreteMap(configmaps::ConfigItem &map);
        void interpreteGuiMaps();

        // DataBrokerReceiver methods
        virtual void receiveData(const data_broker::DataInfo &info,
                                 const data_broker::DataPackage &package,
                                 int callbackParam);
        // CFGClient methods
        virtual void cfgUpdateProperty(cfg_manager::cfgPropertyStruct _property);

        // MenuInterface methods
        void menuAction(int action, bool checked = false);

        // PythonMars methods

      private:
        cfg_manager::cfgPropertyStruct example;
        //PythonMars_MainWin *plugin_win;
        utils::Mutex gpMutex, mutex, guiMapMutex, mutexPoints, mutexCamera;
        shared_ptr<Module> plugin;
        std::map<std::string, unsigned long> motorMap;
        configmaps::ConfigItem requestMap;
        bool pythonException;
        std::map<std::string, PointStruct> points;
        std::map<std::string, LineStruct> lines;
        std::map<std::string, CameraStruct> cameras;
        osg_material_manager::OsgMaterialManager *materialManager;
        osg_points::PointsFactory *pf;
        osg_lines::LinesFactory *lf;
        bool updateGraphics, nextStep;
        configmaps::ConfigItem iMap;
        double updateTime;
        std::vector<configmaps::ConfigMap> guiMaps;

        }; // end of class definition PythonMars

    } // end of namespace PythonMars
  } // end of namespace plugins
} // end of namespace mars

#endif // MARS_PLUGINS_PYTHONMARS_H
