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
 * \file ControlCenter.h
 * \author Malte Roemmermann
 * \brief "ControlCenter" contains the instances of all modules of the
 * simulation. The modules are connected via interfaces to this class.
 * These interfaces are public and are used by the modules to interact with
 * each other.
 *
 */

#ifndef CONTROL_CENTER_H
#define CONTROL_CENTER_H

#ifdef _PRINT_HEADER_
  #warning "ControlCenter.h"
#endif

#ifndef ROCK
#include <mars/data_broker/DataBrokerInterface.h>
#define LOG_FATAL(...) (mars::interfaces::ControlCenter::theDataBroker->pushFatal(__VA_ARGS__))
#define LOG_ERROR(...) (mars::interfaces::ControlCenter::theDataBroker->pushError(__VA_ARGS__))
#define LOG_WARN(...) (mars::interfaces::ControlCenter::theDataBroker->pushWarning(__VA_ARGS__))
#define LOG_INFO(...) (mars::interfaces::ControlCenter::theDataBroker->pushInfo(__VA_ARGS__))
#define LOG_DEBUG(...) (mars::interfaces::ControlCenter::theDataBroker->pushDebug(__VA_ARGS__))
#endif


// Include stddef for basic defs like NULL
#include <cstddef>

namespace mars {
  namespace main_gui {
    class GuiInterface;
  }

  namespace cfg_manager {
    class CFGManagerInterface;
  }
  
  namespace data_broker {
    class DataBrokerInterface;
  }

  namespace interfaces {
    class ControlCenter;
    class NodeManagerInterface;
    class JointManagerInterface;
    class MotorManagerInterface;
    class ControllerManagerInterface;
    class SensorManagerInterface;
    class SimulatorInterface;
    class EntityManagerInterface;
    class LoadCenter;
    class GraphicsManagerInterface;

    /**
     * The declaration of the ControlCenter.
     *
     */
    class ControlCenter {
    public:
      ControlCenter(){
        sim = NULL;
        cfg = NULL;
        nodes  = NULL;
        joints = NULL;
        motors = NULL;
        controllers = NULL;
        sensors = NULL;
        graphics = NULL;
        dataBroker = NULL;
        loadCenter = NULL;
      }

      cfg_manager::CFGManagerInterface *cfg;
      NodeManagerInterface *nodes;
      JointManagerInterface *joints;
      MotorManagerInterface *motors;
      ControllerManagerInterface *controllers;
      SensorManagerInterface *sensors;
      SimulatorInterface *sim;
      GraphicsManagerInterface *graphics;
      EntityManagerInterface *entities;
      data_broker::DataBrokerInterface *dataBroker;
      LoadCenter *loadCenter;

      static data_broker::DataBrokerInterface *theDataBroker;
    };

  } // end of namespace interfaces
} // end of namespace mars

#endif //CONTROL_CENTER_H
