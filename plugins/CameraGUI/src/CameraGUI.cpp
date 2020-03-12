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
 * \file CameraGUI.cpp
 * \author Malte (malte.langosz@dfki.de)
 * \brief A
 *
 * Version 0.1
 */


#include "CameraGUI.hpp"
#include "CameraWidget.hpp"


namespace mars {
  namespace plugins {
    namespace CameraGUI {

      using namespace mars::utils;
      using namespace mars::interfaces;

      CameraGUI::CameraGUI(lib_manager::LibManager *theManager)
        : MarsPluginTemplateGUI(theManager, "CameraGUI") {
      }

      void CameraGUI::init() {
        gui->addGenericMenuAction("../View/Camera Window", 1, this);
      }

      void CameraGUI::menuAction(int action, bool checked) {
        static unsigned long counter = 0;
        std::stringstream ss;
        ss << "CameraWidget_" << ++counter;
        CameraWidget* c = new CameraWidget(control, gui, NULL, ss.str());
        c->setAttribute(Qt::WA_DeleteOnClose, true);
        c->show();
        c->update();
      }

      void CameraGUI::reset() {
      }

      CameraGUI::~CameraGUI() {
      }


      void CameraGUI::update(sReal time_ms) {
      }

    } // end of namespace CameraGUI
  } // end of namespace plugins
} // end of namespace mars

DESTROY_LIB(mars::plugins::CameraGUI::CameraGUI);
CREATE_LIB(mars::plugins::CameraGUI::CameraGUI);
