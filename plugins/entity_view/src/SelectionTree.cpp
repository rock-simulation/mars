/*
 *  Copyright 2015 DFKI GmbH Robotics Innovation Center
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


#include "SelectionTree.h"

#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/sim/JointManagerInterface.h>
#include <mars/interfaces/sim/MotorManagerInterface.h>
#include <mars/interfaces/sim/SensorManagerInterface.h>
#include <mars/interfaces/sim/ControllerManagerInterface.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>
#include <mars/utils/misc.h>
#include <mars/utils/mathUtils.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

using namespace std;

namespace mars {
  using namespace interfaces;
  using namespace utils;
  namespace plugins {

    SelectionTree::SelectionTree(interfaces::ControlCenter *c,
                                 config_map_gui::DataWidget *dw,
                                 QWidget *parent) :
      dw(dw), main_gui::BaseWidget(parent, c->cfg, "SelectionTree") {
      filled = false;
      control = c;
      editCategory = 0;
      this->setWindowTitle(tr("Node Selection"));

      selectAllowed = true;
      treeWidget = new QTreeWidget(this);
      treeWidget->setHeaderHidden(true);
      treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
      connect(treeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(selectNodes()));
      connect(dw, SIGNAL(valueChanged(std::string, std::string)), this, SLOT(valueChanged(std::string, std::string)));
      //connect(treeWidget, SIGNAL(itemSelectionChanged()), this, SIGNAL(itemSelectionChanged()));
      if(control->graphics) {
        control->graphics->addEventClient((interfaces::GraphicsEventClient*)this);
      }

      // todo: improve default handling for all entities
      MaterialData md;
      md.toConfigMap(&defaultMaterial, false, true);

      LightData ld;
      ld.toConfigMap(&defaultLight);

      createTree();

      QVBoxLayout *layout = new QVBoxLayout;
      QHBoxLayout *hlayout = new QHBoxLayout;
      layout->addWidget(treeWidget);
      QPushButton *button = new QPushButton("Delete Entities");
      connect(button, SIGNAL(clicked()), this, SLOT(deleteEntities()));
      hlayout->addWidget(button);
      button = new QPushButton("Update TreeView");
      connect(button, SIGNAL(clicked()), this, SLOT(update()));
      hlayout->addWidget(button);
      layout->addLayout(hlayout);
      setLayout(layout);
      filled = true;
    }


    SelectionTree::~SelectionTree() {
    }

    void SelectionTree::fill(unsigned long id, QTreeWidgetItem *current) {
      QStringList temp;

      for (unsigned int i = 0; i < simNodes.size(); i++) {
        if (simNodes[i].index == id) {
          bool found = false;
          for (unsigned int k = 0; k < present.size() && !found; k++) {
            if (present[k] == id) found = true;
          }
          if (found == false) {
            temp << QString::number(id) + ":" + QString::fromStdString(simNodes[i].name);
            QTreeWidgetItem *next = new QTreeWidgetItem(temp);
            // todo: should read the selected state in the simulation
            nodeItemMap[id] = next;
            current->addChild(next);
            current = next;
            present.push_back(id);
          }
          simNodes.erase(simNodes.begin()+i);
          break;
        }
      }

      vector<unsigned long> children = control->nodes->getConnectedNodes(id);
      vector<unsigned long> newNodes;

      for(size_t i=0; i<children.size(); i++)  {
        bool found = false;
        for(size_t k = 0; k<present.size() && !found; k++) {
          if(present[k] == children[i]) {
            found = true;
          }
        }
        if(!found) {
          newNodes.push_back(children[i]);
        }
      }

      for (unsigned int i = 0; i < newNodes.size(); i++) {
        fill(newNodes[i], current);
      }
    }

    void SelectionTree::createTree()  {
      std::vector<interfaces::core_objects_exchange> tmpList;
      present.clear();
      control->nodes->getListNodes(&tmpList);
      simNodes = tmpList;
      QStringList temp;
      temp << "nodes";
      QTreeWidgetItem *current = new QTreeWidgetItem(temp);
      treeWidget->addTopLevelItem(current);

      while(simNodes.size() > 0) fill(simNodes[0].index, current);
      tmpList.swap(simNodes);
      //treeWidget->expandAll();

      control->joints->getListJoints(&simJoints);
      addCoreExchange(simJoints, "joints");

      control->motors->getListMotors(&simMotors);
      addCoreExchange(simMotors, "motors");

      control->sensors->getListSensors(&simSensors);
      addCoreExchange(simSensors, "sensors");

      control->controllers->getListController(&simControllers);
      addCoreExchange(simControllers, "controllers");

      temp.clear();
      temp << "materials";
      current = new QTreeWidgetItem(temp);
      treeWidget->addTopLevelItem(current);
      std::vector<interfaces::MaterialData> mList;
      mList = control->graphics->getMaterialList();
      materialMap.clear();
      for(size_t i=0; i<mList.size(); ++i) {
        temp.clear();
        temp << QString::fromStdString(mList[i].name);
        configmaps::ConfigMap map;
        mList[i].toConfigMap(&map);
        materialMap[mList[i].name] = map;
        //map.toYamlStream(std::cerr);
        QTreeWidgetItem *next = new QTreeWidgetItem(temp);
        current->addChild(next);
      }

      if(control->graphics) {
        { // handle lights
          lightMap.clear();
          temp.clear();
          temp << "lights";
          current = new QTreeWidgetItem(temp);
          treeWidget->addTopLevelItem(current);
          std::vector<interfaces::LightData*> simLights;
          control->graphics->getLights(&simLights);
          for(size_t i=0; i<simLights.size(); ++i) {
            temp.clear();
            temp << QString::fromStdString(simLights[i]->name);
            configmaps::ConfigMap map;
            simLights[i]->toConfigMap(&map);
            lightMap[simLights[i]->name] = map;
            //map.toYamlStream(std::cerr);
            QTreeWidgetItem *next = new QTreeWidgetItem(temp);
            current->addChild(next);
          }
        }
        { // handle windows
          lightMap.clear();
          temp.clear();
          temp << "graphics";
          current = new QTreeWidgetItem(temp);
          treeWidget->addTopLevelItem(current);
          temp.clear();
          temp << "scene";
          QTreeWidgetItem *next = new QTreeWidgetItem(temp);
          current->addChild(next);
          std::vector<unsigned long> ids;
          control->graphics->getList3DWindowIDs(&ids);
          for(auto it: ids) {
            GraphicsWindowInterface *gw = control->graphics->get3DWindow(it);
            temp.clear();
            std::string gwName = gw->getName();
            if(gwName.empty()) {
              temp << QString::number(it) + ":window";
            }
            else {
              temp << QString::number(it) + gwName.c_str();
            }
            QTreeWidgetItem *next = new QTreeWidgetItem(temp);
            current->addChild(next);
          }
        }
      }
    }

    void SelectionTree::reset(void) {
      while (treeWidget->topLevelItemCount() > 0)
        (void)treeWidget->takeTopLevelItem(0);
      present.clear();
    }

    void SelectionTree::selectNodes(void) {
      if(selectAllowed) { // handle selection state
        int n;
        unsigned long id;
        QList<QTreeWidgetItem*> selectedItems = treeWidget->selectedItems();
        std::vector<unsigned long> ids;
        for(size_t i=0; i<selectedItems.size(); ++i) {
          n = selectedItems[i]->text(0).indexOf(":");
          QTreeWidgetItem *parent = selectedItems[i]->parent();
          if(!parent) continue;
          while(parent->parent()) parent = parent->parent();
          if(parent->text(0) == "nodes") {
            ids.push_back(selectedItems[i]->text(0).left(n).toULong());
          }
        }
        bool selected;
        /* todo: get drawid2 should be used, so far we assume every node
           uses two ids */
        unsigned long drawID;
        for(size_t i=0; i<simNodes.size(); ++i) {
          drawID = control->nodes->getDrawID(simNodes[i].index);
          if(!drawID) continue;
          selected = false;
          for(size_t k=0; k<ids.size(); ++k) {
            if(ids[k] == simNodes[i].index) {
              selected = true;
              ids.erase(ids.begin()+k);
              break;
            }
          }
          control->graphics->setDrawObjectSelected(drawID, selected);
          control->graphics->setDrawObjectSelected(drawID+1, selected);
        }
      }

      { // handle config map gui
        QTreeWidgetItem* currentItem = treeWidget->currentItem();
        if(currentItem) {
          QTreeWidgetItem *parent = currentItem->parent();
          if(!parent) return;
          while(parent->parent()) parent = parent->parent();
          int n = currentItem->text(0).indexOf(":");
          nodeData.index = motorData.index = jointData.index = 0;
          currentLight.clear();
          currentMaterial.clear();
          // todo: remove current information (motorData, currentLigth etc.)
          if(n>-1 && parent->text(0) != "graphics") {
            std::vector<std::string> editPattern;
            std::vector<std::string> filePattern;
            std::vector<std::string> colorPattern;
            std::vector<std::string> dropDownPattern;
            std::vector<std::vector<std::string> > dropDownValues;
            configmaps::ConfigMap map;
            std::string name;
            unsigned long id = currentItem->text(0).left(n).toULong();
            if(parent->text(0) == "nodes") {
              nodeData = control->nodes->getFullNode(id);
              name = nodeData.name;
              std::string preStr = "../"+name+"/";
              editPattern.push_back(preStr+"pose/*");
              editPattern.push_back(preStr+"extend/*");
              editPattern.push_back(preStr+"visual/material");
              editPattern.push_back(preStr+"visual/cullMask");
              editPattern.push_back(preStr+"visual/brightness");
              editPattern.push_back(preStr+"name");
              editPattern.push_back(preStr+"c*");
              editPattern.push_back(preStr+"mass");
              editPattern.push_back(preStr+"density");
              editPattern.push_back(preStr+"movable");
              editPattern.push_back(preStr+"groupid");
              //editPattern.push_back(preStr+"rotation*");
              nodeData.toConfigMap(&map, false, true);
              updateNodeMap(map);
              editCategory = 1;
            }
            else if(parent->text(0) == "joints") {
              jointData = control->joints->getFullJoint(id);
              jointData.toConfigMap(&map);
              name = jointData.name;
              std::string preStr = "../"+name+"/";
              //editPattern.push_back(preStr+"type");
              editPattern.push_back(preStr+"axis*");
              editPattern.push_back(preStr+"anchor*");
              editPattern.push_back(preStr+"lowStop*");
              editPattern.push_back(preStr+"highStop*");
              editPattern.push_back(preStr+"damping_const_*");
              editPattern.push_back(preStr+"spring_const_*");
              editPattern.push_back(preStr+"invertAxis");
              dropDownPattern.push_back(preStr+"anchorpos");
              dropDownValues.resize(1);
              dropDownValues[0].push_back("node1");
              dropDownValues[0].push_back("node2");
              dropDownValues[0].push_back("center");
              dropDownValues[0].push_back("custom");
              editCategory = 2;
            }
            else if(parent->text(0) == "motors") {
              motorData = control->motors->getFullMotor(id);
              motorData.toConfigMap(&map);
              name = motorData.name;
              // todo: remove index from pattern
              std::string preStr = "../"+name+"/";
              //editPattern.push_back(preStr+"name");
              editPattern.push_back(preStr+"p");
              editPattern.push_back(preStr+"i");
              editPattern.push_back(preStr+"d");
              editPattern.push_back(preStr+"type");
              editPattern.push_back(preStr+"maxSpeed");
              editPattern.push_back(preStr+"maxEffort");
              editPattern.push_back(preStr+"minValue");
              editPattern.push_back(preStr+"maxValue");
              //editPattern.push_back(preStr+"jointIndex");
              dropDownPattern.push_back("*/type");
              dropDownValues.resize(1);
              dropDownValues[0].push_back("PID");
              dropDownValues[0].push_back("DC");
              editCategory = 3;
            }
            else if(parent->text(0) == "sensors") {
              const BaseSensor *sensor = control->sensors->getFullSensor(id);
              map = sensor->createConfig();
              name = sensor->name;
              editCategory = 7;
            }
            dw->setEditPattern(editPattern);
            dw->setFilePattern(filePattern);
            dw->setColorPattern(colorPattern);
            dw->setDropDownPattern(dropDownPattern, dropDownValues);
            dw->setConfigMap(name, map);
          }
          else if(parent->text(0) == "controllers") {
            editCategory = 4;
          }
          else if(parent->text(0) == "materials") {
            std::vector<std::string> editPattern;
            std::vector<std::string> filePattern;
            std::vector<std::string> colorPattern;
            // fake pattern to disable everthing
            editPattern.push_back("*/ambientColor/*");
            editPattern.push_back("*/diffuseColor/*");
            editPattern.push_back("*/specularColor/*");
            editPattern.push_back("*/emissionColor/*");
            editPattern.push_back("*/diffuseTexture");
            editPattern.push_back("*/normalTexture");
            //editPattern.push_back("*/displacementTexture");
            editPattern.push_back("*/bumpNorFac");
            editPattern.push_back("*/transparency");
            editPattern.push_back("*/shininess");
            editPattern.push_back("*/tex_scale");
            editPattern.push_back("*/getLight");

            filePattern.push_back("*/diffuseTexture");
            filePattern.push_back("*/normalTexture");
            //filePattern.push_back("*/diaplacementTexture");

            colorPattern.push_back("*/ambientColor");
            colorPattern.push_back("*/diffuseColor");
            colorPattern.push_back("*/specularColor");
            colorPattern.push_back("*/emissionColor");

            currentMaterial = materialMap[currentItem->text(0).toStdString()];
            configmaps::ConfigMap map = defaultMaterial;
            map.append(currentMaterial);
            dw->setEditPattern(editPattern);
            dw->setFilePattern(filePattern);
            dw->setColorPattern(colorPattern);
            dw->setConfigMap(currentMaterial["name"], map);
            editCategory = 5;
          }
          else if(parent->text(0) == "lights") {
            std::vector<std::string> editPattern;
            std::vector<std::string> filePattern;
            std::vector<std::string> colorPattern;
            // fake pattern to disable everthing
            editPattern.push_back("*/ambient/*");
            editPattern.push_back("*/diffuse/*");
            editPattern.push_back("*/specular/*");
            editPattern.push_back("*/position/*");
            editPattern.push_back("*/lookat/*");
            editPattern.push_back("*/constantAttenuation");
            editPattern.push_back("*/linearAttenuation");
            editPattern.push_back("*/quadraticAttenuation");
            editPattern.push_back("*/type");
            editPattern.push_back("*/angle");
            editPattern.push_back("*/exponent");
            editPattern.push_back("*/directional");
            editPattern.push_back("*/nodeName");

            colorPattern.push_back("*/ambient");
            colorPattern.push_back("*/diffuse");
            colorPattern.push_back("*/specular");
            std::string lightName = currentItem->text(0).toStdString();
            currentLight = lightMap[lightName];
            configmaps::ConfigMap map = defaultLight;
            map.append(currentLight);
            dw->setEditPattern(editPattern);
            dw->setFilePattern(filePattern);
            dw->setColorPattern(colorPattern);
            dw->setConfigMap(lightName, map);
            editCategory = 6;
          }
          else if(parent->text(0) == "graphics") {
            std::string item = currentItem->text(0).toStdString();
            if(item == "scene") {
              std::vector<std::string> editPattern;
              std::vector<std::string> filePattern;
              std::vector<std::string> colorPattern;
              editPattern.push_back("*/");
              colorPattern.push_back("*/fogColor");
              GraphicData gd = control->graphics->getGraphicOptions();
              configmaps::ConfigMap map;
              gd.toConfigMap(&map);
              map.erase("clearColor");
              dw->setEditPattern(editPattern);
              dw->setFilePattern(filePattern);
              dw->setColorPattern(colorPattern);
              dw->setConfigMap("general", map);
              editCategory = 8;
            }
            else {
              // const GraphicData gd = control->graphics->getGraphicOptions();
              // configmaps::ConfigMap map = gd.toConfigMap();
              std::vector<std::string> editPattern;
              std::vector<std::string> filePattern;
              std::vector<std::string> colorPattern;
              std::vector<std::string> dropDownPattern;
              std::vector<std::vector<std::string> > dropDownValues;
              editPattern.push_back("*/");
              colorPattern.push_back("*/clearColor");
              dropDownPattern.push_back("*/projection");
              dropDownPattern.push_back("*/mouse");
              dropDownValues.resize(2);
              dropDownValues[0].push_back("perspective");
              dropDownValues[0].push_back("orthogonal");
              dropDownValues[1].push_back("default");
              dropDownValues[1].push_back("invert");
              dropDownValues[1].push_back("osg");
              dropDownValues[1].push_back("iso");

              dw->setEditPattern(editPattern);
              dw->setFilePattern(filePattern);
              dw->setColorPattern(colorPattern);
              dw->setDropDownPattern(dropDownPattern, dropDownValues);

              std::vector<std::string> arrString = explodeString(':', item);
              currentWindowID = atoi(arrString[0].c_str());
              GraphicsWindowInterface *gw = control->graphics->get3DWindow(currentWindowID);
              GraphicsCameraInterface *gc = gw->getCameraInterface();
              ConfigMap map;
              int mouse = gc->getCamera();

              if(gc->getCameraType() == 1) map["projection"] = "perspective";
              else map["projection"] = "orthogonal";

              if(mouse == ODE_CAM) map["mouse"] = "default";
              else if(mouse == MICHA_CAM) map["mouse"] = "invert";
              else if(mouse == OSG_CAM) map["mouse"] = "osg";
              else if(mouse == ISO_CAM) map["mouse"] = "iso";

              Color c = gw->getClearColor();
              c.toConfigItem(map["clearColor"]);
              double v[7];
              gc->getViewportQuat(v, v+1, v+2, v+3, v+4, v+5, v+6);
              Quaternion q;
              q.x() = v[3];
              q.y() = v[4];
              q.z() = v[5];
              q.w() = v[6];
              sRotation r = quaternionTosRotation(q);
              map["pose"]["position"]["x"] = v[0];
              map["pose"]["position"]["y"] = v[1];
              map["pose"]["position"]["z"] = v[2];
              map["pose"]["euler"]["alpha"] = r.alpha;
              map["pose"]["euler"]["beta"] = r.beta;
              map["pose"]["euler"]["gamma"] = r.gamma;
              dw->setConfigMap(arrString[1], map);
              editCategory = 9;
            }
          }
        }
      }
    }

    void SelectionTree::updateNodeMap(ConfigMap &map) {
      Vector p(map["position"]["x"], map["position"]["y"],
               map["position"]["z"]);
      Quaternion q;
      q.x() = map["rotation"]["x"];
      q.y() = map["rotation"]["y"];
      q.z() = map["rotation"]["z"];
      q.w() = map["rotation"]["w"];
      sRotation r = quaternionTosRotation(q);
      map["rotation"]["alpha"] = r.alpha;
      map["rotation"]["beta"] = r.beta;
      map["rotation"]["gamma"] = r.gamma;
      if(!map.hasKey("cfdir1")) {
        map["contact"]["cfdir1"]["x"] = 0.;
        map["contact"]["cfdir1"]["y"] = 0.;
        map["contact"]["cfdir1"]["z"] = 0.;
      }
      std::vector<std::string> move {"cmax_num_contacts", "cerp", "ccfm", "cfriction1", "cfriction2", "cmotion1", "cmotion2", "cfds1", "cfds2", "cbounce", "cbounce_vel", "capprox", "coll_bitmask", "cfdir1"};
      for(auto it: move) {
        if(map.hasKey(it)) {
          map["contact"][it] = map[it];
          map.erase(it);
        }
      }
      move = {"material", "cullMask", "brightness", "pivot", "visualsize", "visualscale", "visualposition", "visualrotation", "filename", "origname", "shadow_id", "shadowcaster", "shadowreceiver"};
      for(auto it: move) {
        if(map.hasKey(it)) {
          map["visual"][it] = map[it];
          map.erase(it);
        }
      }
      if((ulong)map["relativeid"] != 0) {
        NodeData parent = control->nodes->getFullNode(map["relativeid"]);
        // inverse calculation to convert world to relative
        p = parent.rot.inverse() * (p-parent.pos);
        q = parent.rot.inverse() * q;
      }
      map["pose"]["local"]["relativeid"] = map["relativeid"];
      map.erase("relativeid");
      map["pose"]["world"]["position"] = map["position"];
      map.erase("position");
      map["pose"]["world"]["rotation"] = map["rotation"];
      map.erase("rotation");
      r = quaternionTosRotation(q);
      map["pose"]["local"]["rotation"]["x"] = q.x();
      map["pose"]["local"]["rotation"]["y"] = q.y();
      map["pose"]["local"]["rotation"]["z"] = q.z();
      map["pose"]["local"]["rotation"]["w"] = q.w();
      map["pose"]["local"]["rotation"]["alpha"] = r.alpha;
      map["pose"]["local"]["rotation"]["beta"] = r.beta;
      map["pose"]["local"]["rotation"]["gamma"] = r.gamma;
      map["pose"]["local"]["position"]["x"] = p.x();
      map["pose"]["local"]["position"]["y"] = p.y();
      map["pose"]["local"]["position"]["z"] = p.z();
    }

    void SelectionTree::deleteEntities(void) {
      int n;
      unsigned long id;
      QList<QTreeWidgetItem*> selectedItems = treeWidget->selectedItems();
      std::vector<unsigned long> ids;
      for(size_t i=0; i<selectedItems.size(); ++i) {
        n = selectedItems[i]->text(0).indexOf(":");
        QTreeWidgetItem *parent = selectedItems[i]->parent();
        if(!parent) continue;
        while(parent->parent()) parent = parent->parent();
        if(parent->text(0) == "nodes") {
          id = selectedItems[i]->text(0).left(n).toULong();
          for(int k=0; k<selectedItems[i]->childCount(); ++k) {
            selectedItems[i]->parent()->addChild(selectedItems[i]->child(k)->clone());
          }
          selectedItems[i]->parent()->removeChild(selectedItems[i]);
          control->nodes->removeNode(id);
          if(nodeData.index == id) dw->clearGUI();

        }
        // todo: delete joints / motors / etc.
        else if(parent->text(0) == "joints") {
          id = selectedItems[i]->text(0).left(n).toULong();
          selectedItems[i]->parent()->removeChild(selectedItems[i]);
          control->joints->removeJoint(id);
          if(jointData.index == id) dw->clearGUI();
        }
        else if(parent->text(0) == "motors") {
          id = selectedItems[i]->text(0).left(n).toULong();
          selectedItems[i]->parent()->removeChild(selectedItems[i]);
          control->motors->removeMotor(id);
          if(motorData.index == id) dw->clearGUI();
        }
        else if(parent->text(0) == "sensors") {
        }
        else if(parent->text(0) == "controllers") {
        }
        else if(parent->text(0) == "materials") {
        }
        else if(parent->text(0) == "lights") {
          if(control->graphics) {
            std::string name = selectedItems[i]->text(0).toStdString();
            std::vector<interfaces::LightData*> simLights;
            control->graphics->getLights(&simLights);
            for(size_t i=0; i<simLights.size(); ++i) {
              if(simLights[i]->name == name) {
                control->graphics->removeLight(i);
                break;
              }
            }
          }
          selectedItems[i]->parent()->removeChild(selectedItems[i]);
        }
      }
    }

    void SelectionTree::update(void) {
      reset();
      dw->clearGUI();
      createTree();
      // todo: more intelligent update
    }

    void SelectionTree::closeEvent(QCloseEvent* event) {
      (void)event;
      emit closeSignal(this);
    }

    void SelectionTree::valueChanged(std::string name, std::string value) {
      if(name.empty()) return;
      // remove name prefix, otherwise the name could influence the pattern matching
      // done in the edit mehtods
      size_t p = name.find('/', 3);
      if(p != std::string::npos) {
        name = name.substr(p);
      }
      //fprintf(stderr, "get feedback: %s\n", name.c_str());
      if(editCategory == 1) {
        // convert local / global pose
        if(matchPattern("*pose/", name) && !matchPattern("*relativeid", name)) {
          if(matchPattern("*local/", name)) {
            p = name.find("local/");
            if(p!=std::string::npos) {
              name = name.substr(p+5);
            }
            // calculate global pose
            nodeData = control->nodes->getFullNode(nodeData.index);
            ConfigMap map;
            nodeData.toConfigMap(&map, false, true);
            updateNodeMap(map);
            ConfigMap &posMap = map["pose"]["local"]["position"];
            ConfigMap &qMap = map["pose"]["local"]["rotation"];
            Vector pos(posMap["x"], posMap["y"], posMap["z"]);
            Quaternion q;
            q.x() = qMap["x"];
            q.y() = qMap["y"];
            q.z() = qMap["z"];
            q.w() = qMap["w"];
            sRotation r = quaternionTosRotation(q);
            double v = atof(value.c_str());
            bool euler = false;
            bool editPos = true;
            if(matchPattern("*position/x", name)) pos.x() = v;
            if(matchPattern("*position/y", name)) pos.y() = v;
            if(matchPattern("*position/z", name)) pos.z() = v;
            if(matchPattern("*rotation/x", name)) q.x() = v, editPos=false;
            if(matchPattern("*rotation/y", name)) q.y() = v, editPos=false;
            if(matchPattern("*rotation/z", name)) q.z() = v, editPos=false;
            if(matchPattern("*rotation/w", name)) q.w() = v, editPos=false;
            if(matchPattern("*rotation/alpha", name)) r.alpha = v, euler=true, editPos=false;
            if(matchPattern("*rotation/beta", name)) r.beta = v, euler=true, editPos=false;
            if(matchPattern("*rotation/gamma", name)) r.gamma = v, euler=true, editPos=false;
            if(euler) {
              q = eulerToQuaternion(r);
            }
            /* this transformation is already done in editNode
            if((ulong)map["pose"]["local"]["relativeid"] != 0) {
              NodeData parent = control->nodes->getFullNode(map["relativeid"]);
              pos = parent.rot.inverse() * (pos-parent.pos);
              q = parent.rot.inverse() * q;
            }
            */
            nodeData.pos = pos;
            nodeData.rot = q;
            if(editPos) {
              control->nodes->editNode(&nodeData, EDIT_NODE_POS);
            }
            else {
              control->nodes->editNode(&nodeData, EDIT_NODE_ROT);
            }
          }
          else {
            p = name.find("world/");
            if(p!=std::string::npos) {
              name = name.substr(p+5);
            }
            control->nodes->edit(nodeData.index, name, value);
          }
        }
        else {
          control->nodes->edit(nodeData.index, name, value);
        }
        // todo: update map
        nodeData = control->nodes->getFullNode(nodeData.index);
        ConfigMap map;
        nodeData.toConfigMap(&map, false, true);
        updateNodeMap(map);
        dw->updateConfigMap(nodeData.name, map);
      }
      else if(editCategory == 2) {
        control->joints->edit(jointData.index, name, value);
      }
      else if(editCategory == 3) {
        control->motors->edit(motorData.index, name, value);
      }
      else if(editCategory == 5) {
        control->graphics->editMaterial(currentMaterial["name"], name, value);
        // update material internal
        std::vector<interfaces::MaterialData> mList;
        mList = control->graphics->getMaterialList();
        for(size_t i=0; i<mList.size(); ++i) {
          if(mList[i].name == (std::string)currentMaterial["name"]) {
            configmaps::ConfigMap map;
            mList[i].toConfigMap(&map);
            materialMap[mList[i].name] = map;
            break;
          }
        }
      }
      else if(editCategory == 6) {
        control->graphics->editLight(currentLight["index"], name, value);
        std::vector<interfaces::LightData*> simLights;
        control->graphics->getLights(&simLights);
        for(size_t i=0; i<simLights.size(); ++i) {
          if(simLights[i]->index == (unsigned long)currentLight["index"]) {
            configmaps::ConfigMap map;
            simLights[i]->toConfigMap(&map);
            lightMap[simLights[i]->name] = map;
            break;
          }
        }
      }
      else if(editCategory == 8) {
        control->graphics->edit(name, value);
      }
      else if(editCategory == 9) {
        control->graphics->edit(currentWindowID, name, value);
      }
    }

    void SelectionTree::selectEvent(unsigned long int id, bool mode) {
      selectAllowed = false;
      treeWidget->setSelectionMode(QAbstractItemView::MultiSelection);
      if(nodeItemMap.find(id) != nodeItemMap.end()) {
        if(mode) {
          QTreeWidgetItem* parent = nodeItemMap[id]->parent();
          while(parent) {
            treeWidget->expandItem(parent);
            parent = parent->parent();
          }
          treeWidget->setCurrentItem(nodeItemMap[id]);
        }
        nodeItemMap[id]->setSelected(mode);
      }
      selectAllowed = true;
      treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    }

    void SelectionTree::addCoreExchange(const std::vector<interfaces::core_objects_exchange> &objects, std::string category) {
      std::vector<interfaces::core_objects_exchange>::const_iterator it;
      std::vector<std::string> path;
      std::vector<std::string>::iterator pt;
      std::map<std::string, QTreeWidgetItem*> treeMap;
      QTreeWidgetItem *current, *top, *next;
      QStringList temp;
      temp << category.c_str();
      top = new QTreeWidgetItem(temp);
      treeWidget->addTopLevelItem(top);
      for(it=objects.begin(); it!=objects.end(); ++it) {
        path = explodeString('/', it->name);
        current = top;
        for(pt=path.begin(); pt!=path.end(); ++pt) {
          temp.clear();
          if(pt==path.end()-1) {
            temp << QString::number(it->index) + ":" + QString::fromStdString(*pt);
            current->addChild(new QTreeWidgetItem(temp));
          }
          else {
            if(treeMap.find(*pt) == treeMap.end()) {
              temp << QString::fromStdString(*pt);
              next = new QTreeWidgetItem(temp);
              current->addChild(next);
              treeMap[*pt] = next;
            }
            current = treeMap[*pt];
          }
        }
      }
    }

  } // end of namespace plugins
} // end of namespace mars
