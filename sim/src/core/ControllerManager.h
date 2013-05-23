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
 * \file ControllerManager.h
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

#ifndef CONTROLLER_MANAGER_H
#define CONTROLLER_MANAGER_H

#ifdef _PRINT_HEADER_
  #warning "ControllerManager.h"
#endif

#include "Controller.h"

#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/interfaces/sim/ControllerManagerInterface.h>
#include <mars/utils/Mutex.h>

namespace mars {
  namespace sim {

    /**
     * \brief "ControllerManager" imlements the interfaces for all controller 
     * operations that are used for the communication between the simulation 
     * modules. Inherits from ControllerManagerInterface.
     *
     * \warning It is very important to assure the serialization between the threads to
     * have the desired results. Currently the verified use of the functions 
     * is only guaranteed by calling it within the main thread (update 
     * callback from \c gui_thread).
     */
    class ControllerManager : public interfaces::ControllerManagerInterface {
    public:

      /**
       * \brief Constructor.
       *
       * \param c The pointer to the ControlCenter of the simulation.
       */ 
      ControllerManager(interfaces::ControlCenter *c);
  
      /**
       * \brief Destructor.
       */
      virtual ~ControllerManager(){}
  
      /**
       * \brief Gives information about core exchange data for controllers.
       *
       * \param controllerList A pointer to a vector that is filled with a
       * core_objects_exchange struct for every controller. The vector is cleared
       * in the beginning of this function.
       */
      virtual void getListController(std::vector<interfaces::core_objects_exchange> *controllerList) const;
  
      /**
       *\brief Returns the number of controllers that are currently present in the simulation.
       * 
       *\return The number of all controllers.
       */
      virtual int getControllerCount(void) const;

      /**
       * \brief Gives all information of a certain controller.
       *
       * \param index The unique id of the controller to get information for.
       *
       * \return A pointer to the ControllerData of the controller with the given id.
       * \throw std::runtime_error if the controller cannot be found
       */
      virtual const interfaces::ControllerData getFullController(unsigned long index) const;

      /**
       * \brief Removes a controller from the simulation.
       *
       * \param index The unique id of the controller to remove form the simulation.
       */
      virtual void removeController(unsigned long index);

      /**
       * \brief Sets the mode of the controller with the given id.
       * 
       * \param id The id of the controller whose mode is to be set.
       *
       * \param mode The new mode.
       */
      virtual void setControllerAutoMode(unsigned long id, bool mode);

      /**
       * \brief Sets the IP of the controller with the given id.
       * 
       * \param id The id of the controller whose IP is to be set.
       *
       * \param ip The new IP.
       */
      virtual void setControllerIP(unsigned long id, const std::string &ip);

      /**
       * \brief Gets the mode of the controller with the given id.
       *
       * \param id The id of the controller whose mode is needed.
       *
       * \return The mode of the controller with the given id.
       */
      virtual void setControllerPort(unsigned long id, int port);

      /**
       * \brief Gets the mode of the controller with the given id.
       *
       * \param id The id of the controller whose mode is needed.
       *
       * \return The mode of the controller with the given id.
       */
      virtual bool getControllerAutoMode(unsigned long id) const;

      /**
       * \brief Gets the IP of the controller with the given id.
       *
       * \param id The id of the controller whose IP is needed.
       *
       * \return The IP of the controller with the given id.
       */
      virtual const std::string getControllerIP(unsigned long id) const;

      /**
       * \brief Gets the port of the controller with the given id.
       *
       * \param id The id of the controller whose port is needed.
       *
       * \return The port of the controller with the given id.
       */
      virtual int getControllerPort(unsigned long id) const;

      /**
       * \brief Connects the controller with the given id.
       *
       * \param id The id of the controller that is to be connected.
       */
      virtual void connectController(unsigned long id);

      /**
       * \brief Disconnects the controller with the given id.
       *
       * \param id The id of the controller that is to be disconnected.
       */
      virtual void disconnectController(unsigned long id);

      /**
       * \brief This function updates all controllers with timing value \c calc_ms in miliseconds.
       * \warning This function is only used internally and should not be called 
       * outside the core.
       *
       * \param calc_ms The timing value in miliseconds. 
       */
      virtual void updateControllers(interfaces::sReal calc_ms);

      /**
       * \brief Resets the data of all controllers.
       */
      virtual void resetControllerData(void);

      /** 
       * \brief Destroys all controllers in the simulation.
       */
      virtual void clearAllControllers(void);


      virtual void handleError(void);

      /**
       * \brief Add a controller to the simulation.
       *
       * \todo Rename the function and send a Controller instance/pointer to
       * the function as a parameter instead of the controller properties.
       *
       * \return The id of the newly added controller.
       */
      virtual unsigned long addController(const interfaces::ControllerData &controller);

      /**
       * \brief Sets the default port, with which all controllers are created.
       *
       * \param port The default port.
       */
      virtual void setDefaultPort(int port);

      /**
       * \brief Gets the default port, with which all controllers are created.
       *
       * \return The default port.
       */
      virtual int getDefaultPort(void) const;

      /**
       * \brief Checks weather adding new controllers is allowed.
       *
       * \return \c true if allowed, \c false otherwise.
       */
      virtual bool isLoadingAllowed(void) const;

      /**
       * \brief Allows or forbids adding new controllers.
       *
       * \param allowed Indicates if adding will be allowed or not.
       */
      virtual void setLoadingAllowed(bool allowed);
  
      virtual std::list<interfaces::sReal> getSensorValues(unsigned long id);

    private:
  
      //! a flag indicating if adding new controllers is allowed
      bool do_not_load_controller;

      //! the default port passed to every newly added controller
      int std_port;

      //! the id of the next controller added to the simulation
      unsigned long next_controller_id;

      //! a containter holding all controllers in the simulation
      std::map<unsigned long, Controller*> simController;

      //! a pointer to the control center
      interfaces::ControlCenter *control;

      //! a mutex for the controllers containter
      mutable utils::Mutex iMutex;

    }; // class ControllerManager

  } // end of namespace sim
} // end of namespace mars

#endif  // CONTROLLER_MANAGER_H
