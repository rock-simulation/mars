/*
 *  Copyright 2011, 2012, 2014, DFKI GmbH Robotics Innovation Center
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
 * \file Simulator.cpp
 * \author Malte Langosz
 *
 */

//Convention:the includes should be defined in the header file

#include "config.h"
#include "Simulator.h"
#include "PhysicsMapper.h"
#include "NodeManager.h"
#include "JointManager.h"
#include "MotorManager.h"
#include "SensorManager.h"
#include "ControllerManager.h"
#include "EntityManager.h"
#include "Controller.h"

#include <mars/utils/misc.h>
#include <mars/interfaces/SceneParseException.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>
#include <mars/interfaces/sim/LoadCenter.h>
#include <mars/interfaces/sim/LoadSceneInterface.h>
#include <mars/data_broker/DataBrokerInterface.h>
#include <lib_manager/LibInterface.hpp>
#include <mars/interfaces/Logging.hpp>

#include <signal.h>
#include <getopt.h>
#include <stdexcept>
#include <algorithm>
#include <cctype> // for tolower()

#ifdef __linux__
#include <time.h>
#include <unistd.h> //for getpid()
#endif

#ifndef DEFAULT_CONFIG_DIR
    #define DEFAULT_CONFIG_DIR "."
#endif

namespace mars {
  namespace sim {

    using namespace std;
    using namespace utils;
    using namespace interfaces;

    void hard_exit(int signal) {
      exit(signal);
    }


    Simulator *Simulator::activeSimulator = 0;

    Simulator::Simulator(lib_manager::LibManager *theManager) :
      lib_manager::LibInterface(theManager),
      exit_sim(false), allow_draw(true),
      sync_graphics(false), physics_mutex_count(0), physics(0) {

      config_dir = DEFAULT_CONFIG_DIR;
      calc_time = 0;
      avg_log_time = 0;
      count = 0;
      config_dir = ".";

      std_port = 1600;

      // we don't want the physical calculation running from the beginning
      simulationStatus = STOPPED;
      was_running = false;
      // control a clean exit from the thread
      kill_sim = 0;
      sim_fault = false;
      // set the calculation step size in ms
      calc_ms      = 10; //defaultCFG->getInt("physics", "calc_ms", 10);
      my_real_time = 0;
      show_time = 0;
      // to synchronise drawing and physics
      sync_time = 40;
      sync_count = 0;
      load_option = OPEN_INITIAL;
      reloadGraphics = reloadSim = false;
      arg_run    = 0;
      arg_grid   = 0;
      arg_ortho  = 0;
      Simulator::activeSimulator = this; // set this Simulator object to the active one

      gravity = Vector(0.0, 0.0, -9.81); // set gravity to earth conditions

      // build the factories
      control = new ControlCenter();
      control->loadCenter = new LoadCenter();
      control->sim = (SimulatorInterface*)this;
      control->cfg = 0;//defaultCFG;
      dbSimTimePackage.add("simTime", 0.);
      // load optional libs
      checkOptionalDependency("data_broker");
      checkOptionalDependency("cfg_manager");
      checkOptionalDependency("mars_graphics");
      checkOptionalDependency("log_console");

      getTimeMutex.lock();
      realStartTime = utils::getTime();
      getTimeMutex.unlock();
    }

    Simulator::~Simulator() {
      while(((Thread*)this)->isRunning())
        utils::msleep(1);
      fprintf(stderr, "Delete mars_sim\n");

      if (control->controllers) delete control->controllers;

      if(control->cfg) {
        string saveFile = configPath.sValue;
        saveFile.append("/mars_Config.yaml");
        control->cfg->writeConfig(saveFile.c_str(), "Config");

        saveFile = configPath.sValue;
        saveFile.append("/mars_Physics.yaml");
        control->cfg->writeConfig(saveFile.c_str(), "Physics");

        saveFile = configPath.sValue;
        saveFile.append("/mars_Preferences.yaml");
        control->cfg->writeConfig(saveFile.c_str(), "Preferences");

        saveFile = configPath.sValue;
        saveFile.append("/mars_Simulator.yaml");
        control->cfg->writeConfig(saveFile.c_str(), "Simulator");
      }
      // TODO: do we need to delete control?
      libManager->releaseLibrary("mars_graphics");
      libManager->releaseLibrary("cfg_manager");
      libManager->releaseLibrary("data_broker");
      libManager->releaseLibrary("log_console");
    }

    void Simulator::newLibLoaded(const std::string &libName) {
      checkOptionalDependency(libName);
    }

    void Simulator::checkOptionalDependency(const string &libName) {
      if(libName == "data_broker") {
        control->dataBroker = libManager->getLibraryAs<data_broker::DataBrokerInterface>("data_broker");
        if(control->dataBroker) {
          ControlCenter::theDataBroker = control->dataBroker;
          // create streams
          getTimeMutex.lock();
          dbSimTimeId = control->dataBroker->pushData("mars_sim", "simTime",
                                                      dbSimTimePackage,
                                                      NULL,
                                                      data_broker::DATA_PACKAGE_READ_FLAG);
          getTimeMutex.unlock();
          control->dataBroker->createTimer("mars_sim/simTimer");
          control->dataBroker->createTrigger("mars_sim/prePhysicsUpdate");
          control->dataBroker->createTrigger("mars_sim/postPhysicsUpdate");
          control->dataBroker->createTrigger("mars_sim/finishedDrawTrigger");
        } else {
          fprintf(stderr, "ERROR: could not get DataBroker!\n");
        }
      } else if(libName == "cfg_manager") {
        control->cfg = libManager->getLibraryAs<cfg_manager::CFGManagerInterface>("cfg_manager");
      } else if(libName == "mars_graphics") {
        control->graphics = libManager->getLibraryAs<interfaces::GraphicsManagerInterface>("mars_graphics");
        if(control->graphics) {
          control->loadCenter->loadMesh = control->graphics->getLoadMeshInterface();
          control->loadCenter->loadHeightmap = control->graphics->getLoadHeightmapInterface();
        }
      } else if(libName == "log_console") {
        LibInterface *lib = libManager->getLibrary("log_console");
        if(control->dataBroker) {
          if(lib) {
            LOG_DEBUG("Simulator: console loaded. stop output to stdout!");
            control->dataBroker->unregisterSyncReceiver(this, "_MESSAGES_", "*");
          } else {
            control->dataBroker->registerSyncReceiver(this, "_MESSAGES_", "fatal",
                                                      data_broker::DB_MESSAGE_TYPE_FATAL);
            control->dataBroker->registerSyncReceiver(this, "_MESSAGES_", "error",
                                                      data_broker::DB_MESSAGE_TYPE_ERROR);
            control->dataBroker->registerSyncReceiver(this, "_MESSAGES_", "warning",
                                                      data_broker::DB_MESSAGE_TYPE_WARNING);
            control->dataBroker->registerSyncReceiver(this, "_MESSAGES_", "info",
                                                      data_broker::DB_MESSAGE_TYPE_INFO);
            control->dataBroker->registerSyncReceiver(this, "_MESSAGES_", "debug",
                                                      data_broker::DB_MESSAGE_TYPE_DEBUG);
            LOG_DEBUG("Simulator: no console loaded. output to stdout!");
          }
        }
      }
    }

    /*
      void Simulator::produceData(const data_broker::DataInfo &info,
      data_broker::DataPackage *dbPackage,
      int callbackParam) {
      (*dbPackage)[0].d += calc_ms;
      }
    */

    void Simulator::runSimulation(bool startThread) {

      if(control->cfg) {
        configPath = control->cfg->getOrCreateProperty("Config", "config_path",
                                                         config_dir);

        control->cfg->getOrCreateProperty("Preferences", "resources_path",
                                          std::string(MARS_PREFERENCES_DEFAULT_RESOURCES_PATH));

	std::string loadFile = configPath.sValue+"/mars_Simulator.yaml";
	control->cfg->loadConfig(loadFile.c_str());
	loadFile = configPath.sValue+"/mars_Physics.yaml";
        control->cfg->loadConfig(loadFile.c_str());

        bool loadLastSave = false;
        control->cfg->getPropertyValue("Config", "loadLastSave", "value",
                                       &loadLastSave);
        if (loadLastSave) {
	  loadFile = configPath.sValue+"/mars_saveOnClose.yaml";
          control->cfg->loadConfig(loadFile.c_str());
        }

        initCfgParams();
      }

      control->nodes = new NodeManager(control);
      control->joints = new JointManager(control);
      control->motors = new MotorManager(control);
      control->sensors = new SensorManager(control);
      control->controllers = new ControllerManager(control);
      control->entities = new EntityManager(control);

      control->controllers->setDefaultPort(std_port);
      control->nodes->setVisualRep(0, cfgVisRep.iValue);

      if (control->graphics) {
        control->graphics->addGraphicsUpdateInterface((GraphicsUpdateInterface*)this);
      }
      // init the physics-engine
      //Convention startPhysics function
      physics = PhysicsMapper::newWorldPhysics(control);
      physics->initTheWorld();
      // the physics step_size is in seconds
      physics->step_size = calc_ms/1000.;
      physics->fast_step = false;

      physics->world_erp = cfgWorldErp.dValue;
      physics->world_cfm = cfgWorldCfm.dValue;

      gravity.x() = cfgGX.dValue;
      gravity.y() = cfgGY.dValue;
      gravity.z() = cfgGZ.dValue;
      physics->world_gravity = gravity;
      physics->draw_contact_points = cfgDrawContact.bValue;
#ifndef __linux__
      this->setStackSize(16777216);
      fprintf(stderr, "INFO: set physics stack size to: %lu\n", getStackSize());
#endif

      while(arg_v_scene_name.size() > 0) {
        LOG_INFO("Simulator: scene to load: %s",
                 arg_v_scene_name.back().c_str());
        loadScene(arg_v_scene_name.back());
        arg_v_scene_name.pop_back();
      }
      if (arg_run) {
        simulationStatus = RUNNING;
        arg_run = 0;
      }
      if (arg_grid) {
        arg_grid = 0;
        if(control->graphics)
          control->graphics->showGrid();
      }
      if (arg_ortho) {
        arg_ortho = 0;
        if(control->graphics)
          control->graphics->get3DWindow(1)->getCameraInterface()->changeCameraTypeToOrtho();
      }

      if(startThread) this->start();
    }

     /**
       * This function is executing while the program is running.
       * It handles the physical simulation, if the physical simulation is started,
       * otherwise the function is in idle mode.
       *
       * pre:
       *     start the simulator thread and by the way the Physics loop
       *
       * post:
       *
       */
    void Simulator::run() {

      while (!kill_sim) {
        stepping_mutex.lock();
        if(simulationStatus == STOPPING)
          simulationStatus = STOPPED;

        if(!isSimRunning()) {
          stepping_wc.wait(&stepping_mutex);
          if(kill_sim){
            stepping_mutex.unlock();
            break;
          }
        }

        if (sync_graphics && !sync_count) {
            msleep(2);
            stepping_mutex.unlock();
            continue;
        }

        if(simulationStatus == STEPPING){
            simulationStatus = STOPPING;
        }
        stepping_mutex.unlock();

        if(my_real_time) {
          myRealTime();
        } else if(physics_mutex_count > 0) {
          // if not in realtime this thread would lock the physicsThread right
          // after releasing it. If an other thread is trying to lock
          // it (physics_mutex_count > 0) we sleep so it has a chance.
          msleep(1);
        }
        step();
      }
      simulationStatus = STOPPED;
      // here everything of the physical simulation can be closed

      //hard_exit(0);
    }

    void Simulator::step(bool setState) {
      std::vector<pluginStruct>::iterator p_iter;
      long time;

      Status oldState;

      physicsThreadLock();

      if(setState) {
        oldState = simulationStatus;
        simulationStatus = STEPPING;
      }

#ifdef DEBUG_TIME
      long startTime = utils::getTime();

#endif
      if(control->dataBroker) {
        control->dataBroker->trigger("mars_sim/prePhysicsUpdate");
      }
      physics->stepTheWorld();
#ifdef DEBUG_TIME
      LOG_DEBUG("Step World: %ld", getTimeDiff(startTime));
#endif

      control->nodes->updateDynamicNodes(calc_ms); //Moved update to here, otherwise RaySensor is one step behind the world every time
      control->joints->updateJoints(calc_ms);
      control->motors->updateMotors(calc_ms);
      control->controllers->updateControllers(calc_ms);

      if(show_time)
        time = utils::getTime();


      getTimeMutex.lock();
      dbSimTimePackage[0].d += calc_ms;
      getTimeMutex.unlock();
      if(control->dataBroker) {
        control->dataBroker->pushData(dbSimTimeId,
                                      dbSimTimePackage);
        control->dataBroker->stepTimer("mars_sim/simTimer", calc_ms);
      }

      if(show_time) {
        avg_log_time += getTimeDiff(time);
        if(++count > 100) {
          avg_log_time /= count;
          count = 0;
          fprintf(stderr, "debug_log_time: %g\n", avg_log_time);
          avg_log_time = 0.0;
        }
      }

      pluginLocker.lockForRead();

      // It is possible for plugins to call switchPluginUpdateMode during
      // the update call and get removed from the activePlugins list there.
      // We use erased_active to notify this loop about an erasure.
      for(unsigned int i = 0; i < activePlugins.size();) {
        erased_active = false;
        if(show_time)
          time = utils::getTime();

        activePlugins[i].p_interface->update(calc_ms);

        if(!erased_active) {
          if(show_time) {
            time = getTimeDiff(time);
            activePlugins[i].timer += time;
            activePlugins[i].t_count++;
            if(activePlugins[i].t_count > 20) {
              activePlugins[i].timer /= activePlugins[i].t_count;
              activePlugins[i].t_count = 0;
              fprintf(stderr, "debug_time: %s: %g\n",
                      activePlugins[i].name.c_str(),
                      activePlugins[i].timer);
              activePlugins[i].timer = 0.0;
            }
          }
          ++i;
        }
      }
      pluginLocker.unlock();
      if (sync_graphics) {
        calc_time += calc_ms;
        if (calc_time >= sync_time) {
          sync_count = 0;
          if(control->graphics)
            this->allowDraw();
          calc_time = 0;
        }
      }
      if(control->dataBroker) {
        control->dataBroker->trigger("mars_sim/postPhysicsUpdate");
      }

      if(setState) {
        simulationStatus = oldState;
      }

      physicsThreadUnlock();
    }

    /**
     * \return \c true if started, \c false if stopped
     */
    bool Simulator::startStopTrigger() {
      //LOG_INFO("Simulator start/stop command.");
      stepping_mutex.lock();

      switch(simulationStatus) {

      case RUNNING:
        // Allow update process to finish -> transition from 2 -> 0 in main loop
        simulationStatus = STOPPING;
        //fprintf(stderr, "Simulator will be stopped\t");
        //fflush(stderr);
        stepping_wc.wakeAll();
        break;
      case STOPPING:
        //fprintf(stderr, "WARNING: Simulator is stopping. Start/Stop Trigger ignored.\t");
        //fflush(stderr);
        break;
      case STOPPED:
        simulationStatus = RUNNING;
        //fprintf(stderr, "Simulator has been started\t");
        //fflush(stderr);
        break;
      case STEPPING:
         simulationStatus = RUNNING;
         stepping_wc.wakeAll();
      default: // UNKNOWN
        //fprintf(stderr, "Simulator has unknown status\n");
        throw std::exception();
      }

      stepping_mutex.unlock();
      // Waiting for transition, i.e. main loop to set STOPPED
      //while(simulationStatus == STOPPING)
      //msleep(10);

      //fprintf(stderr, " [OK]\n");

      if(simulationStatus == STOPPED)
        return false;
      else
        return true;

    }

    //consider the case where the time step is smaller than 1 ms
    void Simulator::myRealTime() {
#ifdef __linux__  //__unix__, wenn Darwin das mitmacht.
      //used to remember last time this function was called
      //and as absolute (minimum) wake-up time.
      static struct timespec ts;
      static bool tsNeedsInit = true;
      if (tsNeedsInit)
        {
          int retval = clock_gettime(CLOCK_MONOTONIC, &ts);
          if (retval != 0)
            {
              throw std::runtime_error("clock_gettime(CLOCK_MONOTONIC, ...) failed");
            }
          tsNeedsInit = false;
        }

      //schedule minimum sleep time
      ts.tv_nsec += calc_ms * 1000000;

      //the nsec value may not exceed one second
      while (ts.tv_nsec > 1000000000)
        {
          ts.tv_nsec -= 1000000000;
          ts.tv_sec += 1;
        }

      //sleep...
      int retval = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, 0);
      if (retval != 0)
        {
          std::cerr << "WARNING: Your system is too slow!" << std::endl;
        }

      //remember time
      retval = clock_gettime(CLOCK_MONOTONIC, &ts);
      if (retval != 0)
        {
          throw std::runtime_error("clock_gettime(CLOCK_MONOTONIC, ...) failed");
        }
#else
      static long myTime = utils::getTime();
      long timeDiff = getTimeDiff(myTime);

      if(show_time) {
        fprintf(stderr, "timeDiff: %ld\n", timeDiff);
      }
      if(timeDiff < calc_ms) {
        long valSleep = calc_ms - timeDiff;
        msleep(valSleep);
        myTime = utils::getTime();
        if(show_time) {
          fprintf(stderr, "sleep time: %ld\n", valSleep);
        }
      } else {
        myTime += timeDiff;
      }
#endif
    }


    bool Simulator::isSimRunning() const {
      return (simulationStatus != STOPPED);
    }

    bool Simulator::sceneChanged() const {
      return b_SceneChanged;
    }

    std::string Simulator::getTmpPath() const{
#ifdef __linux__
        std::stringstream str;
        str << "/tmp/mars/" << (int) getpid() << "/";
        return str.str();
#else
        return configPath.sValue + std::string("/tmp/");
#endif
        
    }

    void Simulator::sceneHasChanged(bool reseted) {
      if (reseted) {
        b_SceneChanged = false;
      } else {
        b_SceneChanged = true;
      }
    }

    int Simulator::loadScene(const std::string &filename, const std::string &robotname, bool threadsave, bool blocking) {
      return loadScene(filename, false, robotname,threadsave,blocking);
    }

    int Simulator::loadScene(const std::string &filename,
                             bool wasrunning, const std::string &robotname, bool threadsave, bool blocking) {
      printf("Load 2\n");
        if(!threadsave){
            return loadScene_internal(filename,wasrunning, robotname);
        }

        //Loading is handles inside the mars thread itsels later
        externalMutex.lock();
        LoadOptions lo;
        lo.filename = filename;
        lo.wasRunning = wasrunning;
        lo.robotname = robotname;
        filesToLoad.push_back(lo);
        externalMutex.unlock();

        while(blocking && !filesToLoad.empty()){
            msleep(10);
        }
        return 1;
    }

    int Simulator::loadScene_internal(const std::string &filename,
                             bool wasrunning, const std::string &robotname) {

      LOG_DEBUG("Loading scene internal\n");

      if(control->loadCenter->loadScene.empty()) {
        LOG_ERROR("Simulator:: no module to load scene found");
        return 0;
      }

      try {
        std::string suffix = utils::getFilenameSuffix(filename);
        if( control->loadCenter->loadScene.find(suffix) !=
            control->loadCenter->loadScene.end() ) {
          if (! control->loadCenter->loadScene[suffix]->loadFile(filename.c_str(), getTmpPath().c_str(), robotname.c_str())) {
          return 0; //failed
          }
        }
        else {
          // no scene loader found
          LOG_ERROR("Simulator: Could not find scene loader for: %s (%s)",
                    filename.c_str(), suffix.c_str());
          return 0; //failed
        }
      } catch(SceneParseException e) {
        LOG_ERROR("Could not parse scene: %s", e.what());
      }

      if (wasrunning) {
        startStopTrigger();//if the simulation has been stopped for loading, now it continues
      }
      sceneHasChanged(false);
      //load_actual = 0;
      return 1;
    }

    int Simulator::saveScene(const std::string &filename, bool wasrunning) {
      std::string suffix = utils::getFilenameSuffix(filename);
      if (control->loadCenter->loadScene[suffix]->saveFile(filename, getTmpPath())!=1) {
        LOG_ERROR("Simulator: an error somewhere while saving scene");
        return 0;
      }
      if (wasrunning) {
        startStopTrigger(); // resuming the simulation
      }
      sceneHasChanged(true);
      return 1;
    }

    void Simulator::addLight(LightData light) {
      sceneHasChanged(false);
      if (control->graphics && control->controllers->isLoadingAllowed())
        control->graphics->addLight(light);
    }


    void Simulator::finishedDraw(void) {
      long time;

      processRequests();

      if (reloadSim) {
        while (simulationStatus != STOPPED) {

          if(simulationStatus == RUNNING) {
            StopSimulation();
          }

          msleep(10);
        }
        reloadSim = false;
        control->controllers->setLoadingAllowed(false);

        /*
          while (isSimRunning()) {
          msleep(100);
          }
        */
        newWorld();
        reloadWorld();
        control->controllers->resetControllerData();

        for (unsigned int i=0; i<allPlugins.size(); i++)
          allPlugins[i].p_interface->reset();
        control->controllers->setLoadingAllowed(true);
        if (was_running) {
          StartSimulation();
        }
        reloadGraphics = true;
      }
      allow_draw = 0;
      sync_count = 1;

      /** NO idea what the following commented section should be good for
          the erased_active is pretty much useless
          and the need for 'first' is not described

          if(first == 0) {
          first--;
          // open Plugin file
          pluginLocker.lockForRead();
          for(unsigned int i = 0; i < activePlugins.size();) {
          erased_active = false;
          activePlugins[i].p_interface->init();
          if(!erased_active) i++;
          }
          pluginLocker.unlock();
          }
      */

      // Add plugins that have been added via Simulator::addPlugin
      pluginLocker.lockForWrite();
      for (unsigned int i=0; i<newPlugins.size(); i++) {
        allPlugins.push_back(newPlugins[i]);
        activePlugins.push_back(newPlugins[i]);
        newPlugins[i].p_interface->init();
      }
      newPlugins.clear();
      pluginLocker.unlock();


      pluginLocker.lockForRead();
      for (unsigned int i=0; i<guiPlugins.size(); i++) {
        if(show_time)
          time = utils::getTime();

        guiPlugins[i].p_interface->update(0);

        if(show_time) {
          time = getTimeDiff(time);
          guiPlugins[i].timer_gui += time;
          guiPlugins[i].t_count_gui++;
          if(guiPlugins[i].t_count_gui > 20) {
            guiPlugins[i].timer_gui /= guiPlugins[i].t_count_gui;
            guiPlugins[i].t_count_gui = 0;
            fprintf(stderr, "debug_time_gui: %s: %g\n",
                    guiPlugins[i].name.data(),
                    guiPlugins[i].timer_gui);
            guiPlugins[i].timer_gui = 0.0;
          }
        }
      }
      pluginLocker.unlock();
      // process ice events
      //while(comServer.eventList->processEvent(control)) {}
      control->dataBroker->trigger("mars_sim/finishedDrawTrigger");
    }

    void Simulator::newWorld(bool clear_all) {
      physicsThreadLock();
      // reset simTime
      dbSimTimePackage[0].set(0.);
      control->controllers->clearAllControllers();
      control->sensors->clearAllSensors(clear_all);
      control->motors->clearAllMotors(clear_all);
      control->joints->clearAllJoints(clear_all);
      control->nodes->clearAllNodes(clear_all, reloadGraphics);
      if(control->graphics) {
        control->graphics->clearDrawItems();
        if(reloadGraphics) {
          control->graphics->reset();
        }
      }

      sceneHasChanged(true);
      physics->freeTheWorld();
      physics->initTheWorld();
      physicsThreadUnlock();
    }

    void Simulator::resetSim(bool resetGraphics) {
      reloadSim = true;
      reloadGraphics = resetGraphics;
      stepping_mutex.lock();
      if(simulationStatus == RUNNING)
        was_running = true;
      else
        was_running = false;

      if(simulationStatus != STOPPED) {
        simulationStatus = STOPPING;
      }
      if(control->graphics)
        this->allowDraw();

      //stepping_wc.wakeAll();
      stepping_mutex.unlock();
    }


    void Simulator::reloadWorld(void) {
      control->nodes->reloadNodes(reloadGraphics);
      control->joints->reloadJoints();
      control->motors->reloadMotors();
      control->sensors->reloadSensors();
    }

    void Simulator::readArguments(int argc, char **argv) {
      int c;
      int option_index = 0;
      int psflag = 0;
      int pgflag = 0;
      int psvflag = 0;
      int pdflag = 0;

      static struct option long_options[] = {
        {"help",no_argument,0,'h'},
        {"run",no_argument,0,'r'},
        {"show_grid",no_argument,0,'g'},
        {"ortho",no_argument,0,'o'},
        {"no-gui",no_argument,0,'G'},
        {"scenename", 1, 0, 's'},
        {"config_dir", required_argument, 0, 'C'},
        {"c_port",1,0,'c'},
        {0, 0, 0, 0}
      };

      while (1) {
        c = getopt_long(argc, argv, "hrgoGs:C:p:", long_options, &option_index);
        if (c == -1)
          break;
        switch (c) {
        case 's':
          {
            std::vector<std::string> tmp_v_s;
            tmp_v_s = explodeString(';', optarg);
            for(unsigned int i=0; i<tmp_v_s.size(); ++i) {
              if(pathExists(tmp_v_s[i])) {
                arg_v_scene_name.push_back(tmp_v_s[i]);
              }
              else {
                LOG_ERROR("The given scene file does not exists: %s\n",
                       tmp_v_s[i].c_str());
              }
            }
          }
          break;
        case 'C':
          if(pathExists(optarg)) config_dir = optarg;
          else printf("The given configuration Directory does not exists: %s\n", optarg);
          break;
        case 'r':
          arg_run = 1;
          break;
        case 'c':
          std_port = atoi(optarg);
          break;
        case 'g':
          arg_grid = 1;
          break;
        case 'o':
          arg_ortho = 1;
          break;
        case 'G':
          break;
        case 'h':
        default:
          printf("\naccepted parameters are:\n");
          printf("=======================================\n");
          printf("-h             this screen:\n");
          printf("-s <filename>  filename for scene to load\n");
          printf("-r             start directly the simulation\n");
          printf("-c             set standard controller port\n");
          printf("-C             path to Configuration\n");
          printf("-g             show 3d grid\n");
          printf("-o             ortho perspective as standard\n");
          printf("\n");
        }
      }

      return;
    }

    void Simulator::physicsThreadLock(void) {
      // physics_mutex_count is used to see how many threads are trying to
      // acquire the lock. Also see Simulator::run() on how this is used.
      physicsCountMutex.lock();
      physics_mutex_count++;
      physicsCountMutex.unlock();
      physicsMutex.lock();
    }

    void Simulator::physicsThreadUnlock(void) {
      // physics_mutex_count is used to see how many threads are trying to
      // acquire the lock. Also see Simulator::run() on how this is used.
      physicsCountMutex.lock();
      physics_mutex_count--;
      physicsCountMutex.unlock();
      physicsMutex.unlock();
    }

    PhysicsInterface* Simulator::getPhysics(void) const {
      return physics;
    }

    void Simulator::postGraphicsUpdate(void) {
      finishedDraw();
    }

    void Simulator::exitMars(void) {
      stepping_mutex.lock();
      kill_sim = 1;
      stepping_wc.wakeAll();
      stepping_mutex.unlock();
      if(isCurrentThread()) {
        return;
      }
      while(this->isRunning()) {
        msleep(1);
      }
    }

    void Simulator::connectNodes(unsigned long id1, unsigned long id2) {
      //NEW_JOINT_STRUCT(connect_joint);
      JointData connect_joint;
      connect_joint.nodeIndex1 = id1;
      connect_joint.nodeIndex2 = id2;
      connect_joint.type = JOINT_TYPE_FIXED;
      control->joints->addJoint(&connect_joint);
      LOG_INFO("Simulator: connect node %lu and %lu", id1, id2);
    }

    void Simulator::disconnectNodes(unsigned long id1, unsigned long id2) {
      control->joints->removeJointByIDs(id1, id2);
    }

    void Simulator::rescaleEnvironment(sReal x, sReal y, sReal z)
    {
      // rescale all nodes
      // reset the ancores positions
      //
      control->nodes->scaleReloadNodes(x, y, z);
      control->joints->scaleReloadJoints(x, y ,z);
      resetSim();
    }


    void Simulator::singleStep(void) {
      stepping_mutex.lock();
      simulationStatus = STEPPING;
      stepping_wc.wakeAll();
      stepping_mutex.unlock();
    }

    void Simulator::switchPluginUpdateMode(int mode, PluginInterface *pl) {
      std::vector<pluginStruct>::iterator p_iter;
      bool afound = false;
      bool gfound = false;

      for(p_iter=activePlugins.begin(); p_iter!=activePlugins.end();
          p_iter++) {
        if((*p_iter).p_interface == pl) {
          afound = true;
          if(!(mode & PLUGIN_SIM_MODE)) {
            activePlugins.erase(p_iter);
            erased_active = true;
          }
          break;
        }
      }

      for(p_iter=guiPlugins.begin(); p_iter!=guiPlugins.end();
          p_iter++) {
        if((*p_iter).p_interface == pl) {
          gfound = true;
          if(!(mode & PLUGIN_GUI_MODE))
            guiPlugins.erase(p_iter);
          break;
        }
      }

      for(p_iter=allPlugins.begin(); p_iter!=allPlugins.end();
          p_iter++) {
        if((*p_iter).p_interface == pl) {
          if(mode & PLUGIN_SIM_MODE && !afound)
            activePlugins.push_back(*p_iter);
          if(mode & PLUGIN_GUI_MODE && !gfound)
            guiPlugins.push_back(*p_iter);
          break;
        }
      }
    }

    /**
     * If \c true you cannot recover (currently) from this point without restarting the simulation.
     * To extend this, restart the simulator thread and reset the scene (untested).
     *
     * \return \c true if the simulation thread was not interrupted by ODE
     */
    bool Simulator::hasSimFault() const{
        return sim_fault;
    }

    void Simulator::handleError(PhysicsError error) {
      std::vector<pluginStruct>::iterator p_iter;

      switch(error) {
      case PHYSICS_NO_ERROR:
      case PHYSICS_DEBUG:
      case PHYSICS_ERROR:
        break;
      case PHYSICS_UNKNOWN:
        LOG_WARN("looks like we caught a unknown exception from ODE.");
        break;
      }

      for(p_iter=allPlugins.begin(); p_iter!=allPlugins.end();
          p_iter++) {
        (*p_iter).p_interface->handleError();
      }

      control->controllers->handleError();

      string onError;
      control->cfg->getPropertyValue("Simulator", "onPhysicsError",
                                     "value", &onError);
      std::transform(onError.begin(), onError.end(),
                     onError.begin(), ::tolower);
      if("abort" == onError || "" == onError) {
        abort();
      } else if("reset" == onError) {
        resetSim();
      } else if("warn" == onError) {
        // warning already happend in message handler
      } else if("shutdown" == onError){
        //Killing the simulation thread, means the simulation gui still runs but the simulation thread get's stopped
        //In this state the the sim_fault is set to true which can be checked externally to react to this fault
        sim_fault = true;
        kill_sim = true;
      } else {
        LOG_WARN("unsupported config value for \"Simulator/onPhysicsError\": \"%s\"", onError.c_str());
        LOG_WARN("aborting by default...");
        abort();
      }
    }

    void Simulator::setGravity(const Vector &gravity) {
      if(control->cfg) {
        control->cfg->setPropertyValue("Simulator", "Gravity x", "value",
                                       gravity.x());
        control->cfg->setPropertyValue("Simulator", "Gravity y", "value",
                                       gravity.y());
        control->cfg->setPropertyValue("Simulator", "Gravity z", "value",
                                       gravity.z());
      }
    }


    void Simulator::noGUITimerUpdate(void) {
      finishedDraw();
    }


    ControlCenter* Simulator::getControlCenter(void) const {
      return control;
    }

    void Simulator::addPlugin(const pluginStruct& plugin) {
      pluginLocker.lockForWrite();
      newPlugins.push_back(plugin);
      pluginLocker.unlock();
    }

    void Simulator::removePlugin(PluginInterface *pl) {
      std::vector<pluginStruct>::iterator p_iter;

      pluginLocker.lockForWrite();

      for(p_iter=activePlugins.begin(); p_iter!=activePlugins.end();
          p_iter++) {
        if((*p_iter).p_interface == pl) {
          activePlugins.erase(p_iter);
          break;
        }
      }

      for(p_iter=guiPlugins.begin(); p_iter!=guiPlugins.end();
          p_iter++) {
        if((*p_iter).p_interface == pl) {
          guiPlugins.erase(p_iter);
          break;
        }
      }

      for(p_iter=allPlugins.begin(); p_iter!=allPlugins.end();
          p_iter++) {
        if((*p_iter).p_interface == pl) {
          allPlugins.erase(p_iter);
          break;
        }
      }

      pluginLocker.unlock();
    }

    int Simulator::checkCollisions(void) {
      return physics->checkCollisions();
    }

    void Simulator::sendDataToPlugin(int plugin_index, void* data) {
      if(plugin_index < 0 || plugin_index+1 > (int)allPlugins.size()) return;

      allPlugins[plugin_index].p_interface->getSomeData(data);
    }


    void Simulator::setSyncThreads(bool value) {
      sync_graphics = value;
    }

    /**
     * Calls GraphicsManager::update() method to redraw all OSG objects in the simulation.
     */
    void Simulator::updateSim() {
      if(control->graphics)
        control->graphics->update();
    }

    /**
     * This method is used for gui and simulation synchronization.
     */
    void Simulator::allowDraw(void) {
      allow_draw = 1;
    }

    /**
     * \return \c true if no external requests are open.
     */
    bool Simulator::allConcurrencysHandled(){
        return filesToLoad.empty();
    }

    /** This method is used for all calls that cannot be done from an external thread.
     * This means the requests have to be caches (like in loadScene) and have to be handled by
     * this method, which is called by Simulator::run().
     */
    void Simulator::processRequests() {
      externalMutex.lock();
      if(filesToLoad.size() > 0) {
        bool wasrunning = false;
        while (simulationStatus != STOPPED) {

          if(simulationStatus == RUNNING) {
            StopSimulation();
            wasrunning = true;
          }

          msleep(10);
        }

        for(unsigned int i=0;i<filesToLoad.size();i++){
          loadScene_internal(filesToLoad[i].filename, false,
                             filesToLoad[i].robotname);
        }
        filesToLoad.clear();

        if(wasrunning) {
          StartSimulation();
        }
      }
      externalMutex.unlock();
    }


    void Simulator::exportScene(void) const {
      if(control->graphics) {
        string filename = "export.obj";
        control->graphics->exportScene(filename);
        filename = "export.osg";
        control->graphics->exportScene(filename);
      }
    }

    /* will be removed soon
       void* Simulator::getWinId() const {
       return 0;//(void*)mainGUI->mainWindow_p()->winId();
       }
    */

    void Simulator::cfgUpdateProperty(cfg_manager::cfgPropertyStruct _property) {

      if(_property.paramId == cfgCalcMs.paramId) {
        calc_ms = _property.dValue;
        if(physics) physics->step_size = calc_ms*0.001; // The physics step_size is defined in seconds.
        if(control->joints) control->joints->changeStepSize();
        return;
      }

      if(_property.paramId == cfgFaststep.paramId) {
        if(physics) physics->fast_step = _property.bValue;
        return;
      }

      if(_property.paramId == cfgRealtime.paramId) {
        my_real_time = _property.bValue;
        return;
      }

      if(_property.paramId == cfgDebugTime.paramId) {
        show_time = _property.bValue;
        return;
      }

      if(_property.paramId == cfgSyncGui.paramId) {
        this->setSyncThreads(_property.bValue);
        return;
      }

      if(_property.paramId == cfgSyncTime.paramId) {
        sync_time = _property.dValue;
        return;
      }

      if(_property.paramId == cfgDrawContact.paramId) {
        physics->draw_contact_points = _property.bValue;
        return;
      }

      if(_property.paramId == cfgGX.paramId) {
        gravity.x() = _property.dValue;
        physics->world_gravity = gravity;
        return;
      }

      if(_property.paramId == cfgGY.paramId) {
        gravity.y() = _property.dValue;
        physics->world_gravity = gravity;
        return;
      }

      if(_property.paramId == cfgGZ.paramId) {
        gravity.z() = _property.dValue;
        physics->world_gravity = gravity;
        return;
      }

      if(_property.paramId == cfgWorldErp.paramId) {
        physics->world_erp = _property.dValue;
        return;
      }

      if(_property.paramId == cfgWorldCfm.paramId) {
        physics->world_cfm = _property.dValue;
        return;
      }

      if(_property.paramId == cfgVisRep.paramId) {
        control->nodes->setVisualRep(0, _property.iValue);
        return;
      }

    }

    void Simulator::initCfgParams(void) {
      if(!control->cfg)
        return;
      cfgCalcMs = control->cfg->getOrCreateProperty("Simulator", "calc_ms",
                                                    calc_ms, this);
      calc_ms = cfgCalcMs.dValue;
      cfgFaststep = control->cfg->getOrCreateProperty("Simulator", "faststep",
                                                      false, this);
      cfgRealtime = control->cfg->getOrCreateProperty("Simulator", "realtime calc",
                                                      false, this);
      my_real_time = cfgRealtime.bValue;

      cfgDebugTime = control->cfg->getOrCreateProperty("Simulator", "debug time",
                                                       false, this);

      cfgSyncGui = control->cfg->getOrCreateProperty("Simulator", "sync gui",
                                                       false, this);

      cfgSyncTime = control->cfg->getOrCreateProperty("Simulator", "sync time",
                                                       40.0, this);

      cfgDrawContact = control->cfg->getOrCreateProperty("Simulator", "draw contacts",
                                                         false, this);

      cfgGX = control->cfg->getOrCreateProperty("Simulator", "Gravity x",
                                                0.0, this);

      cfgGY = control->cfg->getOrCreateProperty("Simulator", "Gravity y",
                                                0.0, this);

      cfgGZ = control->cfg->getOrCreateProperty("Simulator", "Gravity z",
                                                -9.81, this);

      cfgWorldErp = control->cfg->getOrCreateProperty("Simulator", "world erp",
                                                      0.1, this);

      cfgWorldCfm = control->cfg->getOrCreateProperty("Simulator", "world cfm",
                                                      1e-10, this);

      cfgVisRep = control->cfg->getOrCreateProperty("Simulator", "visual rep.",
                                                    (int)1, this);

      cfgUseNow = control->cfg->getOrCreateProperty("Simulator", "getTime:useNow",
                                                    (bool)false, this);

      control->cfg->getOrCreateProperty("Simulator", "onPhysicsError",
                                        "abort", this);
      show_time = cfgDebugTime.bValue;

    }

    void Simulator::receiveData(const data_broker::DataInfo &info,
                                const data_broker::DataPackage &package,
                                int callbackParam) {
      if(info.groupName != "_MESSAGES_") {
        fprintf(stderr, "got unexpected data broker package: %s %s\n",
                info.groupName.c_str(), info.dataName.c_str());
        return;
      }
      // output to stdout
      std::string message;
      package.get(0, &message);
      switch(callbackParam) {
      case data_broker::DB_MESSAGE_TYPE_FATAL:
#ifndef WIN32
        fprintf(stderr, "\033[31mfatal: %s\033[0m\n", message.c_str());
#else
        fprintf(stderr, "fatal: %s\n", message.c_str());
#endif
        break;
      case data_broker::DB_MESSAGE_TYPE_ERROR:
#ifndef WIN32
        fprintf(stderr, "\033[1;31merror: %s\033[0m\n", message.c_str());
#else
        fprintf(stderr, "error: %s\n", message.c_str());
#endif
        break;
      case data_broker::DB_MESSAGE_TYPE_WARNING:
#ifndef WIN32
        fprintf(stderr, "\033[0;32mwarning: %s\033[0m\n", message.c_str());
#else
        fprintf(stderr, "warning: %s\n", message.c_str());
#endif
        break;
      case data_broker::DB_MESSAGE_TYPE_INFO:
      case data_broker::DB_MESSAGE_TYPE_DEBUG:
#ifndef WIN32
        fprintf(stderr, "\033[1;34minfo: %s\033[0m\n", message.c_str());
#else
        fprintf(stderr, "info: %s\n", message.c_str());
#endif
        break;
      default:
        fprintf(stderr, "???: %s\n", message.c_str());
        break;
      }
    }

    const utils::Vector& Simulator::getGravity(void) {
      return physics->world_gravity;
    }

    unsigned long Simulator::getTime() {
      unsigned long returnTime;
      getTimeMutex.lock();
      if(cfgUseNow.bValue) {
        returnTime = realStartTime+dbSimTimePackage[0].d;
      }
      else {
        returnTime = realStartTime+dbSimTimePackage[0].d;
      }
      getTimeMutex.unlock();
      return returnTime;
    }


  } // end of namespace sim

  namespace interfaces {
      SimulatorInterface* SimulatorInterface::getInstance(lib_manager::LibManager *libManager) {
      return new sim::Simulator(libManager);
    }
  } // end of namespace interfaces

} // end of namespace mars

DESTROY_LIB(mars::sim::Simulator);
CREATE_LIB(mars::sim::Simulator);
