/*
 *  Copyright 2018, DFKI GmbH Robotics Innovation Center
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
 * \file CameraGUI.hpp
 * \author Malte (malte.langosz@dfki.de)
 * \brief A
 *
 * Version 0.1
 */

#ifndef MARS_PLUGINS_CAMERAGUI_HPP
#define MARS_PLUGINS_CAMERAGUI_HPP

#ifdef _PRINT_HEADER_
  #warning "CameraGUI.hpp"
#endif

// set define if you want to extend the gui
//#define PLUGIN_WITH_MARS_GUI
#include <mars/interfaces/sim/MarsPluginTemplateGUI.h>
#include <mars/interfaces/MARSDefs.h>

#include <string>

namespace mars {

  namespace graphics {
    class GraphicsWidget;
  }

  namespace plugins {
    namespace CameraGUI {

      // inherit from MarsPluginTemplateGUI for extending the gui
      class CameraGUI: public mars::interfaces::MarsPluginTemplateGUI,
        // for gui
        public mars::main_gui::MenuInterface {

      public:
        CameraGUI(lib_manager::LibManager *theManager);
        ~CameraGUI();

        // LibInterface methods
        int getLibVersion() const
        { return 1; }
        const std::string getLibName() const
        { return std::string("CameraGUI"); }
        CREATE_MODULE_INFO();

        // MarsPlugin methods
        void init();
        void reset();
        void update(mars::interfaces::sReal time_ms);

        // MenuInterface methods
        void menuAction(int action, bool checked = false);

        // CameraGUI methods

      }; // end of class definition CameraGUI

    } // end of namespace CameraGUI
  } // end of namespace plugins
} // end of namespace mars

#endif // MARS_PLUGINS_CAMERAGUI_H
