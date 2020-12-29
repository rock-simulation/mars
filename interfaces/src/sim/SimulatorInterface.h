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
 * \file SimulatorInterface.h
 * \author Malte Langosz
 * \brief "SimulatorInterface" provides an interface for using the Simulator class
 *
 */


#ifndef SIMULATOR_INTERFACE_H
#define SIMULATOR_INTERFACE_H

#ifdef _PRINT_HEADER_
  #warning "SimulatorInterface.h"
#endif

#include "PhysicsInterface.h"
#include "PluginInterface.h"
#include "../sim_common.h"
#include "../graphics/draw_structs.h"
#include "../LightData.h"
#include <mars/utils/Vector.h>

namespace lib_manager {
  class LibManager;
}

namespace mars {

  namespace interfaces {

    class SimulatorInterface {
    public:

      static SimulatorInterface* getInstance(lib_manager::LibManager *libManager);
      virtual ~SimulatorInterface() {}
      
      // controlling the simulation
      virtual void runSimulation(bool startThread = true) = 0;
      virtual void step(bool setState = false) = 0;
      virtual void StartSimulation() = 0;
      virtual void StopSimulation() = 0;
      virtual void resetSim(bool resetGraphics=true) = 0;
      virtual bool isSimRunning() const = 0;
      virtual bool startStopTrigger() = 0;
      virtual void singleStep(void) = 0;
      virtual void newWorld(bool clear_all=false) = 0;
      virtual void exitMars(void) = 0;
      virtual void readArguments(int argc, char **argv) = 0;
      virtual ControlCenter* getControlCenter(void) const = 0;      

      // simulation contents
      virtual void addLight(LightData light) = 0;
      virtual void connectNodes(unsigned long id1, unsigned long id2) = 0;
      virtual void disconnectNodes(unsigned long id1, unsigned long id2) = 0;
      virtual void rescaleEnvironment(sReal x, sReal y, sReal z) = 0;

      // scenes
      virtual int loadScene(const std::string &filename, const std::string &robotname, bool threadsave=false, bool blocking=false) = 0;
      virtual int loadScene(const std::string &filename, bool wasrunning=false,
                        const std::string &robotname = "", bool threadsave=false, bool blocking=false) = 0;
      virtual int loadScene(const std::string &filename, const std::string &robotname, utils::Vector pos, utils::Vector rot, 
                        bool threadsave=false, bool blocking=false, bool wasrunning=false) = 0;
      virtual int saveScene(const std::string &filename, bool wasrunning) = 0;
      /**make sure the string objects exist during the execution of those functions even if they
       * are running in a different thread; it would probably be better to just copy them instead
       * of using references
       */
      virtual void exportScene() const = 0;
      virtual bool sceneChanged() const = 0;
      virtual void sceneHasChanged(bool reset) = 0;

      //threads
      bool allConcurrencysHandled();
      virtual void setSyncThreads(bool value) = 0;
      virtual void physicsThreadLock(void) = 0;
      virtual void physicsThreadUnlock(void) = 0;      

      //physics
      virtual std::shared_ptr<PhysicsInterface> getPhysics(void) const = 0;
      virtual void handleError(PhysicsError error) = 0;
      virtual void setGravity(const utils::Vector &gravity) = 0;
      virtual const utils::Vector& getGravity(void) = 0;
      virtual int checkCollisions(void) = 0;
      virtual bool hasSimFault() const = 0;

      //graphics
      virtual void finishedDraw(void) = 0;
      virtual void allowDraw(void) = 0;
      virtual bool getAllowDraw(void) = 0;
      virtual bool getSyncGraphics(void) = 0;

      //plugins
      virtual void addPlugin(const pluginStruct& plugin) = 0;
      virtual void removePlugin(PluginInterface *pl) = 0;
      virtual void switchPluginUpdateMode(int mode, PluginInterface *pl) = 0;
      virtual void sendDataToPlugin(int plugin_index, void* data) = 0;

      /*
       *  returns the calculated simulation time + the start timestamp
       */
      virtual unsigned long getTime() = 0;

    };


  } // end of namespace interfaces
} // end of namespace mars

#endif  // SIMULATOR_INTERFACE_H
