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


#include "DialogDistance.h"
#include <iostream>

#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/sim/JointManagerInterface.h>

#include <QGridLayout>

using namespace std;

namespace mars {
  namespace gui {

    DialogDistance::DialogDistance(interfaces::ControlCenter* c)
      : main_gui::BaseWidget(0, c->cfg, "DialogDistance"),
        pDialog(new main_gui::PropertyDialog(NULL)) {
      filled = false;
      control = c;
      selection = distance = ap1 = ap2 = rp1 = rp2 = NULL;
 
      this->setWindowTitle(tr("Object Distance Info"));
 
      pDialog->setPropCallback(this);
      pDialog->hideAllButtons();


      viewNodes = pDialog->addGenericProperty("../Nodes", QVariant::Bool, true);
      viewJoints = pDialog->addGenericProperty("../Joints", QVariant::Bool, true);
  
      objectList = new QListWidget(NULL);
      objectList->setSelectionMode(QAbstractItemView::MultiSelection);
      connect(objectList, SIGNAL(itemSelectionChanged()), this, SLOT(selectObjects()));

      control->nodes->getListNodes(&simNodes);

      for (unsigned int i =0 ; i<simNodes.size(); i++ )
        objectList->addItem("Node " + QString::number(simNodes[i].index) + 
                            ":" + QString::fromStdString(simNodes[i].name));

      control->joints->getListJoints(&simJoints);

      for (unsigned int i =0 ; i<simJoints.size(); i++ )
        objectList->addItem("Joint " + QString::number(simJoints[i].index) + 
                            ":" + QString::fromStdString(simJoints[i].name));

      QVBoxLayout *vLayout = new QVBoxLayout;
      vLayout->setAlignment(Qt::AlignTop);
      vLayout->addWidget(pDialog);

      QGridLayout *layout = new QGridLayout;
      layout->addWidget(objectList, 0, 0);
      layout->addLayout(vLayout, 0, 1);
      layout->setColumnStretch(1, 1);

      this->setLayout(layout);
      filled = true;
    }

    DialogDistance::~DialogDistance() {
      delete pDialog;
    }

    void DialogDistance::valueChanged(QtProperty* property,
                                      const QVariant &value) {
      (void)value;
  
      if (filled == false) return;

      if (property == viewNodes) {
        if (value.toBool() == false) {
          QList<QListWidgetItem*> nodeItems = objectList->findItems("Node ", Qt::MatchStartsWith);
          for (unsigned int i = 0; i < (unsigned int)nodeItems.size(); i++) 
            (void)objectList->takeItem(objectList->row(nodeItems[i]));
          selectedNodes.clear();
          selectObjects();
        } else {
          control->nodes->getListNodes(&simNodes);
          QStringList nodeItems;
          for (unsigned int i = 0; i < simNodes.size(); i++ )
            nodeItems << "Node " + QString::number(simNodes[i].index) 
              + ":" + QString::fromStdString(simNodes[i].name);
          objectList->insertItems(0, nodeItems);
        }
      } else if (property == viewJoints) {
        if (value.toBool() == false) {
          QList<QListWidgetItem*> jointItems = objectList->findItems("Joint ", Qt::MatchStartsWith);
          for (unsigned int i = 0; i < (unsigned int)jointItems.size(); i++)
            (void)objectList->takeItem(objectList->row(jointItems[i]));
          selectedJoints.clear();
          selectObjects();
        } else {
          control->joints->getListJoints(&simJoints);
          QStringList jointItems;
          for (unsigned int i = 0; i < simJoints.size(); i++ )
            jointItems << "Joint " + QString::number(simJoints[i].index) 
              + ":" + QString::fromStdString(simJoints[i].name);
          objectList->addItems(jointItems);
        }
      }

      if (property == distance) 
        changeDistance(value.toDouble());

    }


    void DialogDistance::selectObjects() {
  
      static bool select_allowed = true;

      if (select_allowed == false)
        return;

      QList<QListWidgetItem*> selectedItems = objectList->selectedItems();

      if (selectedItems.size() < 2) 
        return;
  
      select_allowed = false;

      while (selectedItems.size() > 2) {
        selectedItems.front()->setSelected(false);
        selectedItems.pop_front();
      }

      selectedNodes.clear();
      selectedJoints.clear();

      for (unsigned int i = 0 ; i < (unsigned int)selectedItems.size(); i++) {
        int m = selectedItems[i]->text().indexOf(" ");
        int n = selectedItems[i]->text().indexOf(":");
        unsigned long object_id = selectedItems[i]->text().mid(m, n-m).toULong();
        if (selectedItems[i]->text().left(m) == "Node") 
          selectedNodes.push_back(object_id);
        else 
          selectedJoints.push_back(object_id);
      }

      updateProperties();
 
      select_allowed = true;
    }


    void DialogDistance::closeEvent(QCloseEvent *event) {
      (void)event;
      emit closeSignal(this);
    }


    void DialogDistance::updateProperties(void) {
  
      if (objectList->selectedItems().size() != 2) {
        if (selection != NULL) {
          pDialog->removeGenericProperty(selection);
          pDialog->removeGenericProperty(distance);
          pDialog->removeGenericProperty(ap1);
          pDialog->removeGenericProperty(ap2);
          pDialog->removeGenericProperty(rp1);
          pDialog->removeGenericProperty(rp2);
        }
        return;
      }
  
      filled = false;
      QString label = objectList->selectedItems()[0]->text() + " - " +
        objectList->selectedItems()[1]->text();
  
      switch (selectedNodes.size()) {
      case 2:
        control->nodes->getNodeExchange(selectedNodes[0], &first);
        control->nodes->getNodeExchange(selectedNodes[1], &second);
        break;
      case 1:
        control->nodes->getNodeExchange(selectedNodes[0], &first);
        control->joints->getJointExchange(selectedJoints[0], &second);
        break;
      case 0:
        control->joints->getJointExchange(selectedJoints[0], &first);
        control->joints->getJointExchange(selectedJoints[1], &second);
        break;
      }

      utils::Vector p1 = first.pos;
      utils::Vector p2 = second.pos;
      //Vector d1 = (~second.rot).VRotate(first.pos-second.pos);
      //Vector d2 = (~first.rot).VRotate(second.pos-first.pos);
      utils::Vector d1 = (second.rot).inverse() * (first.pos-second.pos);
      utils::Vector d2 = (first.rot).inverse()  * (second.pos-first.pos);
      //original_distance = first.pos.distanceTo(second.pos);
      original_distance = (first.pos - second.pos).norm();

      direction = p2;
      origin = p1;

      QString abs1 = "Vector(" + QString::number(p1.x()) + ", " + QString::number(p1.y()) + 
        ", " + QString::number(p1.z()) + ")";
      QString abs2 = "Vector(" + QString::number(p2.x()) + ", " + QString::number(p2.y()) + 
        ", " + QString::number(p2.z()) + ")";
      QString rel1 = "Vector(" + QString::number(d1.x()) + ", " + QString::number(d1.y()) + 
        ", " + QString::number(d1.z()) + ")";
      QString rel2 = "Vector(" + QString::number(d2.x()) + ", " + QString::number(d2.y()) + 
        ", " + QString::number(d2.z()) + ")";

      if (selection == NULL) {
        map<QString, QVariant> attr;
        attr["decimals"] = 9; 
        attr["singleStep"] = 0.1;
        attr["minimum"] = 0.0;
        selection =  pDialog->addGenericProperty("../Current selection", QVariant::String, label);
        distance =  pDialog->addGenericProperty("../Distance", QVariant::Double, original_distance, &attr);
        ap1 = pDialog->addGenericProperty("../Absolute position of " + first.name,  QVariant::String, abs1);
        ap2 = pDialog->addGenericProperty("../Absolute position of " + second.name, QVariant::String, abs2);
        rp1 = pDialog->addGenericProperty("../" + first.name + " relative to " + second.name,  QVariant::String, rel1);
        rp2 = pDialog->addGenericProperty("../" + second.name + " relative to " + first.name, QVariant::String, rel2);
      } else {
        selection->setValue(label);
        distance->setValue(original_distance);
        ap1->setValue(abs1); 
        ap1->setPropertyName("Absolute position of " + QString::fromStdString(first.name));
        ap2->setValue(abs2); 
        ap2->setPropertyName("Absolute position of " + QString::fromStdString(second.name));
        rp1->setValue(rel1); 
        rp1->setPropertyName(QString::fromStdString(first.name) + " relative to " + QString::fromStdString(second.name));
        rp2->setValue(rel2);
        rp2->setPropertyName(QString::fromStdString(second.name) + " relative to " + QString::fromStdString(first.name));
 
        pDialog->addGenericProperty(NULL, selection);
        pDialog->addGenericProperty(NULL, distance);
        pDialog->addGenericProperty(NULL, ap1);
        pDialog->addGenericProperty(NULL, ap2);
        pDialog->addGenericProperty(NULL, rp1);
        pDialog->addGenericProperty(NULL, rp2);  
      }
      filled = true;
    }



    void DialogDistance::changeDistance(double new_distance)
    {
      //Vector delta = (direction-origin).getNormalized()*(new_distance-original_distance);
      utils::Vector delta = (direction-origin).normalized()*(new_distance-original_distance);

      switch (selectedNodes.size()) {
      case 0:
      case 1:
        control->joints->setReloadAnchor(second.index, second.pos + delta);
        break;
    
      case 2: // node - node
        {
          interfaces::NodeData moveNode = control->nodes->getFullNode(second.index);
          moveNode.pos += delta;
          control->nodes->editNode(&moveNode, interfaces::EDIT_NODE_POS);
        }      
      }
  
      updateProperties();
    }

  } // end of namespace gui
} // end of namespace mars
