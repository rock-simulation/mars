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
#include <mars/logging/Logging.hpp>

//#define DEBUG_PARSE_SENSOR 1

namespace mars {
  namespace scene_loader {

    using namespace std;
    using namespace interfaces;

    Load::Load(std::string fileName, ControlCenter *c,
               std::string tmpPath_, const std::string &robotname) :
      mFileName(fileName), mRobotName(robotname),
      control(c), tmpPath(tmpPath_) {
    	mFileSuffix = utils::getFilenameSuffix(mFileName);
    }

    unsigned int Load::load() {

      if(!prepareLoad()) return 0;
      if(!parseScene()) return 0;
      return loadScene();
    }

    unsigned int Load::prepareLoad() {
      std::string filename = mFileName;

      groupIDOffset = control->nodes->getMaxGroupID() + 1;

      if(mRobotName != ""){
        control->entities->addEntity(mRobotName);
      }

      // need to unzip into a temporary directory
      if (mFileSuffix == ".scn" || mFileSuffix == ".zip") {
        if(unzip(tmpPath, mFileName) == 0)
          return 0;
      }
      else {
        // can parse file without unzipping
        tmpPath = utils::getPathOfFile(mFileName);
      }

      utils::removeFilenamePrefix(&filename);
      utils::removeFilenameSuffix(&filename);

      mapIndex = control->loadCenter->getMappedSceneByName(mFileName);
      if (mapIndex == 0) {
        control->loadCenter->setMappedSceneName(mFileName);
        mapIndex = control->loadCenter->getMappedSceneByName(mFileName);
      }
      sceneFilename = tmpPath + filename + ".scene";
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

    unsigned int Load::parseScene() {
      checkEncodings();
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

      //first checking wether there is a node with name "node"
      //by passing it in a xmlnodelist and checking if it's not empty.
      //if so, there is at least an element with name "node"
      xmlnodelist = root.elementsByTagName(QString("node"));
      for (int i=0; i<xmlnodelist.size(); i++) {
        getGenericConfig(&nodeList, xmlnodelist.at(i).toElement());
      }

      xmlnodelist = root.elementsByTagName(QString("material"));
      for (int i=0; i<xmlnodelist.size(); i++) {
        getGenericConfig(&materialList, xmlnodelist.at(i).toElement());
      }

      xmlnodelist = root.elementsByTagName(QString("joint"));
      for (int i=0; i<xmlnodelist.size(); i++) {
        getGenericConfig(&jointList, xmlnodelist.at(i).toElement());
      }

      xmlnodelist = root.elementsByTagName(QString("motor"));//motor
      for (int i=0; i<xmlnodelist.size(); i++) {
        getGenericConfig(&motorList, xmlnodelist.at(i).toElement());
      }

      xmlnodelist = root.elementsByTagName(QString("light"));//lights
      for (int i=0; i<xmlnodelist.size(); i++) {
        getGenericConfig(&lightList, xmlnodelist.at(i).toElement());
      }

      xmlnodelist = root.elementsByTagName(QString("sensor"));//sensor
      for (int i=0; i<xmlnodelist.size(); i++) {
        getGenericConfig(&sensorList, xmlnodelist.at(i).toElement());
      }

      xmlnodelist = root.elementsByTagName(QString("controller"));
      for (int i=0; i<xmlnodelist.size(); i++) {
        getGenericConfig(&controllerList, xmlnodelist.at(i).toElement());
      }

      xmlnodelist = root.elementsByTagName(QString("graphicOptions"));//graphicoptions
      if (!xmlnodelist.isEmpty()) {
        getGenericConfig(&graphicList, xmlnodelist.at(0).toElement());
      }

      file.close();

      return 1;
    }

    unsigned int Load::loadScene() {
      for(unsigned int i=0; i<materialList.size(); ++i) if(!loadMaterial(materialList[i])) return 0;
      for(unsigned int i=0; i<nodeList.size(); ++i) if(!loadNode(nodeList[i])) return 0;
      for(unsigned int i=0; i<jointList.size(); ++i) if(!loadJoint(jointList[i])) return 0;
      for(unsigned int i=0; i<motorList.size(); ++i) if(!loadMotor(motorList[i])) return 0;
      for(unsigned int i=0; i<sensorList.size(); ++i) if(!loadSensor(sensorList[i])) return 0;
      for(unsigned int i=0; i<controllerList.size(); ++i) if(!loadController(controllerList[i])) return 0;
      for(unsigned int i=0; i<graphicList.size(); ++i) if(!loadGraphic(graphicList[i])) return 0;
      for(unsigned int i=0; i<lightList.size(); ++i) if(!loadLight(lightList[i])) return 0;

      return 1;
    }

    unsigned int Load::loadMaterial(utils::ConfigMap config) {
      MaterialData material;
      unsigned long id;

      int valid = material.fromConfigMap(&config, tmpPath);
      if((id = config["id"][0].getULong())) {
        materials[id] = material;
      }

      return valid;
    }

    unsigned int Load::loadNode(utils::ConfigMap config) {
      NodeData node;
      config["mapIndex"].push_back(utils::ConfigItem(mapIndex));
      int valid = node.fromConfigMap(&config, tmpPath, control->loadCenter);
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
        node.groupID += groupIDOffset;

      NodeId oldId = node.index;
      NodeId newId = control->nodes->addNode(&node);
      if(!newId) {
        LOG_ERROR("addNode returned 0");
        return 0;
      }
      control->loadCenter->setMappedID(oldId, newId, MAP_TYPE_NODE, mapIndex);

      if(mRobotName != "") {
        control->entities->addNode(mRobotName, node.index, node.name);
      }
      return 1;
    }

    unsigned int Load::loadJoint(utils::ConfigMap config) {
      JointData joint;
      config["mapIndex"].push_back(utils::ConfigItem(mapIndex));
      int valid = joint.fromConfigMap(&config, tmpPath,
                                      control->loadCenter);
      if(!valid) {
        fprintf(stderr, "Load: error while loading joint\n");
        return 0;
      }

      JointId oldId = joint.index;
      JointId newId = control->joints->addJoint(&joint);
      if(!newId) {
        LOG_ERROR("addJoint returned 0");
        return 0;
      }
      control->loadCenter->setMappedID(oldId, newId,
                                       MAP_TYPE_JOINT, mapIndex);

      if(mRobotName != "") {
        control->entities->addJoint(mRobotName, joint.index, joint.name);
      }
      return true;
    }

    unsigned int Load::loadMotor(utils::ConfigMap config) {
      MotorData motor;
      config["mapIndex"].push_back(utils::ConfigItem(mapIndex));

      int valid = motor.fromConfigMap(&config, tmpPath, control->loadCenter);
      if(!valid) {
        fprintf(stderr, "Load: error while loading motor\n");
        return 0;
      }

      MotorId oldId = motor.index;
      MotorId newId = control->motors->addMotor(&motor);
      if(!newId) {
        LOG_ERROR("addMotor returned 0");
        return 0;
      }
      control->loadCenter->setMappedID(oldId, newId,
                                       MAP_TYPE_MOTOR, mapIndex);

      if(mRobotName != "") {
        control->entities->addMotor(mRobotName, motor.index, motor.name);
      }
      return true;
    }

    BaseSensor* Load::loadSensor(utils::ConfigMap config) {
      config["mapIndex"].push_back(utils::ConfigItem(mapIndex));
      unsigned long sceneID = config["index"][0].getULong();
      BaseSensor *sensor = control->sensors->createAndAddSensor(&config);
      if (sensor != 0) {
        control->loadCenter->setMappedID(sceneID, sensor->getID(),
                                         MAP_TYPE_SENSOR,
                                         mapIndex);
      }

      return sensor;
    }

    unsigned int Load::loadController(utils::ConfigMap config) {
      ControllerData controller;
      config["mapIndex"].push_back(utils::ConfigItem(mapIndex));

      int valid = controller.fromConfigMap(&config, tmpPath,
                                           control->loadCenter);
      if(!valid) {
        fprintf(stderr, "Load: error while loading Controller\n");
        return 0;
      }

      MotorId oldId = controller.id;
      MotorId newId = control->controllers->addController(controller);
      if(!newId) {
        LOG_ERROR("Load: addController returned 0");
        return 0;
      }
      control->loadCenter->setMappedID(oldId, newId,
                                       MAP_TYPE_CONTROLLER,
                                       mapIndex);
      if(mRobotName != "") {
        control->entities->addController(mRobotName, newId);
      }
      return 1;
    }

    unsigned int Load::loadGraphic(utils::ConfigMap config) {
      GraphicData graphic;
      config["mapIndex"].push_back(utils::ConfigItem(mapIndex));
      int valid = graphic.fromConfigMap(&config, tmpPath,
                                        control->loadCenter);
      if(!valid) {
        fprintf(stderr, "Load: error while loading graphic\n");
        return 0;
      }

      if (control->graphics)
        control->graphics->setGraphicOptions(graphic);

      return 1;
    }

    unsigned int Load::loadLight(utils::ConfigMap config) {
      LightData light;
      config["mapIndex"].push_back(utils::ConfigItem(mapIndex));
      int valid = light.fromConfigMap(&config, tmpPath,
                                      control->loadCenter);
      if(!valid) {
        fprintf(stderr, "Load: error while loading light\n");
        return 0;
      }

      control->sim->addLight(light);
      return true;
    }


    void Load::getGenericConfig(std::vector<utils::ConfigMap> *configList,
                                const QDomElement &elementNode) {
      utils::ConfigMap config;
      getGenericConfig(&config, elementNode);
      configList->push_back(config);
    }

    void Load::getGenericConfig(utils::ConfigMap *config,
                                const QDomElement &elementNode) {

      QDomNodeList xmlnodepartlist=elementNode.childNodes();
      QDomNamedNodeMap attributes = elementNode.attributes();

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
          getGenericConfig(&((*config)[tagName].back().children), child);
        }
      }

      // we can or should also iterate over the attributes
    }

    void Load::checkEncodings(){
        bool existing = true;
        QString str("<xml><easter_egg>3.1418</easter_egg></xml>");
        QString tmpFilename = QString(tmpPath.c_str()) + QString("mars-encoding-check");
        QFile tmpFile(tmpFilename);
        if(!QFile::exists(tmpFilename)){
            tmpFile.open(QIODevice::WriteOnly);
            QTextStream out(&tmpFile);
            out << str;
            tmpFile.close();
            existing = false;
        }
        if(!tmpFile.open(QIODevice::ReadOnly)){
            LOG_FATAL("Cannot open language checking file\n");
            exit(-1);
        }
        QDomDocument doc;
        if(!doc.setContent(&tmpFile, false)){
            LOG_FATAL("Cannot parse language checking file\n");
            exit(-2);
        }
        QDomElement root = doc.documentElement();
        QDomElement tmpElement;
        if(root.elementsByTagName(QString("easter_egg")).at(0).toElement().text().toDouble() != 3.1418){
            LOG_ERROR("Encoding of the system is invalid, therefore Scene loading will fail quitting here to prevent errors later");
            exit(-3);
        }
        tmpFile.close();
        if(!existing){
           tmpFile.remove();
        }
    }

  } // end of namespace scene_loader
} // end of namespace mars
