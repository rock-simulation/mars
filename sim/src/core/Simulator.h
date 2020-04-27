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
 * \file Simulator.h
 * \author Malte Langosz
 * \brief "Simulator" is the main class of the simulation
 *
 */

#ifndef SIMULATOR_H
#define SIMULATOR_H

#ifdef _PRINT_HEADER_
  #warning "Simulator.h"
#endif

#include <mars/data_broker/DataPackage.h>
#include <mars/data_broker/ReceiverInterface.h>
#include <mars/cfg_manager/CFGManagerInterface.h>
#include <mars/utils/Thread.h>
#include <mars/utils/Mutex.h>
#include <mars/utils/WaitCondition.h>
#include <mars/utils/ReadWriteLock.h>
#include <mars/interfaces/sim/SimulatorInterface.h>
#include <mars/interfaces/sim/PhysicsInterface.h>
#include <mars/interfaces/sim/PluginInterface.h>
#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/interfaces/graphics/GraphicsUpdateInterface.h>

#include <iostream>


namespace mars {
  namespace sim {

    /**
     *\brief The Simulator class implements the main functions of the MARS simulation.
     *
     * Its constructor presents the core function in the simulation and directly takes the arguments
     * given by the user if the simulation is starting from a command line.
     * It inherits the mars::Thread \c class and the i_GuiToSim
     * A Simulator object presents a separate thread and shares data with all other threads within the process.(remarque)
     * To handle and access the data of the Simulator properly, the mutex variable \c coreMutex is used.
     *
     */
    class Simulator : public utils::Thread,
                      public interfaces::SimulatorInterface,
                      public interfaces::GraphicsUpdateInterface,
                      public lib_manager::LibInterface,
                      public cfg_manager::CFGClient,
                      public data_broker::ReceiverInterface {

    public:

      enum Status {
        UNKNOWN = -1,
        STOPPED = 0,
        RUNNING = 1,
        STOPPING = 2,
        STEPPING=3
      };

      Simulator(lib_manager::LibManager *theManager); ///< Constructor of the \c class Simulator.
      virtual ~Simulator();
      static Simulator *activeSimulator;


      // --- LibInterface ---
      int getLibVersion() const {
        return 1;
      }

      const std::string getLibName() const {
        return std::string("mars_sim");
      }

      void newLibLoaded(const std::string &libName);
      CREATE_MODULE_INFO();
      void checkOptionalDependency(const std::string &libName);


      // --- SimulatorInterface ---

      // controlling the simulation
      void updateSim(); ///< Updates the graphical simulation.
      void myRealTime(void); ///< control the realtime calculation
      void runSimulation(bool startThread = true); ///< Initiates the simulation

      /**
       * Returns the tmp path for temprary files, on linux /tmp/mars on windows the current config_dir
       */
      std::string getTmpPath() const;

      virtual void StartSimulation() {
        stepping_mutex.lock();
        simulationStatus = RUNNING;
        stepping_wc.wakeAll();
        stepping_mutex.unlock();
      }

      virtual void StopSimulation() {
        stepping_mutex.lock();
        if(simulationStatus != STOPPED) {
          simulationStatus = STOPPING;
          //stepping_wc.wakeAll();
        }
        stepping_mutex.unlock();
      }

      virtual void resetSim(bool resetGraphics=true);
      virtual bool isSimRunning() const;
      bool startStopTrigger(); ///< Starts and pauses the simulation.
      virtual void singleStep(void);
      virtual void newWorld(bool clear_all = false);
      virtual void exitMars(void);
      void readArguments(int argc, char **argv);
      virtual interfaces::ControlCenter* getControlCenter(void) const;

      // simulation contents
      void addLight(interfaces::LightData light);
      virtual void connectNodes(unsigned long id1, unsigned long id2);
      virtual void disconnectNodes(unsigned long id1, unsigned long id2);
      virtual void rescaleEnvironment(interfaces::sReal x, interfaces::sReal y, interfaces::sReal z);

      // scenes
      virtual int loadScene(const std::string &filename,
                            const std::string &robotname,bool threadsave=false, bool blocking=false);
      virtual int loadScene(const std::string &filename,
                            bool wasrunning=false,
                            const std::string &robotname="",bool threadsave=false, bool blocking=false);
      virtual int saveScene(const std::string &filename, bool wasrunning);
      virtual void exportScene() const; ///< Exports the current scene as both *.obj and *.osg file.
      virtual bool sceneChanged() const;
      virtual void sceneHasChanged(bool reset);

      //threads
      virtual bool allConcurrencysHandled(); ///< Checks if external requests are open.
      void setSyncThreads(bool value); ///< Syncs the threads of GUI and simulation.
      virtual void physicsThreadLock(void);
      virtual void physicsThreadUnlock(void);


      //physics
      virtual std::shared_ptr<interfaces::PhysicsInterface> getPhysics(void) const;
      virtual void handleError(interfaces::PhysicsError error);
      virtual void setGravity(const utils::Vector &gravity);
      virtual int checkCollisions(void);
      virtual bool hasSimFault() const; ///< Checks if the physic simulation thread has been stopped caused by an ODE error.

      //graphics
      virtual void postGraphicsUpdate(void);
      virtual void finishedDraw(void);
      void allowDraw(void); ///< Allows the osgWidget to draw a frame.

      virtual bool getAllowDraw(void) {
        return allow_draw;
      }

      virtual bool getSyncGraphics(void) {
        return sync_graphics;
      }

      // plugins
      virtual void addPlugin(const interfaces::pluginStruct& plugin);
      virtual void removePlugin(interfaces::PluginInterface *pl);
      virtual void switchPluginUpdateMode(int mode, interfaces::PluginInterface *pl);
      virtual void sendDataToPlugin(int plugin_index, void* data);

      //  virtual double initTimer(void);
      //  virtual double getTimer(double start) const;

      virtual void cfgUpdateProperty(cfg_manager::cfgPropertyStruct _property);

      const std::string getConfigDir() const {
        return config_dir;
      }

      //public slots:
      // TODO: currently this is disabled
      void noGUITimerUpdate(void);


      // --- ReceiverInterface ---

      virtual void receiveData(const data_broker::DataInfo &info,
                               const data_broker::DataPackage &package,
                               int callbackParam);

      virtual const utils::Vector& getGravity(void);

      virtual void step(bool setState = false);

      /*
       * returns the real startTimestamp plus the calculated simulation time
       */
      virtual unsigned long getTime();

    private:

      struct LoadOptions {
        std::string filename;
        std::string robotname;
        bool wasRunning;
      };

      // simulation control
      void processRequests();
      void reloadWorld(void);      

      int arg_no_gui, arg_run, arg_grid, arg_ortho;
      bool reloadSim, reloadGraphics;
      short running;
      char was_running;
      bool kill_sim;
      interfaces::ControlCenter *control; ///< Pointer to instance of ControlCenter (created in Simulator::Simulator(lib_manager::LibManager *theManager))
      std::vector<LoadOptions> filesToLoad;
      bool sim_fault;
      bool exit_sim;
      Status simulationStatus;
      interfaces::sReal sync_time;
      bool my_real_time;
      bool fast_step;      

      // graphics
      bool allow_draw;
      bool sync_graphics;
      int cameraMenuCheckedIndex;

      // threads
      bool erased_active;
      utils::ReadWriteLock pluginLocker;
      int sync_count;
      utils::Mutex externalMutex;
      utils::Mutex coreMutex;
      utils::Mutex physicsMutex;
      utils::Mutex physicsCountMutex;
      utils::Mutex stepping_mutex; ///< Used for preventing active waiting for a single step or start event.
      utils::WaitCondition stepping_wc; ///< Used for preventing active waiting for a single step or start event.
      utils::Mutex getTimeMutex;
      int physics_mutex_count;
      double avg_log_time, avg_step_time;
      int count, avg_count_steps;
      interfaces::sReal calc_time;
      
      // physics
      std::shared_ptr<interfaces::PhysicsInterface> physics;
      double calc_ms;
      int load_option;
      int std_port; ///< Controller port (default value: 1600)
      utils::Vector gravity;
      unsigned long dbPhysicsUpdateId;
      unsigned long dbSimTimeId, dbSimDebugId;
      unsigned long realStartTime;

      // plugins
      std::vector<interfaces::pluginStruct> allPlugins;
      std::vector<interfaces::pluginStruct> newPlugins;
      std::vector<interfaces::pluginStruct> activePlugins;
      std::vector<interfaces::pluginStruct> guiPlugins;

      // scenes
      int loadScene_internal(const std::string &filename, bool wasrunning, const std::string &robotname);
      std::string scenename;
      std::list<std::string> arg_v_scene_name;
      bool b_SceneChanged;
      bool haveNewPlugin;

      // configuration
      void initCfgParams(void);
      std::string config_dir;
      cfg_manager::cfgPropertyStruct cfgCalcMs, cfgFaststep;
      cfg_manager::cfgPropertyStruct cfgRealtime, cfgDebugTime;
      cfg_manager::cfgPropertyStruct cfgSyncGui, cfgDrawContact;
      cfg_manager::cfgPropertyStruct cfgGX, cfgGY, cfgGZ;
      cfg_manager::cfgPropertyStruct cfgWorldErp, cfgWorldCfm;
      cfg_manager::cfgPropertyStruct cfgVisRep;
      cfg_manager::cfgPropertyStruct cfgSyncTime;
      cfg_manager::cfgPropertyStruct configPath;
      cfg_manager::cfgPropertyStruct cfgUseNow;
      cfg_manager::cfgPropertyStruct cfgAvgCountSteps;
      
      // data
      data_broker::DataPackage dbPhysicsUpdatePackage;
      data_broker::DataPackage dbSimTimePackage;
      data_broker::DataPackage dbSimDebugPackage;

      // IceServer comServer;

    protected:
      
      // simulation control
      void run(); ///< The simulator main loop.
      
    };

  } // end of namespace sim
} // end of namespace mars

#endif  // SIMULATOR_H
