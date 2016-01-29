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

      MaterialData md;
      md.toConfigMap(&defaultMaterial, false, true);

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
        map.toYamlStream(std::cerr);
        QTreeWidgetItem *next = new QTreeWidgetItem(temp);
        current->addChild(next);
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
          if(n>-1) {
            unsigned long id = currentItem->text(0).left(n).toULong();
            if(parent->text(0) == "nodes") {
              std::vector<std::string> editPattern;
              editPattern.push_back("*/position/*");
              editPattern.push_back("*/extend/*");
              editPattern.push_back("*/material");
              editPattern.push_back("*/c*");
              nodeData = control->nodes->getFullNode(id);
              configmaps::ConfigMap map = nodeData.map;
              if(!map.hasKey("cfdir1")) {
                map["cfdir1"]["x"] = 0.;
                map["cfdir1"]["y"] = 0.;
                map["cfdir1"]["z"] = 0.;
              }
              nodeData.toConfigMap(&map, false, true);
              dw->setConfigMap(nodeData.name, map, editPattern);
              editCategory = 1;
            }
            else if(parent->text(0) == "joints") {
              std::vector<std::string> editPattern;
              // fake pattern to disable everthing
              editPattern.push_back("//");
              jointData = control->joints->getFullJoint(id);
              configmaps::ConfigMap map;
              jointData.toConfigMap(&map);
              dw->setConfigMap(jointData.name, map, editPattern);
              editCategory = 2;
            }
            else if(parent->text(0) == "motors") {
              std::vector<std::string> editPattern;
              // fake pattern to disable everthing
              editPattern.push_back("//");
              motorData = control->motors->getFullMotor(id);
              configmaps::ConfigMap map;
              motorData.toConfigMap(&map);
              dw->setConfigMap(motorData.name, map, editPattern);
              editCategory = 3;
            }
          }
          else if(parent->text(0) == "controllers") {
            editCategory = 4;
          }
          else if(parent->text(0) == "materials") {
            std::vector<std::string> editPattern;
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

            currentMaterial = materialMap[currentItem->text(0).toStdString()];
            configmaps::ConfigMap map = defaultMaterial;
            map.append(currentMaterial);
            dw->setConfigMap(currentMaterial["name"], map,
                             editPattern);
            editCategory = 5;
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
      }
      // todo: delete joints / motors / etc.
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

      }
      else if(editCategory == 5) {
        control->graphics->editMaterial(currentMaterial["name"], name, value);
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
