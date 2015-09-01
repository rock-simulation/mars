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
 * \file SimEntity.h
 * \author Jonas Peter
 * \brief "SimEntity" is the class that contains information about entities
 *
 */

#ifndef SIMENTITY_H
#define SIMENTITY_H

#include <configmaps/ConfigData.h>
#include <string>
#include <set>
#include <map>
#include <vector>

namespace mars {

  namespace interfaces {
    class ControlCenter;
  }
  namespace sim {

    class SimEntity {
    public:
      SimEntity(const std::string& name);
      SimEntity(const configmaps::ConfigMap& parameters);
      SimEntity(interfaces::ControlCenter *c, const std::string& name);
      SimEntity(interfaces::ControlCenter *c,
                const configmaps::ConfigMap& parameters);

      void appendConfig(const configmaps::ConfigMap& parameters);

      void removeEntity();

      // adds a node to the robot
      void addNode(unsigned long nodeId, const std::string& name);

      // adds a joint to the robot
      void addJoint(long unsigned int jointId, const std::string& name);

      // adds a motor to the robot
      void addMotor(unsigned long motorId, const std::string& name);

      // adds a controller to the robot
      void addController(unsigned long controllerId);

      // adds a sensor to the robot
      void addSensor(unsigned long sensorId, const std::string& name);

      /**notify the robot that a node has been selected
       * the robot will check if the node belongs to it
       * \return true if the node belongs to the robot; false otherwise
       */
      bool select(unsigned long nodeId);

      /**notify the robot that a node has been deselected
       * the robot will check if the node belongs to it
       * \return true if the node belongs to the robot; false otherwise
       */
      bool deSelect(unsigned long nodeId);

      // returns the name of the robot
      std::string getName() {
        return name;
      }

      // returns true if the robot is selected
      bool isSelected() {
        return selected;
      }

      // returns true if the node is part of the robot
      bool belongsToRobot(unsigned long nodeId);

      /**returns the id of the node with the given name
       * with the current implementation this is slow
       */
      unsigned long getNode(const std::string &name);

      // returns the name of the node with the given id
      std::string getNode(unsigned long id);

      /**returns the id of the motor with the given name
       * with the current implementation this is slow O(n)
       */
      unsigned long getMotor(const std::string &name);

      // returns the name of the motor with the given id
      std::string getMotor(long unsigned int id);

      std::vector<unsigned long> getControllerList();

      unsigned long getJoint(const std::string &name);

      std::string getJoint(unsigned long id);

      void setInitialPose(bool reset=false);

      //debug functions
      void printNodes();
      void printMotors();
      void printControllers();

    private:
      std::string name;
      interfaces::ControlCenter *control;
      configmaps::ConfigMap config;

      // stores the ids of the nodes belonging to the robot
      std::map<unsigned long, std::string> nodeIds;

      // stores the ids of the motors belonging to the robot
      std::map<unsigned long, std::string> motorIds;

      // stores the ids of the controllers belonging to the robot
      std::vector<unsigned long> controllerIds;

      // stores the ids of the sensors belonging to the robot
      std::map<unsigned long, std::string> sensorIds;

      // stores the ids of the joints belonging to the robot
      std::map<unsigned long, std::string> jointIds;

      // the nodes that are currently selected
      std::set<unsigned long> selectedNodes;

      // the selection state of the robot; true if selected, false otherwise
      bool selected;

    };

  } // end of namespace sim
} // end of namespace mars

#endif // SIMENTITY_H
