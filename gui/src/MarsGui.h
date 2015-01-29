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

/**
 * \file MarsGui.h
 * \brief Sets up the main gui of the simulation and handles various generic gui options.
 */


#ifndef MARS_GUI_H
#define MARS_GUI_H

#include <mars/interfaces/gui/MarsGuiInterface.h>
#include <mars/cfg_manager/CFGManagerInterface.h>

namespace mars {

  namespace interfaces {
    class ControlCenter;
  }

  namespace main_gui {
    class GuiInterface;
  }

  namespace gui {

    /**
     * \brief Sets up the main gui of the simulation and handles various generic gui options.
     */
    class MarsGui : public interfaces::MarsGuiInterface
                    ,public cfg_manager::CFGClient 
      {

    public:
      MarsGui();
      virtual ~MarsGui();
  
      // LibInterface methods
      int getLibVersion() const {return 1;}
      const std::string getLibName() const {return std::string("mars_gui");}
      //CREATE_MODULE_INFO();

      virtual void cfgUpdateProperty(cfg_manager::cfgPropertyStruct _property);

      void setupGui();

    private:
      interfaces::ControlCenter *control;
      main_gui::GuiInterface *gui;
      cfg_manager::cfgPropertyStruct resourcesPath;
      cfg_manager::cfgPropertyStruct configPath;
      cfg_manager::cfgPropertyStruct cfgW_top, cfgW_left, cfgW_height, cfgW_width;
      cfg_manager::cfgPropertyStruct marsStyle;
      cfg_manager::cfgPropertyStruct dockStyle;
      cfg_manager::cfgPropertyStruct stateNamesProp, dockArea, dockFloat, dockLeft, dockTop, dockWidth, dockHeight;

    };

  } // end of namespace gui
} // end of namespace mars

#endif // MARS_GUI_H
