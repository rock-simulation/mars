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
 * \file ControllerManagerInterface.h
 * \author  Vladimir Komsiyski
 * \brief "ControllerManagerInterface" declares the interfaces for all controller 
 * operations that are used for the communication between the simulation 
 * modules.
 *
 * \version 1.3
 * Moved from the Simulator Interface
 */

#ifndef CONTROLLER_MANAGER_INTERFACE_H
#define CONTROLLER_MANAGER_INTERFACE_H

#ifdef _PRINT_HEADER_
  #warning "ControllerManagerInterface.h"
#endif

#include "../ControllerData.h"
#include "../core_objects_exchange.h"

#include <vector>
#include <list>

namespace mars {
  namespace interfaces {

    /**
     * \brief "ControllerManagerInterface" declares the interfaces for all controller 
     * operations that are used for the communication between the simulation modules.
     *
     * The class defines only pure virtual functions that are implemented within
     * the simulation and can be used everywhere by accessing this interface via
     * the ControlCenter class.
     *
     * It is very important to assure the serialization between the threads to
     * have the desired results. Currently the verified use of the functions 
     * is only guaranteed by calling it within the main thread (update 
     * callback from \c gui_thread).
     */
    class ControllerManagerInterface {
    public:

      /**
       * \brief Destructor.
       */
      virtual ~ControllerManagerInterface() {}
  
      /**
       * \brief Gives information about core exchange data for controllers.
       *
       * \param controllerList A pointer to a vector that is filled with a
       * core_objects_exchange struct for every controller. The vector is cleared
       * in the beginning of this function.
       */
      virtual void getListController(std::vector<core_objects_exchange> *controllerList) const = 0;

      /**
       *\brief Returns the number of controllers that are currently present in the simulation.
       * 
       *\return The number of all controllers.
       */
      virtual int getControllerCount(void) const = 0;

      /**
       * \brief Gives all information of a certain controller.
       *
       * \param index The unique id of the controller to get information for.
       *
       * \return A pointer to the ControllerData of the controller with the given id.
       */
      virtual const ControllerData getFullController(unsigned long index)const=0;

      /**
       * \brief Removes a controller from the simulation.
       *
       * \param index The unique id of the controller to remove form the simulation.
       */
      virtual void removeController(unsigned long index) = 0;

      /**
       * \brief Sets the mode of the controller with the given id.
       * 
       * \param id The id of the controller whose mode is to be set.
       *
       * \param mode The new mode.
       */
      virtual void setControllerAutoMode(unsigned long id, bool mode) = 0;

      /**
       * \brief Sets the IP of the controller with the given id.
       * 
       * \param id The id of the controller whose IP is to be set.
       *
       * \param ip The new IP.
       */
      virtual void setControllerIP(unsigned long id, const std::string &ip) = 0;

      /**
       * \brief Sets the port of the controller with the given id.
       * 
       * \param id The id of the controller whose port is to be set.
       *
       * \param mode The new port.
       */
      virtual void setControllerPort(unsigned long id, int port) = 0;

      /**
       * \brief Gets the mode of the controller with the given id.
       *
       * \param id The id of the controller whose mode is needed.
       *
       * \return The mode of the controller with the given id.
       */
      virtual bool getControllerAutoMode(unsigned long id) const = 0;

      /**
       * \brief Gets the IP of the controller with the given id.
       *
       * \param id The id of the controller whose IP is needed.
       *
       * \return The IP of the controller with the given id.
       */
      virtual const std::string getControllerIP(unsigned long id) const = 0;

      /**
       * \brief Gets the port of the controller with the given id.
       *
       * \param id The id of the controller whose port is needed.
       *
       * \return The port of the controller with the given id.
       */
      virtual int getControllerPort(unsigned long id) const = 0;

      /**
       * \brief Connects the controller with the given id.
       *
       * \param id The id of the controller that is to be connected.
       */
      virtual void connectController(unsigned long id) = 0;

      /**
       * \brief Disconnects the controller with the given id.
       *
       * \param id The id of the controller that is to be disconnected.
       */
      virtual void disconnectController(unsigned long id) = 0;

      /**
       * \brief This function updates all controllers with timing value \c calc_ms in miliseconds.
       * \warning This function is only used internally and should not be called 
       * outside the core.
       *
       * \param calc_ms The timing value in miliseconds. 
       */
      virtual void updateControllers(double calc_ms) = 0;

      /**
       * \brief Resets the data of all controllers.
       */
      virtual void resetControllerData(void) = 0;

      /** 
       * \brief Destroys all controllers in the simulation.
       */
      virtual void clearAllControllers(void) = 0;

 
      virtual void handleError(void) = 0 ;

      /**
       * \brief Add a controller to the simulation.
       *
       * \todo Rename the function and send a Controller instance/pointer to
       * the function as a parameter instead of the controller properties.
       *
       * \return The id of the newly added controller.
       */
      virtual unsigned long addController(const ControllerData &controllerData) = 0;

      /**
       * \brief Sets the default port, with which all controllers are created.
       *
       * \param port The default port.
       */
      virtual void setDefaultPort(int port) = 0;

      /**
       * \brief Gets the default port, with which all controllers are created.
       *
       * \return The default port.
       */
      virtual int getDefaultPort(void) const = 0;

      /**
       * \brief Checks weather adding new controllers is allowed.
       *
       * \return \c true if allowed, \c false otherwise.
       */
      virtual bool isLoadingAllowed(void) const = 0;

      /**
       * \brief Allows or forbids adding new controllers.
       *
       * \param allowed Indicates if adding will be allowed or not.
       */
      virtual void setLoadingAllowed(bool allowed) = 0;

      virtual std::list<sReal> getSensorValues(unsigned long id) = 0;

    }; // class ControllerManagerInterface

  } // end of namespace interfaces
} // end of namespace mars

#endif // CONTROLLER_MANAGER_INTERFACE_H
