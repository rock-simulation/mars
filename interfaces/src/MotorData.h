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

#ifndef MARS_INTERFACES_MOTOR_DATA_H
#define MARS_INTERFACES_MOTOR_DATA_H

#include <string>
#include "MARSDefs.h" //for sReal
#include <configmaps/ConfigData.h>

namespace mars {
  namespace interfaces {

    // forward declaration
    class LoadCenter;

    /**
     * MotorData is a struct to exchange motor information between the GUI and the simulation
     */
    class MotorData {
    public:
      explicit MotorData(const std::string& name = "",
                         MotorType type = MOTOR_TYPE_UNDEFINED);

//      MotorData& operator= (const MotorData& other);

      /**
       * @brief initialize motor struct with zero values.
       *
       * mainly kept for compatibility with ZERO_MOTOR_STRUCT macro
       */
      void init(const std::string& name = "", MotorType type = MOTOR_TYPE_UNDEFINED);

      bool fromConfigMap(configmaps::ConfigMap *config, std::string filenamePrefix,
                         LoadCenter *loadCenter = 0);
      void toConfigMap(configmaps::ConfigMap *config,
                       bool skipFilenamePrefix = false);
      void getFilesToSave(std::vector<std::string> *fileList);

      std::string name;         // name of the motor
      std::string jointName;    // Name of the joint to which the motor should be attached
      unsigned long index; // index number of the motor
      unsigned long jointIndex, jointIndex2; // index of the joint the motor moves
      int axis; // index of the joints axis the motor moves
      sReal maxSpeed; // maximum speed the motor can reach
      sReal maxEffort; // maximum force/torque [F/Nm] the motor can apply
      sReal maxAcceleration; // maximum acceleration the motor can produce
      sReal minSpeed;
      MotorType type; // motor type
      sReal p;  // p part of the controller
      sReal i;  // i part of the controller
      sReal d;  // d part of the controller
      sReal value; // initial / current control parameter value
      sReal maxValue; // maximum position the motor can reach
      sReal minValue; // minimum position the motor can reach
      configmaps::ConfigMap config;

      static std::map<std::string, std::string> legacynames;

      static std::map<std::string, std::string> init_legacynames() {
        std::map<std::string, std::string> tmpmap;
        tmpmap["maxEffort"] = "motorMaxForce";
        tmpmap["maxSpeed"] = "maximumVelocity";
        tmpmap["maxPosition"] = "maxValue";
        tmpmap["minPosition"] = "minValue";
        return tmpmap;
      }
    }; // end of class MotorData

  } // end of namespace interfaces
} // end of namespace mars

#endif /* MARS_INTERFACES_MOTOR_DATA_H */
