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
 * \file SensorManagerInterface.h
 * \author  Vladimir Komsiyski
 * \brief "SensorManagerInterface" declares the interfaces for all sensor 
 * operations that are used for the communication between the simulation 
 * modules.
 *
 * \version 1.3
 * Moved from the Simulator Interface
 */

#ifndef SENSOR_MANAGER_INTERFACE_H
#define SENSOR_MANAGER_INTERFACE_H

#ifdef _PRINT_HEADER_
  #warning "SensorManagerInterface.h"
#endif

#include "ControlCenter.h"
#include "../sensor_bases.h"

#include <mars/utils/ConfigData.h>


namespace mars {
  namespace interfaces {

    class ControlCenter;
    struct cameraStruct;

    /**
     * \brief "SensorManagerInterface" declares the interfaces for all sensor 
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
    class SensorManagerInterface {
    public:

      /**
       * \brief Destructor.
       */
      virtual ~SensorManagerInterface() {}
  
      /**
       * \brief Add a sensor to the simulation.
       *
       * \param sensorS A pointer to the sensorStruct that defines the new sensor.
       *
       * \param reload Used internally by the simulation. The
       * default value is \c false. If this param is set to \c true the new sensor
       * will not be reloaded after a reset of the simulation.
       *
       * \return The unique id of the newly added sensor.
       */
      //  virtual unsigned long addSensor(BaseSensor *sensorS, bool reload = false) = 0;
  
      /**
       * \brief Gives information about core exchange data for sensors.
       *
       * \param sensorList A pointer to a vector that is filled with a
       * core_objects_exchange struct for every sensor. The vector is cleared
       * in the beginning of this function.
       */
      virtual void getListSensors(std::vector<core_objects_exchange> *sensorList)const=0;

      /**
       * \brief Gives information about core exchange data for camera sensors.
       *
       * \param cameraList A pointer to a vector that is filled with a
       * core_objects_exchange struct for every camera sensor. The vector is cleared
       * in the beginning of this function.
       */
      //virtual void getListCameraSensors(std::vector<core_objects_exchange> *cameraList) const = 0;
      //Depricated could be done by the user
  
      /**
       * \brief Gives all information of a certain sensor.
       *
       * \param index The unique id of the sensor to get information for.
       *
       * \return A pointer to the sensorStruct of the sensor with the given id.
       */
      virtual const BaseSensor* getFullSensor(unsigned long index) const = 0;
      virtual unsigned long getSensorID(std::string name) const = 0; 
  
      /**
       * \brief Removes a sensor from the simulation.
       *
       * \param index The unique id of the sensor to remove form the simulation.
       */
      virtual void removeSensor(unsigned long index) = 0;
  
      /**
       * \brief This function returns the SimSensor object for a given index.
       *
       * \param name The index of the sensor to get the core sensor object.
       *
       * \returns Returns a pointer to the corresponding sensor object.
       */
      virtual BaseSensor* getSimSensor(unsigned long index) const = 0;
  
      /**
       * \brief This function returns the GridSensor object for a given index.
       *
       * \param name The index of the sensor to get the core sensor object.
       *
       * \returns Returns a pointer to the corresponding sensor object.
       */
      //virtual GridSensorInterface* getRayGridSensor(unsigned long index) const = 0;

      /**
       * \brief This function provides the CameraSensor object for a given index.
       *
       * \param cs The cameraStruct of the sensor.
       *
       * \param index The index of the sensor to get the \c cameraStruct 
       * \returns Returns 1 if the sensor is found, 0 otherwise.
       */
      ///virtual int getCameraSensor(cameraStruct *cs, int index) const = 0;
  
      /**
       * \brief This function provides the sensor data for a given index.
       *
       * \param data The sensor data of the sensor.
       *
       * \param index The index of the sensor to get the data 
       */
      virtual int getSensorData(unsigned long id, sReal **data) const = 0;
  
      /**
       *\brief Returns the number of sensors that are currently present in the simulation.
       * 
       *\return The number of all sensors.
       */
      virtual int getSensorCount() const = 0;
  
      /** 
       * \brief Destroys all sensors in the simulation.
       *
       * \details The \c clear_all flag indicates if the reload sensors should
       * be destroyed as well. If set to \c false they are left intact.
       *
       * \param clear_all Indicates if the reload sensors should
       * be destroyed as well. If set to \c false they are left intact.
       */
      virtual void clearAllSensors(bool clear_all = false) = 0;
  
      /**
       * \brief This function reloads all sensors from a temporary sensor pool.
       *
       * \details All sensors that have been added with \c reload value as \c true
       * are added back to the simulation again with a \c reload value of \c true. 
       */
      virtual void reloadSensors(void) = 0;

      /**
       * Adds an sensor to the known sensors list
       */
      //virtual void addSensorType(const std::string &name,  BaseSensor* (*func)(ControlCenter*,const unsigned long int,const std::string,QDomElement*))=0;
  
      //virtual BaseSensor* createAndAddSensor(const std::string &type_name, std::string name="",QDomElement* config=0, bool reload=true)=0;

      //virtual void addSensorType(const std::string &name, BaseSensor* (*func)(ControlCenter*,const unsigned long int, const std::string, mars::ConfigMap*)) = 0;

      virtual BaseSensor* createAndAddSensor(utils::ConfigMap* config,
                                             bool reload=true) = 0;

      virtual BaseSensor* createAndAddSensor(const std::string &type_name,
                                             BaseConfig *config,
                                             bool reload=false)=0;


    }; // class SensorManagerInterface

  } // end of namespace interfaces
} // end of namespace mars

#endif // SENSOR_MANAGER_INTERFACE_H
