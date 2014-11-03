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

#include "SimEntity.h"
#include <mars/utils/ConfigData.h>
#include <iostream>

#include <iterator> // ostream_iterator

namespace mars {
  namespace sim {

    SimEntity::SimEntity(const std::string &name)
    {
      this->name = name;
      selected = false;
    }

    SimEntity::SimEntity(const utils::ConfigMap& parameters) {
        config = parameters;
        this->name = (std::string)config["name"];
        this->selected = false;
    }

    void SimEntity::appendConfig(const utils::ConfigMap& parameters) {
    	utils::ConfigMap map = parameters;
    	config.append(map);
    }

    void SimEntity::removeEntity() {
//        for (std::vector<NodeId>::iterator it = oldNodeIDs.begin();
//                it != oldNodeIDs.end(); ++it) {
//            control->nodes->removeNode(*it);
//        }
//        oldNodeIDs.clear();
    }

    void SimEntity::addNode(unsigned long nodeId, const std::string& name)
    {
      nodeIds[nodeId] = name;
    }

    void SimEntity::addMotor(unsigned long motorId, const std::string& name)
    {
      motorIds[motorId] = name;
    }

    void SimEntity::addController(long unsigned int controllerId)
    {
      controllerIds.push_back(controllerId);
    }

    void SimEntity::addJoint(long unsigned int jointId, const std::string& name)
    {
      jointIds[jointId] = name;
    }

    void SimEntity::addSensor(long unsigned int sensorId, const std::string& name)
    {
      sensorIds[sensorId] = name;
    }


    bool SimEntity::select(unsigned long nodeId)
    {
      //check if the node belongs to the robot
      //   std::map<unsigned long, std::string>::iterator it = nodeIds.find(nodeId);
      //node belongs to the robot
      if(nodeIds.count(nodeId)){
        //no need to check if it has been previously selected as std::set does that
        selectedNodes.insert(nodeId);
        return true;
      }else{
        return false;
      }
    }

    long unsigned int SimEntity::getNode(const std::string& name)
    {
      for ( std::map< unsigned long, std::string>::const_iterator iter = nodeIds.begin(); iter != nodeIds.end(); ++iter ){
        if(iter->second == name)
          return iter->first;
      }
      return 0;
    }


    std::string SimEntity::getNode(long unsigned int id)
    {
      //TODO problem if node does not exist
      return nodeIds.find(id)->second;
    }

    long unsigned int SimEntity::getMotor(const std::string& name)
    {
      for ( std::map< unsigned long, std::string>::const_iterator iter = motorIds.begin(); iter != motorIds.end(); ++iter ){
        if(iter->second == name)
          return iter->first;
      }
      return 0;
    }

    std::string SimEntity::getMotor(long unsigned int id)
    {
      //TODO problem if node does not exist
      return motorIds.find(id)->second;
    }

    long unsigned int SimEntity::getJoint(const std::string& name)
    {
      for ( std::map< unsigned long, std::string>::const_iterator iter = jointIds.begin(); iter != jointIds.end(); ++iter ){
        if(iter->second == name)
          return iter->first;
      }
      return 0;
    }

    std::string SimEntity::getJoint(long unsigned int id)
    {
      //TODO problem if node does not exist
      return jointIds.find(id)->second;
    }

    std::vector< unsigned long > SimEntity::getControllerList()
    {
      return controllerIds;
    }

    void SimEntity::printNodes()
    {
      std::cout << "Nodes of Robot " << name << "with id: " << ":\n";
      for ( std::map< unsigned long, std::string>::const_iterator iter = nodeIds.begin(); iter != nodeIds.end(); ++iter ){
        std::cout << iter->first << '\t' << iter->second << '\n';
      }
      std::cout << std::endl;
    }

    void SimEntity::printMotors()
    {
      std::cout << "Motors of Robot " << name << ":\n";
      for ( std::map< unsigned long, std::string>::const_iterator iter = motorIds.begin(); iter != motorIds.end(); ++iter ){
        std::cout << iter->first << '\t' << iter->second << '\n';
      }
      std::cout << std::endl;
    }

    void SimEntity::printControllers()
    {
      std::cout << "Controllers of Robot " << name << ":\n";
      for(size_t i = 0; i < controllerIds.size(); i++){
        std::cout << controllerIds.at(i) << std::endl;
      }
    }

  } // end of namespace sim
} // end of namespace mars
