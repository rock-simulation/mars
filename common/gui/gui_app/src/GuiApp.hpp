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
 * \file GuiApp.hpp
 * \author Malte Langosz
 *
 */

#ifndef GUI_APP_H
#define GUI_APP_H


#ifdef _PRINT_HEADER_
  #warning "GuiApp.hpp"
#endif

#include <lib_manager/LibInterface.hpp>
#include <mars/cfg_manager/CFGManagerInterface.h>

namespace gui_app {
  void exit_main(int signal);

  class GuiApp {
  public:
    GuiApp();
    virtual ~GuiApp();

    void init(const std::vector<std::string> &arguments);
    lib_manager::LibManager *libManager;
    std::string configDir;
    mars::cfg_manager::CFGManagerInterface *cfg;
  };

} // end of namespace gui_app

#endif // end of namespace GUI_APP_H
