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
 * \file SensorManager.h
 * \author  Vladimir Komsiyski
 * \brief SensorManager implements SensorManagerInterface and
 * manages all sensors and all sensor
 * operations that are used for the communication between the simulation
 * modules.
 *
 * \version 1.3
 * Moved from the Simulator
 * \date 07.07.2011
 */

#include "SensorManager.h"

// sensor includes
#include "JointAVGTorqueSensor.h"
#include "JointLoadSensor.h"
#include "NodePositionSensor.h"
#include "NodeRotationSensor.h"
#include "NodeContactSensor.h"
#include "NodeContactForceSensor.h"
#include "NodeCOMSensor.h"
#include "JointPositionSensor.h"
#include "JointVelocitySensor.h"
#include "CameraSensor.h"
#include "NodeVelocitySensor.h"
#include "RaySensor.h"
#include "RotatingRaySensor.h"
#include "MultiLevelLaserRangeFinder.h"
#include "RayGridSensor.h"
#include "NodeAngularVelocitySensor.h"
#include "MotorCurrentSensor.h"
#include "HapticFieldSensor.h"
#include "Joint6DOFSensor.h"
#include "JointTorqueSensor.h"
#include "ScanningSonar.h"

#include <mars/interfaces/sim/SimulatorInterface.h>
#include <mars/utils/MutexLocker.h>
#include <mars/interfaces/Logging.hpp>

#include <cstdio>
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
    SensorManager::SensorManager(ControlCenter *c)
    {
      control = c;
      next_sensor_id = 1;
      addSensorType("RaySensor",&RaySensor::instanciate);
      addSensorType("RotatingRaySensor",&RotatingRaySensor::instanciate);
      addSensorType("MultiLevelLaserRangeFinder",&MultiLevelLaserRangeFinder::instanciate);
      addSensorType("CameraSensor",&CameraSensor::instanciate);
      addSensorType("ScanningSonar",&ScanningSonar::instanciate);
      addSensorType("JointPosition",&JointPositionSensor::instanciate);
      addSensorType("JointVelocity",&JointVelocitySensor::instanciate);
      addSensorType("JointLoad",&JointLoadSensor::instanciate);
      addSensorType("JointTorque",&JointTorqueSensor::instanciate);
      addSensorType("JointAVGTorque",&JointAVGTorqueSensor::instanciate);
      addSensorType("Joint6DOF",&Joint6DOFSensor::instanciate);
      addSensorType("NodeContact",&NodeContactSensor::instanciate);
      addSensorType("NodePosition",&NodePositionSensor::instanciate);
      addSensorType("NodeRotation",&NodeRotationSensor::instanciate);
      addSensorType("NodeContactForce",&NodeContactForceSensor::instanciate);
      addSensorType("NodeCOM",&NodeCOMSensor::instanciate);
      addSensorType("NodeVelocity",&NodeVelocitySensor::instanciate);
      addSensorType("NodeAngularVelocity",&NodeAngularVelocitySensor::instanciate);
      addSensorType("MotorCurrent",&MotorCurrentSensor::instanciate);
      addSensorType("HapticField",&HapticFieldSensor::instanciate);

      addMarsParser("RaySensor",&RaySensor::parseConfig);
      addMarsParser("RotatingRaySensor",&RotatingRaySensor::parseConfig);
      addMarsParser("MultiLevelLaserRangeFinder",&MultiLevelLaserRangeFinder::parseConfig);
      addMarsParser("CameraSensor",&CameraSensor::parseConfig);
      addMarsParser("ScanningSonar",&ScanningSonar::parseConfig);
      addMarsParser("JointPosition",&JointArraySensor::parseConfig);
      addMarsParser("JointVelocity",&JointArraySensor::parseConfig);
      addMarsParser("JointLoad",&JointArraySensor::parseConfig);
      addMarsParser("JointTorque",&JointArraySensor::parseConfig);
      addMarsParser("JointAVGTorque",&JointArraySensor::parseConfig);
      addMarsParser("Joint6DOF",&Joint6DOFSensor::parseConfig);
      addMarsParser("NodeContact",&NodeContactSensor::parseConfig);
      addMarsParser("NodePosition",&NodeArraySensor::parseConfig);
      addMarsParser("NodeRotation",&NodeArraySensor::parseConfig);
      addMarsParser("NodeContactForce",&NodeArraySensor::parseConfig);
      addMarsParser("NodeCOM",&NodeArraySensor::parseConfig);
      addMarsParser("NodeVelocity",&NodeArraySensor::parseConfig);
      addMarsParser("NodeAngularVelocity",&NodeArraySensor::parseConfig);
      addMarsParser("MotorCurrent",&MotorCurrentSensor::parseConfig);
      addMarsParser("HapticField",&HapticFieldSensor::parseConfig);

      // missing sensors:
      //   RayGridSensor
    }


    /**
     * \brief Gives information about core exchange data for sensors.
     *
     * \param sensorList A pointer to a vector that is filled with a
     * core_objects_exchange struct for every sensor. The vector is cleared
     * in the beginning of this function.
     */
    void SensorManager::getListSensors(vector<core_objects_exchange> *sensorList) const {
      core_objects_exchange obj;
      map<unsigned long, BaseSensor*>::const_iterator iter;
      sensorList->clear();
      iMutex.lock();
      for (iter = simSensors.begin(); iter != simSensors.end(); iter++) {
        iter->second->getCoreExchange(&obj);
        sensorList->push_back(obj);
      }
      iMutex.unlock();
    }

    /**
     * \brief Gives all information of a certain sensor.
     *
     * \param index The unique id of the sensor to get information for.
     *
     * \return A pointer to the BaseSensor of the sensor with the given id.
     * \throw std::runtime_error if a motor with the given index does not exist.
     */
    const BaseSensor* SensorManager::getFullSensor(unsigned long index) const {
      MutexLocker locker(&iMutex);
      map<unsigned long, BaseSensor*>::const_iterator iter;

      iter = simSensors.find(index);
      if (iter != simSensors.end())
        return iter->second;
      else {
        char msg[128];
        sprintf(msg, "could not find sensor with index: %lu", index);
        throw std::runtime_error(msg);
      }
    }

    unsigned long SensorManager::getSensorID(std::string name) const {
      MutexLocker locker(&iMutex);
      std::map<unsigned long, BaseSensor*>::const_iterator it;
      for(it = simSensors.begin(); it != simSensors.end(); it++){
        if(it->second->name.compare(name) == 0){
          return it->first;
        }
      }
      printf("Cannot find Sensor with name: \"%s\"\n",name.c_str());
      return 0;
    }

    /**
     * \brief Removes a sensor from the simulation.
     *
     * \param index The unique id of the sensor to remove form the simulation.
     */
    void SensorManager::removeSensor(unsigned long index) {
      BaseSensor* tmpSensor = NULL;
      iMutex.lock();
      map<unsigned long, BaseSensor*>::iterator iter = simSensors.find(index);
      if (iter != simSensors.end()) {
        tmpSensor = iter->second;
        simSensors.erase(iter);
        if (tmpSensor)
          delete tmpSensor;
      }
      iMutex.unlock();

      control->sim->sceneHasChanged(false);
    }


    /**
     * \brief This function returns the SimSensor object for a given index.
     *
     * \param name The index of the sensor to get the core sensor object.
     *
     * \returns Returns a pointer to the corresponding sensor object.
     */
    BaseSensor* SensorManager::getSimSensor(unsigned long index) const {
      MutexLocker locker(&iMutex);
      map<unsigned long, BaseSensor*>::const_iterator iter = simSensors.find(index);

      if (iter != simSensors.end())
        return iter->second;
      else
        return NULL;
    }

    /**
     * \brief This function provides the sensor data for a given index.
     *
     * \param data The sensor data of the sensor.
     *
     * \param index The index of the sensor to get the data
     */
    int SensorManager::getSensorData(unsigned long id, sReal **data) const {
      MutexLocker locker(&iMutex);
      map<unsigned long, BaseSensor*>::const_iterator iter;

      iter = simSensors.find(id);
      if (iter != simSensors.end())
        return iter->second->getSensorData(data);

      LOG_DEBUG("Cannot Find Sensor wirh id: %lu\n",id);
      return 0;
    }


    /**
     *\brief Returns the number of sensors that are currently present in the simulation.
     *
     *\return The number of all sensors.
     */
    int SensorManager::getSensorCount() const {
      MutexLocker locker(&iMutex);
      return simSensors.size();
    }


    /**
     * \brief Destroys all sensors in the simulation.
     *
     * \details The \c clear_all flag indicates if the reload sensors should
     * be destroyed as well. If set to \c false they are left intact.
     *
     * \param clear_all Indicates if the reload sensors should
     * be destroyed as well. If set to \c false they are left intact.
     */
    void SensorManager::clearAllSensors(bool clear_all) {
      MutexLocker locker(&iMutex);
      map<unsigned long, BaseSensor*>::iterator iter;
      for(iter = simSensors.begin(); iter != simSensors.end(); iter++) {
        assert(iter->second);
        BaseSensor *sensor = iter->second;
        delete sensor;
      }
      simSensors.clear();
      if(clear_all) simSensorsReload.clear();
      next_sensor_id = 1;
    }


    /**
     * \brief This function reloads all sensors from a temporary sensor pool.
     *
     * \details All sensors that have been added with \c reload value as \c true
     * are added back to the simulation again with a \c reload value of \c true.
     */
    void SensorManager::reloadSensors(void) {
  
      vector<SensorReloadHelper>::iterator iter;
      iMutex.lock();
      for(iter=simSensorsReload.begin(); iter!=simSensorsReload.end(); ++iter) {
        iMutex.unlock();
    
        createAndAddSensor(iter->type, iter->config, true);
        iMutex.lock();
      }
      iMutex.unlock();
    }

    void SensorManager::addMarsParser(const std::string string,
				      BaseConfig* (*func)(ControlCenter*, ConfigMap*)){
      marsParser.insert(std::pair<const std::string, BaseConfig* (*)(ControlCenter*, ConfigMap*)>(string,func));
    }

    void SensorManager::addSensorType(const std::string &name, BaseSensor* (*func)(ControlCenter*, BaseConfig*)){
      availableSensors.insert(std::pair<const std::string,BaseSensor* (*)(ControlCenter*,BaseConfig*)>(name,func));
    }

    BaseSensor* SensorManager::createAndAddSensor(const std::string &type_name,
                                                  BaseConfig *config,
						  bool reload){
      assert(config);
      std::map<const std::string,BaseSensor* (*)(ControlCenter*, BaseConfig*)>::iterator it = availableSensors.find(type_name);
      if(it == availableSensors.end()){
        std::cerr << "Could not load unknown Sensor with name: \"" << type_name.c_str()<< "\"" << std::endl;
        return 0;
      }

      int id = -1;
      iMutex.lock();
      id = next_sensor_id++;
      iMutex.unlock();
  
      if(config->name.empty()){
        std::stringstream str;
        str << "SENSOR-" << id;
        config->name = str.str();
      }
      config->id = id;
      BaseSensor *sensor = ((*it).second)(this->control,config);
      iMutex.lock();
      simSensors[id] = sensor;
      iMutex.unlock();
  
      if(!reload) {
        simSensorsReload.push_back(SensorReloadHelper(type_name, config));
      }
  
      return sensor;
    }

    BaseSensor* SensorManager::createAndAddSensor(ConfigMap *config,
                                                  bool reload) {

      std::string type = (*config)["type"][0].getString();
      std::map<const std::string,BaseConfig* (*)(ControlCenter*, ConfigMap*)>::iterator it = marsParser.find(type);

      if(it == marsParser.end()){
        std::cerr << "Could not find MarsParser for sensor with name: \"" << type.c_str()<< "\"" << std::endl;
        return 0;
      }
      //LOG_DEBUG("found sensor: %s", type.c_str());
      BaseConfig *cfg = ((*it).second)(control, config);
      cfg->name = (*config)["name"][0].getString();
      return createAndAddSensor(type, cfg);
    }

  } // end of namespace sim
} // end of namespace mars
