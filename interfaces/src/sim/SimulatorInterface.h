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

namespace mars {

  namespace lib_manager {
    class LibManager;
  }

  namespace interfaces {

    /** \todo write docs */
    class SimulatorInterface {
    public:

      static SimulatorInterface* getInstance(lib_manager::LibManager *libManager);

      virtual ~SimulatorInterface() {}

      /** \todo write docs */
      virtual void runSimulation() = 0;
      /** \todo write docs */
      virtual void StartSimulation() = 0;
      /** \todo write docs */
      virtual void StopSimulation() = 0;

      /** \todo write docs */
      virtual void addLight(LightData light) = 0;

      /** \todo write docs */
      virtual void exportScene() const = 0;
  
      //saves the actual scene
      /** \todo write docs */
      virtual int saveScene(const std::string &filename, bool wasrunning) = 0;
      /** \todo write docs */
      /**make sure the string objects exist during the execution of those functions even if they
       * are running in a different thread; it would probably be better to just copy them instead
       * of using references
       */
      virtual int loadScene(const std::string &filename, const std::string &robotname) = 0;
      virtual int loadScene(const std::string &filename, bool wasrunning=false,
                            const std::string &robotname = "") = 0;
      /** \todo write docs */
      virtual bool sceneChanged() const = 0;
      /** \todo write docs */
      virtual void sceneHasChanged(bool reseted) = 0;
      /** \todo write docs */
      virtual bool isSimRunning() const = 0;

      /** 
       * \brief start and pause the simulation
       * \return true if started, false if stopped
       */
      virtual bool startStopTrigger() = 0;

      /** \todo write docs */
      virtual void finishedDraw(void) = 0;

      /** \todo write docs */
      virtual void newWorld(bool clear_all=false) = 0;
      /** \todo write docs */
      virtual void resetSim(void) = 0;

      /** \todo write docs */
      virtual void controlSet(unsigned long id, sReal value) = 0;
      /** \todo write docs */
      virtual void physicsThreadLock(void) = 0;
      /** \todo write docs */
      virtual void physicsThreadUnlock(void) = 0;
      /** \todo write docs */
      virtual PhysicsInterface* getPhysics(void) const = 0;
      /** \todo write docs */
      virtual void exitMars(void) = 0;
      /** \todo write docs */
      virtual void connectNodes(unsigned long id1, unsigned long id2) = 0;
      /** \todo write docs */
      virtual void disconnectNodes(unsigned long id1, unsigned long id2) = 0;
      /** \todo write docs */
      virtual void rescaleEnvironment(sReal x, sReal y, sReal z) = 0;
      /** \todo write docs */
      virtual void singleStep(void) = 0;
      /** \todo write docs */
      virtual void switchPluginUpdateMode(int mode, PluginInterface *pl) = 0;
      /** \todo write docs */
      virtual void handleError(PhysicsError error) = 0;
      /** \todo write docs */
      virtual void setGravity(const utils::Vector &gravity) = 0;

      /** \todo write docs */
      virtual ControlCenter* getControlCenter(void) const = 0;
      /** \todo write docs */
      virtual void addPlugin(const pluginStruct& plugin) = 0;
      /** \todo write docs */
      virtual void removePlugin(PluginInterface *pl) = 0;

      /** \todo write docs */
      virtual int checkCollisions(void) = 0;
      /** \todo write docs */
      virtual void sendDataToPlugin(int plugin_index, void* data) = 0;

      /** \brief syncs the threads of gui and simulation */
      virtual void setSyncThreads(bool value) = 0;

      /** \brief used for gui and simulation synchronization */
      virtual void allowDraw(void) = 0;
      /** \todo write docs */
      virtual void readArguments(int argc, char **argv) = 0;

      /** \todo write docs */
      virtual bool getAllowDraw(void) = 0;
      /** \todo write docs */
      virtual bool getSyncGraphics(void) = 0;
    };


  } // end of namespace interfaces
} // end of namespace mars

#endif  // SIMULATOR_INTERFACE_H
