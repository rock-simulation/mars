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
 * \file ControllerManager.cpp
 * \author  Vladimir Komsiyski
 * \brief "ControllerManager" implements the ControllerManagerInterface.
 * It is manages all controllers and all controller
 * operations that are used for the communication between the simulation
 * modules.
 *
 * \version 1.3
 * Moved from the Simulator class
 * \date 07.07.2011
 */

#include "ControllerManager.h"

#include <mars/interfaces/sim/SimulatorInterface.h>
#include <mars/interfaces/sim/MotorManagerInterface.h>
#include <mars/interfaces/sim/SensorManagerInterface.h>
#include <mars/utils/MutexLocker.h>

#include <stdexcept>

namespace mars {
  namespace sim {
    using namespace std;
    using namespace utils;
    using namespace interfaces;

    /**
     * \brief Constructor.
     *
     * \param c The pointer to the ControlCenter of the simulation.
     */
    ControllerManager::ControllerManager(ControlCenter *c) {
      control = c;
      next_controller_id = 1;
      // default controller port
      std_port = 1600;
      do_not_load_controller = false;
    }

    /**
     * \brief Gives information about core exchange data for controllers.
     *
     * \param controllerList A pointer to a vector that is filled with a
     * core_objects_exchange struct for every controller. The vector is cleared
     * in the beginning of this function.
     */
    void ControllerManager::getListController(vector<core_objects_exchange> *controllerList) const {
      core_objects_exchange obj;
      map<unsigned long, Controller*>::const_iterator iter;
      controllerList->clear();
      iMutex.lock();
      for (iter = simController.begin(); iter != simController.end(); iter++) {
        iter->second->getCoreExchange(&obj);
        controllerList->push_back(obj);
      }
      iMutex.unlock();
    }


    /**
     *\brief Returns the number of controllers that are currently present in the simulation.
     *
     *\return The number of all controllers.
     */
    int ControllerManager::getControllerCount(void) const {
      MutexLocker locker(&iMutex);
      return simController.size();
    }


    /**
     * \brief Gives all information of a certain controller.
     *
     * \param index The unique id of the controller to get information for.
     *
     * \return A pointer to the ControllerData of the controller with the given id.
     * \throw std::runtime_error if the controller cannot be found
     */
    const ControllerData ControllerManager::getFullController(unsigned long index) const {
      MutexLocker locker(&iMutex);
      map<unsigned long, Controller*>::const_iterator iter;
      iter = simController.find(index);
      if (iter != simController.end())
        return iter->second->getSController();
      else {
        char msg[128];
        sprintf(msg, "could not find controller with index: %lu", index);
        throw std::runtime_error(msg);
      }
    }

    /**
     * \brief Removes a controller from the simulation.
     *
     * \param index The unique id of the controller to remove form the simulation.
     */
    void ControllerManager::removeController(unsigned long index) {
      Controller* tmpController = 0;
      iMutex.lock();
      map<unsigned long, Controller*>::iterator iter = simController.find(index);
      if (iter != simController.end()) {
        tmpController = iter->second;
        simController.erase(iter);
        if (tmpController)
          delete tmpController;
      }
      iMutex.unlock();
      control->sim->sceneHasChanged(false);
    }


    /**
     * \brief Sets the mode of the controller with the given id.
     *
     * \param id The id of the controller whose mode is to be set.
     *
     * \param mode The new mode.
     */
    void ControllerManager::setControllerAutoMode(unsigned long id, bool mode) {
      MutexLocker locker(&iMutex);
      map<unsigned long, Controller*>::iterator iter = simController.find(id);
      if (iter != simController.end())
        iter->second->setAutoMode(mode);
    }


    /**
     * \brief Sets the IP of the controller with the given id.
     *
     * \param id The id of the controller whose IP is to be set.
     *
     * \param ip The new IP.
     */
    void ControllerManager::setControllerIP(unsigned long id,
                                            const std::string &ip) {
      MutexLocker locker(&iMutex);
      map<unsigned long, Controller*>::iterator iter = simController.find(id);
      if (iter != simController.end())
        iter->second->setIP(ip);
    }



    void ControllerManager::setControllerPort(unsigned long id, int port) {
      MutexLocker locker(&iMutex);
      map<unsigned long, Controller*>::iterator iter = simController.find(id);
      if (iter != simController.end())
        iter->second->setPort(port);
    }



    /**
     * \brief Gets the mode of the controller with the given id.
     *
     * \param id The id of the controller whose mode is needed.
     *
     * \return The mode of the controller with the given id.
     */
    bool ControllerManager::getControllerAutoMode(unsigned long id) const {
      MutexLocker locker(&iMutex);
      map<unsigned long, Controller*>::const_iterator iter = simController.find(id);
      if (iter != simController.end())
        return iter->second->getAutoMode();
      else
        return false;
    }

    /**
     * \brief Gets the IP of the controller with the given id.
     *
     * \param id The id of the controller whose IP is needed.
     *
     * \return The IP of the controller with the given id.
     */
    const std::string ControllerManager::getControllerIP(unsigned long id) const {
      MutexLocker locker(&iMutex);
      map<unsigned long, Controller*>::const_iterator iter = simController.find(id);
      if (iter != simController.end()) {
        return iter->second->getIP();
      }
      else {
        return string("");
      }
    }



    /**
     * \brief Gets the port of the controller with the given id.
     *
     * \param id The id of the controller whose port is needed.
     *
     * \return The port of the controller with the given id.
     */
    int ControllerManager::getControllerPort(unsigned long id) const {
      MutexLocker locker(&iMutex);
      map<unsigned long, Controller*>::const_iterator iter = simController.find(id);
      if (iter != simController.end())
        return iter->second->getPort();
      else
        return 0;
    }



    /**
     * \brief Connects the controller with the given id.
     *
     * \param id The id of the controller that is to be connected.
     */
    void ControllerManager::connectController(unsigned long id) {
      MutexLocker locker(&iMutex);
      map<unsigned long, Controller*>::iterator iter = simController.find(id);
      if (iter != simController.end())
        iter->second->connect();
    }


    /**
     * \brief Disconnects the controller with the given id.
     *
     * \param id The id of the controller that is to be disconnected.
     */
    void ControllerManager::disconnectController(unsigned long id) {
      MutexLocker locker(&iMutex);
      map<unsigned long, Controller*>::iterator iter = simController.find(id);
      if (iter != simController.end())
        iter->second->connect();
    }


    /**
     * \brief This function updates all controllers with timing value \c calc_ms in miliseconds.
     * \warning This function is only used internally and should not be called
     * outside the core.
     *
     * \param calc_ms The timing value in miliseconds.
     */
    void ControllerManager::updateControllers(double calc_ms) {
      MutexLocker locker(&iMutex);

      map<unsigned long, Controller*>::iterator iter;
      for(iter = simController.begin(); iter != simController.end(); iter++)
        iter->second->update(calc_ms);
    }


    /**
     * \brief Resets the data of all controllers.
     */
    void ControllerManager::resetControllerData(void) {
      MutexLocker locker(&iMutex);
      map<unsigned long, Controller*>::iterator iter;
      for(iter = simController.begin(); iter != simController.end(); iter++)
        iter->second->resetData();
    }



    /**
     * \brief Destroys all controllers in the simulation.
     */
    void ControllerManager::clearAllControllers(void) {
      if (do_not_load_controller)
        return;
      MutexLocker locker(&iMutex);

      map<unsigned long, Controller*>::iterator iter;
      while(!simController.empty()) {
        delete simController.begin()->second;
        simController.erase(simController.begin());
      }
      /*
        for(iter = simController.begin(); iter != simController.end(); iter++)
        delete iter->second;
        simController.clear();
      */
      next_controller_id = 1;
    }


    void ControllerManager::handleError(void) {
      MutexLocker locker(&iMutex);
      map<unsigned long, Controller*>::iterator iter;
      for(iter = simController.begin(); iter != simController.end(); iter++)
        iter->second->handleError();
    }

    /**
     * \brief Add a controller to the simulation.
     *
     * \todo Rename the function and send a Controller instance/pointer to
     * the function as a parameter instead of the controller properties.
     *
     * \return The id of the newly added controller.
     */
    unsigned long ControllerManager::addController(const ControllerData &controller) {
      if (do_not_load_controller)
        return 0;
      std::vector<SimMotor*> vmotor;
      std::vector<BaseSensor*> vsensor;
      std::vector<NodeData*> nodes;
      std::vector<unsigned long>::const_iterator nter;
      Controller *newController;
      unsigned long id = next_controller_id++;

      for (nter = controller.motors.begin(); nter != controller.motors.end();
           nter++) {
        SimMotor* retval = control->motors->getSimMotor(*nter);
        if (retval)
          vmotor.push_back(retval);
      }

      for (nter = controller.sensors.begin(); nter != controller.sensors.end();
           nter++) {
        BaseSensor* retval = control->sensors->getSimSensor(*nter);
        if(retval) {
          vsensor.push_back(retval);
        }
      }

      if (!controller.sNodes.empty()) {
        LOG_WARN("ControllerManager::addController: sNodes are not implemented yet and are currently ignored!");
      }

      newController = new Controller(controller.rate, vmotor, vsensor, nodes,
                                     control, std_port);
      newController->setDylibPath(controller.dylib_path);
      newController->setID(id);
      iMutex.lock();
      simController[id] = newController;
      iMutex.unlock();
      return id;
    }

    /**
     * \brief Gets the default port, with which all controllers are created.
     *
     * \return The default port.
     */
    int ControllerManager::getDefaultPort(void) const {
      return std_port;
    }

    /**
     * \brief Sets the default port, with which all controllers are created.
     *
     * \param port The default port.
     */
    void ControllerManager::setDefaultPort(int port) {
      std_port = port;
    }

    /**
     * \brief Checks weather adding new controllers is allowed.
     *
     * \return \c true if allowed, \c false otherwise.
     */
    bool ControllerManager::isLoadingAllowed(void) const {
      return !do_not_load_controller;
    }

    /**
     * \brief Allows or forbids adding new controllers.
     *
     * \param allowed Indicates if adding will be allowed or not.
     */
    void ControllerManager::setLoadingAllowed(bool allowed) {
      do_not_load_controller = !allowed;
    }

    std::list<sReal> ControllerManager::getSensorValues(unsigned long id) {
      MutexLocker locker(&iMutex);
      map<unsigned long, Controller*>::iterator iter = simController.find(id);
      if (iter != simController.end())
        return iter->second->getSensorValues();
      return std::list<sReal>();
    }

  } // end of namespace sim
} // end of namespace mars
