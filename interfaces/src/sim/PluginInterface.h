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
 * \file PluginInterface.h
 * \author Malte Roemmermann
 * \brief "PluginInterface" is an interface to load dynamically Plugin
 *        into the simulation
 */

#ifndef PLUGIN_INTERFACE_H
#define PLUGIN_INTERFACE_H

#ifdef _PRINT_HEADER_
  #warning "PluginInterface.h"
#endif

namespace mars {
  namespace interfaces {

    class ControlCenter;

    /**
     * The interface to load plugin dynamically into the simulation
     *
     */
    class PluginInterface {
    public:
      PluginInterface(ControlCenter *control) { this->control = control; };
      virtual ~PluginInterface(void) {};
      virtual void update(sReal time_ms) = 0;
      virtual void reset(void) = 0;
      virtual void init(void) = 0;
      virtual void handleError(void) {};
      virtual void getSomeData(void* data) {(void)data;};

    protected:
      ControlCenter *control;

    };

    typedef void *pDestroyPlugin(PluginInterface *sp);
    typedef PluginInterface* create_plugin(ControlCenter *control);

    struct pluginStruct {
      std::string name;
      PluginInterface *p_interface;
      pDestroyPlugin *p_destroy;
      double timer, timer_gui;
      int t_count, t_count_gui;
    };

    void destroy_plugin(PluginInterface *sp);

  } // end of namespace interfaces
} // end of namespace mars

#endif  // PLUGIN_INTERFACE_H
