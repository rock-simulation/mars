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

//#define DEBUG_PARSE_SENSOR 1

namespace mars {
namespace urdf_loader {

using namespace std;
using namespace interfaces;

unsigned long Load::hack_ids = 0;

Load::Load(std::string fileName, ControlCenter *c, std::string tmpPath_,
        const std::string &robotname) :
        mFileName(fileName), mRobotName(robotname), control(c), tmpPath(
                tmpPath_) {
    mFileSuffix = utils::getFilenameSuffix(mFileName);
}

unsigned int Load::load() {

    if (!prepareLoad())
        return 0;
    if (!parseScene())
        return 0;
    return loadScene();
}

unsigned int Load::prepareLoad() {
    LoadSceneInterface *loadScene = NULL;
    std::string filename = mFileName;

    if (control)
        loadScene = control->loadCenter->loadScene[mFileSuffix];
    hack_ids++;

    if (mRobotName != "") {
        control->entities->addEntity(mRobotName);
    }

    // need to unzip into a temporary directory
    if (mFileSuffix == ".zsmurf") {
        if (unzip(tmpPath, mFileName) == 0)
            return 0;
    } else {
        // can parse file without unzipping
        tmpPath = utils::getPathOfFile(mFileName);
    }

    utils::removeFilenamePrefix(&filename);
    utils::removeFilenameSuffix(&filename);

    if (loadScene) {
        mapIndex = loadScene->getMappedSceneByName(mFileName);
        if (mapIndex == 0) {
            loadScene->setMappedSceneName(mFileName);
            mapIndex = loadScene->getMappedSceneByName(mFileName);
        }
    } else {
        mapIndex = 0;
    }
    sceneFilename = tmpPath + filename + ".smurf";
    return 1;
}

unsigned int Load::unzip(const std::string& destinationDir,
        const std::string& zipFilename) {
    if (!utils::createDirectory(destinationDir))
        return 0;

    Zipit myZipFile(zipFilename);
    LOG_INFO("Load: unsmurfing zipped SMURF: %s", zipFilename.c_str());

    if (!myZipFile.unpackWholeZipTo(destinationDir))
        return 0;

    return 1;
}

std::vector<double> Load::getPositionFromPose(boost::shared_ptr<urdf::Pose> pose, std::vector<double> &position) {
	position.push_back(pose->position.x);
	position.push_back(pose->position.y);
	position.push_back(pose->position.z);
	return position;
}

std::vector<double> Load::getRotationFromPose(boost::shared_ptr<urdf::Pose> pose, std::vector<double> &rotation) {
	for (int i = 0; i < 4; ++ i) {
		rotation.push_back(0.0);
	}
	pose->rotation.getQuaternion(rotation[0], rotation[1], rotation[2], rotation[3]);
	return rotation;
}

void Load::handleLink(boost::shared_ptr<urdf::Link> curlink, int &id) {
    utils::ConfigMap config;
    id++;
    config["index"].push_back(utils::ConfigItem(id));
    config["groupid"]
    //handle inertial
    config["mass"].push_back(utils::ConfigItem(curlink->inertial->mass));
    utils::ConfigMap inertia;
    inertia["i00"].push_back(utils::ConfigItem(curlink->inertial->ixx));
    inertia["i01"].push_back(utils::ConfigItem(curlink->inertial->ixy));
    inertia["i02"].push_back(utils::ConfigItem(curlink->inertial->ixz));
    inertia["i10"].push_back(utils::ConfigItem(curlink->inertial->iyy));
    inertia["i11"].push_back(utils::ConfigItem(curlink->inertial->iyz));
    inertia["i12"].push_back(utils::ConfigItem(curlink->inertial->izz));
    config["inertia"] = inertia;
    config["pivot"] = getVectorConfig(curlink->inertial->origin.position);

    //handle visual
    boost::shared_ptr<Visual> tmpVisual = curlink->visual;
    
    config["filename"]
    config["visualsize"]

    config["origname"]
           config["name"]

    //handle collision
    config["physicmode"] = collision.type
    config["extend"] = boundingbox.extend
    config["pivot"] = boundingbox.center
    config["movable"]
    //handle joint/parent information
    unsigned int parent_index = 0;
    for (unsigned int i = 0; i < nodeList.size(); ++i) { //TODO: Use iterator
        if (nodeList[i]["name"] == curlink->getParent()->name) {
            parent_index = nodeList[i]["index"];
            break;
        }
    }
    config["relativeid"] = parent_index;
    boost::shared_ptr<urdf::Joint> joint = curlink->parent_joint;
    config["position"] = getPositionFromPose(parent_joint.parent_to_joint_origin_transform);
    config["rotation"] = getRotationFromPoseparent_joint.parent_to_joint_origin_transform);
    nodeList.push_back(config);
    for (std::vector<boost::shared_ptr<urdf::Link>>::iterator it =
            curlink->child_links.begin(); it != curlink->child_links.end(); ++it) {
        handleLink(*it, id); //TODO: check if this is correct with shared_ptr
    }
}

unsigned int Load::parseScene() {
    //  HandleFileNames h_filenames;
    vector<string> v_filesToLoad;
    QString xmlErrorMsg = "";

    //creating a handle for the xmlfile
    QFile file(sceneFilename.c_str());

    QLocale::setDefault(QLocale::C);

    LOG_INFO("Load: loading scene: %s", sceneFilename.c_str());

    //test to open the xmlfile
    if (!file.open(QIODevice::ReadOnly)) {
        std::cout << "Error while opening scene file content " << sceneFilename
                << " in Load.cpp->parseScene" << std::endl;
        std::cout << "Make sure your scenefile name corresponds to"
                << " the name given to the enclosed .scene file" << std::endl;
        return 0;
    }

    boost::shared_ptr<urdf::ModelInterface> model = urdf::parseURDFFile(
            sceneFilename);
    if (!model) {
        return 0;
    }

    int id = 0;
    boost::shared_ptr<urdf::Link> root = model->getRoot();
    boost::shared_ptr<urdf::Link> link = root;
    handleLink(link, id);


//    //This "linear" parsing into MARS format might not be possible or at least not very practical,
//    //as transformations are safed in joints and there are no groupid, thus we have to traverse
//    //the entire tree recursively anyway
//    std::vector<boost::shared_ptr<urdf::Link>> urdflinklist;
//    std::vector<boost::shared_ptr<urdf::Joint>> urdfjointlist;
    std::vector<boost::shared_ptr<urdf::Material>> urdfmateriallist;

//    model.getLinks(urdflinklist);
//
//    for (std::vector<boost::shared_ptr<urdf::Link>>::iterator it =
//            urdflinklist.begin(); it != urdflinklist.end(); ++it) {
//        getGenericConfig(&nodeList, it);
//    }
//
//    model.getJoints(urdfjointlist);
//    for (std::vector<boost::shared_ptr<urdf::Link>>::iterator it =
//            urdfjointlist.begin(); it != urdfjointlist.end(); ++it) {
//        getGenericConfig(&jointList, it);
//    }

    model.getMaterials(urdfmateriallist);
    for (std::vector<boost::shared_ptr<urdf::Link>>::iterator it =
            urdfmateriallist.begin(); it != urdfmateriallist.end(); ++it) {
        getMaterialConfig(&materialList, it);
    }

//      xmlnodelist = root.elementsByTagName(QString("motor"));//motor
//      for (int i=0; i<xmlnodelist.size(); i++) {
//        getGenericConfig(&motorList, xmlnodelist.at(i).toElement());
//      }
//
//      xmlnodelist = root.elementsByTagName(QString("light"));//lights
//      for (int i=0; i<xmlnodelist.size(); i++) {
//        getGenericConfig(&lightList, xmlnodelist.at(i).toElement());
//      }
//
//      xmlnodelist = root.elementsByTagName(QString("sensor"));//sensor
//      for (int i=0; i<xmlnodelist.size(); i++) {
//        getGenericConfig(&sensorList, xmlnodelist.at(i).toElement());
//      }
//
//      xmlnodelist = root.elementsByTagName(QString("controller"));
//      for (int i=0; i<xmlnodelist.size(); i++) {
//        getGenericConfig(&controllerList, xmlnodelist.at(i).toElement());
//      }
//
//      xmlnodelist = root.elementsByTagName(QString("graphicOptions"));//graphicoptions
//      if (!xmlnodelist.isEmpty()) {
//        getGenericConfig(&graphicList, xmlnodelist.at(0).toElement());
//      }

return 1;
}

unsigned int Load::loadScene() {
for (unsigned int i = 0; i < materialList.size(); ++i)
    if (!loadMaterial(materialList[i]))
        return 0;
for (unsigned int i = 0; i < nodeList.size(); ++i)
    if (!loadNode(nodeList[i]))
        return 0;
for (unsigned int i = 0; i < jointList.size(); ++i)
    if (!loadJoint(jointList[i]))
        return 0;
//    for (unsigned int i = 0; i < motorList.size(); ++i)
//        if (!loadMotor(motorList[i]))
//            return 0;
//    for (unsigned int i = 0; i < sensorList.size(); ++i)
//        if (!loadSensor(sensorList[i]))
//            return 0;
//    for (unsigned int i = 0; i < controllerList.size(); ++i)
//        if (!loadController(controllerList[i]))
//            return 0;
//    for (unsigned int i = 0; i < graphicList.size(); ++i)
//        if (!loadGraphic(graphicList[i]))
//            return 0;
//    for (unsigned int i = 0; i < lightList.size(); ++i)
//        if (!loadLight(lightList[i]))
//            return 0;

return 1;
}

unsigned int Load::loadMaterial(utils::ConfigMap config) {
MaterialData material;
unsigned long id;

int valid = material.fromConfigMap(&config, tmpPath);
if ((id = config["id"][0].getULong())) {
    materials[id] = material;
}

return valid;
}

unsigned int Load::loadNode(utils::ConfigMap config) {
NodeData node;
config["mapIndex"].push_back(utils::ConfigItem(mapIndex));
int valid = node.fromConfigMap(&config, tmpPath,
        control->loadCenter->loadScene[mFileSuffix]);
if (!valid)
    return 0;

// handle material
utils::ConfigMap::iterator it;
if ((it = config.find("material_id")) != config.end()) {
    unsigned long id = it->second[0].getULong();
    if (id) {
        std::map<unsigned long, MaterialData>::iterator it = materials.find(id);
        if (it != materials.end())
            node.material = it->second;
    }
}

// the group ids could be also handled in the NodeData by the mapIndex
if (node.groupID)
    node.groupID += hack_ids * 10000;

NodeId oldId = node.index;
NodeId newId = control->nodes->addNode(&node);
if (!newId) {
    LOG_ERROR("addNode returned 0");
    return 0;
}
control->loadCenter->loadScene[mFileSuffix]->setMappedID(oldId, newId,
        MAP_TYPE_NODE, mapIndex);

if (mRobotName != "") {
    control->entities->addNode(mRobotName, node.index, node.name);
}
return 1;
}

unsigned int Load::loadJoint(utils::ConfigMap config) {
JointData joint;
config["mapIndex"].push_back(utils::ConfigItem(mapIndex));
int valid = joint.fromConfigMap(&config, tmpPath,
        control->loadCenter->loadScene[mFileSuffix]);
if (!valid) {
    fprintf(stderr, "Load: error while loading joint\n");
    return 0;
}

JointId oldId = joint.index;
JointId newId = control->joints->addJoint(&joint);
if (!newId) {
    LOG_ERROR("addJoint returned 0");
    return 0;
}
control->loadCenter->loadScene[mFileSuffix]->setMappedID(oldId, newId,
        MAP_TYPE_JOINT, mapIndex);

if (mRobotName != "") {
    control->entities->addJoint(mRobotName, joint.index, joint.name);
}
return true;
}

unsigned int Load::loadMotor(utils::ConfigMap config) {
MotorData motor;
config["mapIndex"].push_back(utils::ConfigItem(mapIndex));

int valid = motor.fromConfigMap(&config, tmpPath,
        control->loadCenter->loadScene[mFileSuffix]);
if (!valid) {
    fprintf(stderr, "Load: error while loading motor\n");
    return 0;
}

MotorId oldId = motor.index;
MotorId newId = control->motors->addMotor(&motor);
if (!newId) {
    LOG_ERROR("addMotor returned 0");
    return 0;
}
control->loadCenter->loadScene[mFileSuffix]->setMappedID(oldId, newId,
        MAP_TYPE_MOTOR, mapIndex);

if (mRobotName != "") {
    control->entities->addMotor(mRobotName, motor.index, motor.name);
}
return true;
}

BaseSensor* Load::loadSensor(utils::ConfigMap config) {
config["mapIndex"].push_back(utils::ConfigItem(mapIndex));
unsigned long sceneID = config["index"][0].getULong();
BaseSensor *sensor = control->sensors->createAndAddSensor(&config);
if (sensor != 0) {
    control->loadCenter->loadScene[mFileSuffix]->setMappedID(sceneID,
            sensor->getID(), MAP_TYPE_SENSOR, mapIndex);
}

return sensor;
}

unsigned int Load::loadController(utils::ConfigMap config) {
ControllerData controller;
config["mapIndex"].push_back(utils::ConfigItem(mapIndex));

int valid = controller.fromConfigMap(&config, tmpPath,
        control->loadCenter->loadScene[mFileSuffix]);
if (!valid) {
    fprintf(stderr, "Load: error while loading Controller\n");
    return 0;
}

MotorId oldId = controller.id;
MotorId newId = control->controllers->addController(controller);
if (!newId) {
    LOG_ERROR("Load: addController returned 0");
    return 0;
}
control->loadCenter->loadScene[mFileSuffix]->setMappedID(oldId, newId,
        MAP_TYPE_CONTROLLER, mapIndex);
if (mRobotName != "") {
    control->entities->addController(mRobotName, newId);
}
return 1;
}

unsigned int Load::loadGraphic(utils::ConfigMap config) {
GraphicData graphic;
config["mapIndex"].push_back(utils::ConfigItem(mapIndex));
int valid = graphic.fromConfigMap(&config, tmpPath,
        control->loadCenter->loadScene[mFileSuffix]);
if (!valid) {
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
        control->loadCenter->loadScene[mFileSuffix]);
if (!valid) {
    fprintf(stderr, "Load: error while loading light\n");
    return 0;
}

control->sim->addLight(light);
return true;
}

void Load::getLinkConfig(std::vector<utils::ConfigMap> *configList,
    const boost::shared_ptr<urdf::Link> link) {
utils::ConfigMap config;

configList->push_back(config);
}

void Load::getJointConfig(std::vector<utils::ConfigMap> *configList,
    const boost::shared_ptr<urdf::Joint> joint) {
utils::ConfigMap config;

configList->push_back(config);
}

void Load::getMaterialConfig(std::vector<utils::ConfigMap> *configList,
    const boost::shared_ptr<urdf::Material> material) {
utils::ConfigMap config;

configList->push_back(config);
}

//void Load::getGenericConfig(utils::ConfigMap *config, const QDomElement &elementNode) {
//
//    QDomNodeList xmlnodepartlist = elementNode.childNodes();
//    QDomNamedNodeMap attributes = elementNode.attributes();
//
//    std::string tagName, value;
//    QDomElement child;
//    QDomNode child2;
//
//    for (int i = 0; i < attributes.size(); i++) {
//        child2 = attributes.item(i);
//        tagName = child2.nodeName().toStdString();
//        value = child2.nodeValue().toStdString();
//        if (!tagName.empty()) {
//            (*config)[tagName].push_back(utils::ConfigItem(value));
//        }
//    }
//
//    for (int i = 0; i < xmlnodepartlist.size(); i++) {
//        child = xmlnodepartlist.at(i).toElement();
//
//        tagName = child.tagName().toStdString();
//        value = child.text().toStdString();
//        if (!tagName.empty()) {
//            (*config)[tagName].push_back(utils::ConfigItem(value));
//            getGenericConfig(&((*config)[tagName].back().children), child);
//        }
//    }
//}
} // end of namespace urdf_loader
} // end of namespace mars
