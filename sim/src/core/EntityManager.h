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
 * \file EntityManager.h
 * \author Jonas Peter, Kai von Szadkowski
 * \brief "EntityManager" is the class that manages information about simulation entities
 * 
 * The class can be used to access information about nodes and the position of the robot
 * to use pass robot name as parameter to the SimulatorInterface::loadScene function
 *
 * TODO delete robots after use
 * TODO handle node deletion (see NodeManager)
 * TODO allow to add nodes to robots via their ids instead of names;
 * TODO possible optimization store the index(iterator) of the last accessed robot as 
 *      it is likely to be accessed again while nodes and motors are added
 */

#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

#include <map>
#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/interfaces/graphics/GraphicsEventClient.h>
#include <mars/interfaces/sim/EntityManagerInterface.h>
#include <mars/utils/Mutex.h>
#include <configmaps/ConfigData.h>

namespace mars {
  namespace sim {

    class SimEntity;

    /*get notifications
     * about selection changes*/
    class EntityManager: public interfaces::GraphicsEventClient,
        public interfaces::EntityManagerInterface {
    public:
      EntityManager(interfaces::ControlCenter *c);
      /**creates a new entity with the given name and returns its id*/
      virtual unsigned long addEntity(const std::string &name);

      virtual unsigned long addEntity(SimEntity* entity);

      /**adds a node and maps the nodeId to the name*/
      virtual void addNode(const std::string &entityName, unsigned long nodeId,
          const std::string &nodeName);

      /**adds a motor to the entity and maps the motorId to its name*/
      virtual void addMotor(const std::string &entityName, unsigned long motorId,
          const std::string &motorName);

      /**adds a controller id to the controller list*/
      virtual void addController(const std::string &entityName, unsigned long controllerId);

      /**adds a joint to the entity and maps the jointId to its name*/
      virtual void addJoint(const std::string &entityName, unsigned long jointId,
          const std::string &jointName);

      /**returns the entity with the given name
       */
      // callback for entity creation
      virtual const std::map<unsigned long, SimEntity*>* subscribeToEntityCreation(interfaces::EntitySubscriberInterface* newsub);

      virtual SimEntity* getEntity(const std::string &name);

      /**returns the entity with the given id
       */
      virtual SimEntity* getEntity(unsigned long id);

      virtual unsigned long getEntityNode(const std::string &entityName,
          const std::string &nodeName);

      virtual unsigned long getEntityMotor(const std::string &entityName,
          const std::string &motorName);

      virtual std::vector<unsigned long> getEntityControllerList(const std::string &entityName);

      /**returns the node of the given entity; returns 0 if the entity or the node don't exist*/
      virtual unsigned long getEntityJoint(const std::string &entityName,
          const std::string &jointName);

      //from graphics event client
      virtual void selectEvent(unsigned long id, bool mode);

      //debug functions
      virtual void printEntityNodes(const std::string &entityName);
      virtual void printEntityMotors(const std::string &entityName);
      virtual void printEntityControllers(const std::string &entityName);
      virtual void resetPose();

    private:
      std::vector<interfaces::EntitySubscriberInterface*> subscribers;
      void notifySubscribers(SimEntity* entity);
      interfaces::ControlCenter *control;
      /**the id assigned to the next created entity; use getNextId function*/
      unsigned long next_entity_id;
      std::map<unsigned long, SimEntity*> entities;

      /**returns the id to be assigned to the next entity*/
      unsigned long getNextId() {
        return next_entity_id++;
      }

      // a mutex for the sensor containers
      mutable utils::Mutex iMutex;

    };

  } // end of namespace sim
} // end of namespace mars

#endif // ENTITYMANAGER_H
