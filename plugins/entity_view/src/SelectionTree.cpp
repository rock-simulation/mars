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
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

using namespace std;

namespace mars {
  using namespace interfaces;
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
          if(n>-1) {
            std::vector<std::string> editPattern;
            std::vector<std::string> filePattern;
            std::vector<std::string> colorPattern;
            configmaps::ConfigMap map;
            std::string name;
            unsigned long id = currentItem->text(0).left(n).toULong();
            if(parent->text(0) == "nodes") {
              editPattern.push_back("*/position/*");
              editPattern.push_back("*/extend/*");
              editPattern.push_back("*/material");
              editPattern.push_back("*/cullMask");
              editPattern.push_back("*/brightness");
              editPattern.push_back("*/name");
              editPattern.push_back("*/c*");
              editPattern.push_back("*/mass");
              editPattern.push_back("*/density");
              editPattern.push_back("*/movable");
              nodeData = control->nodes->getFullNode(id);
              name = nodeData.name;
              map = nodeData.map;
              if(!map.hasKey("cfdir1")) {
                map["cfdir1"]["x"] = 0.;
                map["cfdir1"]["y"] = 0.;
                map["cfdir1"]["z"] = 0.;
              }
              nodeData.toConfigMap(&map, false, true);
              editCategory = 1;
            }
            else if(parent->text(0) == "joints") {
              // fake pattern to disable everthing
              editPattern.push_back("*/type");
              editPattern.push_back("*/axis*");
              editPattern.push_back("*/anchor*");
              editPattern.push_back("*/lowStop*");
              editPattern.push_back("*/highStop*");
              editPattern.push_back("*/damping_const_*");
              editPattern.push_back("*/spring_const_*");
              editPattern.push_back("*/invertAxis");
              jointData = control->joints->getFullJoint(id);
              jointData.toConfigMap(&map);
              name = jointData.name;
              editCategory = 2;
            }
            else if(parent->text(0) == "motors") {
              // todo: remove index from pattern
              editPattern.push_back("*/p");
              editPattern.push_back("*/i");
              editPattern.push_back("*/d");
              editPattern.push_back("*/type");
              editPattern.push_back("*/maxSpeed");
              editPattern.push_back("*/maxEffort");
              editPattern.push_back("*/minValue");
              editPattern.push_back("*/maxValue");
              motorData = control->motors->getFullMotor(id);
              motorData.toConfigMap(&map);
              name = motorData.name;
              editCategory = 3;
            }
            dw->setEditPattern(editPattern);
            dw->setFilePattern(filePattern);
            dw->setColorPattern(colorPattern);
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
            currentLight = lightMap[currentItem->text(0).toStdString()];
            configmaps::ConfigMap map = defaultLight;
            map.append(currentLight);
            dw->setEditPattern(editPattern);
            dw->setFilePattern(filePattern);
            dw->setColorPattern(colorPattern);
            dw->setConfigMap(currentLight["name"], map);
            editCategory = 6;
          }
        }
      }
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
      //fprintf(stderr, "get feedback: %s\n", name.c_str());
      if(editCategory == 1) {
        control->nodes->edit(nodeData.index, name, value);
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
        path = utils::explodeString('/', it->name);
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
