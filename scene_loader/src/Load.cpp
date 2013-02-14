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

#include "Load.h"
#include "zipit.h"


#include <QtXml>
#include <QDomNodeList>

#include <mars/data_broker/DataBrokerInterface.h>

#include <mars/interfaces/sim/SimulatorInterface.h>
#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/sim/JointManagerInterface.h>
#include <mars/interfaces/sim/SensorManagerInterface.h>
#include <mars/interfaces/sim/MotorManagerInterface.h>
#include <mars/interfaces/sim/ControllerManagerInterface.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>

#include <mars/interfaces/sim/EntityManagerInterface.h>
#include <mars/interfaces/sim/LoadSceneInterface.h>
#include <mars/utils/misc.h>

namespace mars {
  namespace scene_loader {

    using namespace std;
    using namespace interfaces;

    unsigned long Load::hack_ids = 0;

    Load::Load(std::string fileName, ControlCenter *c,
               std::string tmpPath_, const std::string &robotname) :
      mFileName(fileName), mRobotName(robotname),
      control(c), tmpPath(tmpPath_) {
    }

    unsigned int Load::load() {
      unsigned int fileNameIndex;
      LoadSceneInterface *loadScene = control->loadCenter->loadScene;
      std::string filename = mFileName;

      hack_ids++;

      if(mRobotName != ""){
        control->entities->addEntity(mRobotName);
      }

      // need to unzip into a temporary directory
      std::string suffix = utils::getFilenameSuffix(mFileName);
      if (suffix == ".scn" || suffix == ".zip") {
        if(unzip(tmpPath, mFileName) == 0)
          return 0;
      }
      else {
        // can parse file without unzipping
        tmpPath = utils::getPathOfFile(mFileName);
      }

      utils::removeFilenamePrefix(&filename);
      utils::removeFilenameSuffix(&filename);

      fileNameIndex = loadScene->getMappedSceneByName(mFileName);
      if (fileNameIndex == 0) {
        loadScene->setMappedSceneName(mFileName);
        fileNameIndex = loadScene->getMappedSceneByName(mFileName);
      }

      parseScene(tmpPath, tmpPath + filename + ".scene", fileNameIndex);

      return 1;
    }

    unsigned int Load::unzip(const std::string& destinationDir,
                             const std::string& zipFilename) {
      if(!utils::createDirectory(destinationDir)) return 0;

      Zipit myZipFile(zipFilename);
      LOG_INFO("Load: unzipping scene: %s", zipFilename.c_str());

      if (!myZipFile.unpackWholeZipTo(destinationDir)) return 0;

      return 1;
    }

    unsigned int Load::parseScene(const std::string& sceneDirectory,
                                  const std::string& sceneFilename,
                                  unsigned int mapIndex) {
      //  HandleFileNames h_filenames;
      vector<string> v_filesToLoad;
      QString xmlErrorMsg="";
      int xmlErrorLine, xmlErrorCol =0;

      //creating a handle for the xmlfile
      QFile file(sceneFilename.c_str());

      QLocale::setDefault(QLocale::C);

      LOG_INFO("Load: loading scene: %s", sceneFilename.c_str());

      //test to open the xmlfile
      if (!file.open(QIODevice::ReadOnly)) {
        std::cout<<"Error while opening scene file content "
                 << sceneFilename << " in Load.cpp->parseScene"
                 << std::endl;
        std::cout << "Make sure your scenefile name corresponds to"
                  << " the name given to the enclosed .scene file"
                  << std::endl;
        return 0;
      }

      //test to pass the content from the xmlfile to the DOM-Object
      QDomDocument doc;
      if (!doc.setContent(&file, false, &xmlErrorMsg,
                          &xmlErrorLine, &xmlErrorCol)) {
        file.close();
        std::cout<<"error passing the file content in->Load.cpp->parseScene"
                 <<std::endl;
        std::cout<<"Message: "<<xmlErrorMsg.toStdString()<<"\n"<<"Line: "
                 <<xmlErrorLine<<"\n"<<"Collumn: "
                 <<xmlErrorCol<<"\n"<<std::endl;
        return 0;
      }

      //the XML-file has been opend and is ready for parsing it's content
      //the first element is the "root" element
      QDomElement root = doc.documentElement();
      QDomElement tmpElement;
      double version = 0.0;

      QDomNodeList xmlnodelist = root.elementsByTagName(QString("version"));
      if (!xmlnodelist.isEmpty()) {
        tmpElement=xmlnodelist.at(0).toElement();
        version = tmpElement.text().toDouble();
      }
      if(version < 0.199999) {
        LOG_WARN("Load: old SceneFile version. Take care of the following changes:");
        LOG_WARN("\t- the sphere object get radius as size instead of diameter");
        LOG_WARN("\t- the origin numbers for primitives are replaced by strings (box, shpere, etc.)");
        LOG_WARN("\t- the sensor type is a string instead of a number, like NodePosition, JointPosition, or NodeCOM");
        LOG_WARN("\t- the JOint6DOF sensor gets nodeID and jointID instead of id list");
        LOG_WARN("\t- the actual_pos and acutal_rot is removed from nodes");
      }

      xmlnodelist = root.elementsByTagName(QString("material"));
      for (int i=0; i<xmlnodelist.size(); i++) {
        tmpElement = xmlnodelist.at(i).toElement();
        if (parseMaterial(&tmpElement, mapIndex)==0) {
          LOG_ERROR("Load:: parseMaterial failed");
          file.close();
          return 0;
        }
      }

      //first checking wether there is a node with name "node"
      //by passing it in a xmlnodelist and checking if it's not empty.
      //if so, there is at least an element with name "node"
      xmlnodelist = root.elementsByTagName(QString("node"));
      for (int i=0; i<xmlnodelist.size(); i++) {
        tmpElement = xmlnodelist.at(i).toElement();
        if (parseNode(&tmpElement, mapIndex)==0) {
          std::cout<<"error while parsing in->Load.cpp->parseScene" <<std::endl;
          file.close();
          return 0;
        }
      }

      //first checking wether there is a node with name "joint"
      //by passing it in a xmlnodelist and checking if it's not empty.
      //if so, there is at least an element with name "joint"
      xmlnodelist = root.elementsByTagName(QString("joint"));
      if (!xmlnodelist.isEmpty()) {
        for (int i=0; i<xmlnodelist.size(); i++) {
          tmpElement = xmlnodelist.at(i).toElement();
          if (parseJoint(&tmpElement, mapIndex)==0) {
            LOG_ERROR("Load::parseJoint returned error");
            file.close();
            return 0;
          }
        }
      }

      xmlnodelist = root.elementsByTagName(QString("motor"));//motor
      if (!xmlnodelist.isEmpty()) {
        for (int i=0; i<xmlnodelist.size(); i++) {
          tmpElement = xmlnodelist.at(i).toElement();
          if (parseMotor(&tmpElement, mapIndex)==0) {
            LOG_ERROR("Load::parseMotor returned error");
            file.close();
            return 0;
          }
        }
      }

      xmlnodelist = root.elementsByTagName(QString("light"));//lights
      if (!xmlnodelist.isEmpty()) {
        for (int i=0; i<xmlnodelist.size(); i++) {
          tmpElement = xmlnodelist.at(i).toElement();
          if (parseLight(&tmpElement, mapIndex)==0) {
            LOG_ERROR("Load::parseLight returned error");
            file.close();
            return 0;
          }
        }
      }

      xmlnodelist = root.elementsByTagName(QString("sensor"));//sensor
      if (!xmlnodelist.isEmpty()) {
        //now generating a vector of sensorstructs to store the extracted datas
        for (int i=0; i<xmlnodelist.size(); i++) {
          tmpElement = xmlnodelist.at(i).toElement();
          if (parseSensor(&tmpElement, mapIndex)==0) {
            LOG_ERROR("Load::parseSensor returned error");
            file.close();
            return 0;
          }
        }
      }

      xmlnodelist = root.elementsByTagName(QString("controller"));
      if (!xmlnodelist.isEmpty()) {
        for (int i=0; i<xmlnodelist.size(); i++) {
          tmpElement = xmlnodelist.at(i).toElement();
          if (parseController(&tmpElement, mapIndex)==0) {
            LOG_ERROR("Load::parseController returned error");
            file.close();
            return 0;
          }
        }
      }

      xmlnodelist = root.elementsByTagName(QString("graphicOptions"));//graphicoptions
      if (!xmlnodelist.isEmpty()) {
        tmpElement = xmlnodelist.at(0).toElement();
        if (parseGraphic(&tmpElement, mapIndex)==0) {
          LOG_ERROR("Load::parseGraphic returned error");
          file.close();
          return 0;
        }
      }

      file.close();
      return 1;
    }

    BaseSensor* Load::parseSensor(QDomElement * elementNode,
                                  unsigned int mapIndex) {

      utils::ConfigMap config;
      config["mapIndex"].push_back(utils::ConfigItem(mapIndex));
      getGenericConfig(&config, elementNode);
      unsigned long sceneID = config["index"][0].getULong();
      BaseSensor *sensor = control->sensors->createAndAddSensor(&config);
      if (sensor != 0) {
        control->loadCenter->loadScene->setMappedID(sceneID, sensor->getID(),
                                                    MAP_TYPE_SENSOR,
                                                    mapIndex);
      }

      return sensor;
    }

    unsigned int Load::parseMaterial(QDomElement * elementNode,
                                     unsigned int mapIndex) {
      MaterialData material;
      unsigned long id;
      utils::ConfigMap config;
      getGenericConfig(&config, elementNode);

      int valid = material.fromConfigMap(&config, tmpPath);
      if((id = config["id"][0].getULong())) {
        materials[id] = material;
      }

      return valid;
    }

    unsigned int Load::parseNode(QDomElement * elementNode,
                                 unsigned int mapIndex) {

      NodeData node;
      utils::ConfigMap config;
      config["mapIndex"].push_back(utils::ConfigItem(mapIndex));
      getGenericConfig(&config, elementNode);

      int valid = node.fromConfigMap(&config, tmpPath, control->loadCenter->loadScene);
      if(!valid) return 0;

      // handle material
      utils::ConfigMap::iterator it;
      if((it = config.find("material_id")) != config.end()) {
        unsigned long id = it->second[0].getULong();
        if(id) {
          std::map<unsigned long, MaterialData>::iterator it = materials.find(id);
          if(it != materials.end())
            node.material = it->second;
        }
      }

      // the group ids could be also handled in the NodeData by the mapIndex
      if(node.groupID)
        node.groupID += hack_ids*10000;

      NodeId oldId = node.index;
      NodeId newId = control->nodes->addNode(&node);
      if(!newId) {
        LOG_ERROR("addNode returned 0");
        return 0;
      }
      control->loadCenter->loadScene->setMappedID(oldId, newId, MAP_TYPE_NODE, mapIndex);

      if(mRobotName != "") {
        control->entities->addNode(mRobotName, node.index, node.name);
      }
      return true;
    }

    unsigned int Load::parseJoint(QDomElement * elementNode,
                                  unsigned int mapIndex) {

      JointData joint;
      utils::ConfigMap config;
      config["mapIndex"].push_back(utils::ConfigItem(mapIndex));
      getGenericConfig(&config, elementNode);

      int valid = joint.fromConfigMap(&config, tmpPath, control->loadCenter->loadScene);
      if(!valid) return 0;

      JointId oldId = joint.index;
      JointId newId = control->joints->addJoint(&joint);
      if(!newId) {
        LOG_ERROR("addJoint returned 0");
        return 0;
      }
      control->loadCenter->loadScene->setMappedID(oldId, newId,
                                                  MAP_TYPE_JOINT, mapIndex);

      if(mRobotName != "") {
        control->entities->addJoint(mRobotName, joint.index, joint.name);
      }
      return true;
    }

    unsigned int Load::parseMotor(QDomElement * elementNode,
                                  unsigned int mapIndex) {

      MotorData motor;
      utils::ConfigMap config;
      config["mapIndex"].push_back(utils::ConfigItem(mapIndex));
      getGenericConfig(&config, elementNode);

      int valid = motor.fromConfigMap(&config, tmpPath, control->loadCenter->loadScene);
      if(!valid) return 0;

      MotorId oldId = motor.index;
      MotorId newId = control->motors->addMotor(&motor);
      if(!newId) {
        LOG_ERROR("addMotor returned 0");
        return 0;
      }
      control->loadCenter->loadScene->setMappedID(oldId, newId,
                                                  MAP_TYPE_MOTOR, mapIndex);

      if(mRobotName != "") {
        control->entities->addMotor(mRobotName, motor.index, motor.name);
      }
      return true;
    }

    unsigned int Load::parseLight(QDomElement * elementNode,
                                  unsigned int mapIndex) {

      LightData light;
      utils::ConfigMap config;
      config["mapIndex"].push_back(utils::ConfigItem(mapIndex));
      getGenericConfig(&config, elementNode);

      int valid = light.fromConfigMap(&config, tmpPath, control->loadCenter->loadScene);
      if(!valid) return 0;

      control->sim->addLight(light);
      return true;
    }

    unsigned int Load::parseGraphic(QDomElement * elementNode,
                                    unsigned int mapIndex) {

      GraphicData graphic;
      utils::ConfigMap config;
      config["mapIndex"].push_back(utils::ConfigItem(mapIndex));
      getGenericConfig(&config, elementNode);

      int valid = graphic.fromConfigMap(&config, tmpPath, control->loadCenter->loadScene);
      if(!valid) return 0;

      if (control->graphics)
        control->graphics->setGraphicOptions(graphic);

      return true;
    }

    unsigned int Load::parseController(QDomElement * elementNode,
                                       unsigned int mapIndex) {

      ControllerData controller;
      utils::ConfigMap config;
      config["mapIndex"].push_back(utils::ConfigItem(mapIndex));
      getGenericConfig(&config, elementNode);

      int valid = controller.fromConfigMap(&config, tmpPath,
                                           control->loadCenter->loadScene);
      if(!valid) return 0;

      MotorId oldId = controller.id;
      MotorId newId = control->controllers->addController(controller);
      if(!newId) {
        LOG_ERROR("Load: addController returned 0");
        return 0;
      }
      control->loadCenter->loadScene->setMappedID(oldId, newId,
                                                  MAP_TYPE_CONTROLLER,
                                                  mapIndex);
      if(mRobotName != "") {
        control->entities->addController(mRobotName, newId);
      }

      return true;
    }

    void Load::getGenericConfig(utils::ConfigMap *config,
                                QDomElement * elementNode) {

      QDomNodeList xmlnodepartlist=elementNode->childNodes();
      QDomNamedNodeMap attributes = elementNode->attributes();

      std::string tagName, value;
      QDomElement child;
      QDomNode child2;

      for(int i=0; i<attributes.size(); i++) {
        child2 = attributes.item(i);
        tagName = child2.nodeName().toStdString();
        value = child2.nodeValue().toStdString();
        if(!tagName.empty()) {
          (*config)[tagName].push_back(utils::ConfigItem(value));
#ifdef DEBUG_PARSE_SENSOR
          LOG_DEBUG("attrib [%s : %s]", tagName.c_str(), value.c_str());
#endif
        }
      }

      for (int i=0; i<xmlnodepartlist.size(); i++) {
        child = xmlnodepartlist.at(i).toElement();

        tagName = child.tagName().toStdString();
        value = child.text().toStdString();
        if(!tagName.empty()) {
          (*config)[tagName].push_back(utils::ConfigItem(value));
#ifdef DEBUG_PARSE_SENSOR
          LOG_DEBUG("element [%s : %s]", tagName.c_str(), value.c_str());
#endif
          getGenericConfig(&((*config)[tagName].back().children), &child);
        }
      }

      // we can or should also iterate over the attributes
    }

  } // end of namespace scene_loader
} // end of namespace mars
