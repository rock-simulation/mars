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
 * \file RobotManagerInterface.h
 * \author Jonas Peter
 * \brief "RobotManagerInterface" is an interface to load manage Robots
 *        into the simulation
 *
 */

#ifndef MARS_INTERFACES_ENTITYMANAGER_INTERFACE_H
#define MARS_INTERFACES_ENTITYMANAGER_INTERFACE_H

#include <string>

namespace mars {
  
  namespace sim {
    class SimEntity;
  }

  namespace interfaces {

    class EntityManagerInterface
    {
    public:
      virtual ~EntityManagerInterface() {}
    
      /**creates a new entity with the given name and returns its id*/
      virtual unsigned long addEntity(const std::string &name) = 0;
    
      /**adds a node to the entity and maps the nodeId to its name*/    
      virtual void addNode(const std::string &entityName, unsigned long nodeId, const std::string &nodeName) = 0;
    
      /**adds a motor to the entity and maps the motorId to its name*/
      virtual void addMotor(const std::string &entityName, unsigned long motorId, const std::string &motorName) = 0;
    
      /**adds a controller id to the controller list*/
      virtual void addController(const std::string &entityName, unsigned long controllerId) = 0;
    
      /**adds a joint to the entity and maps the jointId to its name*/
      virtual void addJoint(const std::string &entityName, unsigned long jointId, const std::string &jointName) = 0;
    
      //from graphics event client
      virtual void selectEvent(unsigned long id, bool mode) = 0;
    
      /**returns the entity with the given name
       */
      virtual sim::SimEntity* getEntity(const std::string &name) = 0;
    
      /**returns the entity with the given id*/
      virtual sim::SimEntity* getEntity(unsigned long id) = 0;
     
      /**returns the node of the given entity; returns 0 if the entity or the node don't exist*/
      virtual unsigned long getEntityNode(const std::string &entityName, const std::string &nodeName) = 0;
     
      virtual unsigned long getEntityMotor(const std::string &entityName, const std::string &motorName) = 0;
     
      virtual std::vector<unsigned long> getEntityControllerList(const std::string &entityName) = 0;

      /**returns the node of the given entity; returns 0 if the entity or the node don't exist*/
      virtual unsigned long getEntityJoint(const std::string &entityName, const std::string &jointName) = 0;
     
      //Debug functions
      virtual void printEntityNodes(const std::string &entityName) = 0;
      virtual void printEntityMotors(const std::string &entityName) = 0;
      virtual void printEntityControllers(const std::string &entityName) = 0;

    };

  } // end of namespace interfaces
} // end of namespace mars

#endif // MARS_INTERFACES_ENTITYMANAGER_INTERFACE_H
