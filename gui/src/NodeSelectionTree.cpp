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


#include "NodeSelectionTree.h"

#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>

#include <QGridLayout>

using namespace std;

namespace mars {
  namespace gui {

    NodeSelectionTree::NodeSelectionTree(interfaces::ControlCenter *c,
                                         QWidget *parent) : 
      main_gui::BaseWidget(parent, c->cfg, "NodeSelectionTree"),
      pDialog(new main_gui::PropertyDialog(this)) {
      filled = false;
      control = c;

      control->graphics->addEventClient((interfaces::GraphicsEventClient*)this);
      control->nodes->getListNodes(&simNodes);

      this->setWindowTitle(tr("Node Selection"));
      pDialog->setPropCallback(this);
      pDialog->hideAllButtons();

      QStringList enumNames;
      enumNames << "Tree Mode" << "List Mode";
      node_view_mode = 
        pDialog->addGenericProperty("../Node View", QtVariantPropertyManager::enumTypeId(), 
                                    0, NULL, &enumNames);
      enumNames.clear();
      enumNames << "Single" << "Recursive";
      node_selection_mode = 
        pDialog->addGenericProperty("../Selection mode", QtVariantPropertyManager::enumTypeId(),
                                    0, NULL, &enumNames);
      enumNames.clear();
      for (unsigned int i = 0; i < simNodes.size(); i++)
        enumNames << QString::number(simNodes[i].index) + ":" + QString::fromStdString(simNodes[i].name);
      root = 
        pDialog->addGenericProperty("../Root node", QtVariantPropertyManager::enumTypeId(),
                                    0, NULL, &enumNames);
  
      select_allowed = true;

      treeWidget = new QTreeWidget(this);
      treeWidget->setHeaderHidden(true);
      treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
      connect(treeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(selectNodes()));
      connect(treeWidget, SIGNAL(itemSelectionChanged()), this, SIGNAL(itemSelectionChanged()));

      if (simNodes.size())
        createTree(simNodes[0].index);

      QGridLayout *layout = new QGridLayout;
      layout->addWidget(treeWidget, 0, 0);
      layout->addWidget(pDialog, 1, 0);
      layout->setRowStretch(0, 1);
      setLayout(layout);
      filled = true;
    }



    NodeSelectionTree::~NodeSelectionTree() {
      delete pDialog;

      control->graphics->removeEventClient(this);
      for (unsigned int i = 0; i < simNodes.size(); i++) {
        int draw_id = control->nodes->getDrawID(simNodes[i].index);
        control->graphics->setDrawObjectSelected(draw_id, false); 
      }
    }

    void NodeSelectionTree::valueChanged(QtProperty *property,
                                         const QVariant &value) {
      if (filled == false) return;

      if (property == node_view_mode) {
        if (value.toInt() == 0) {
          node_selection_mode->setEnabled(true);
          root->setEnabled(true);
          reset();
          QString str_value = root->attributeValue("enumNames").toStringList().at(value.toInt());
          int n = str_value.indexOf(":");
          createTree(str_value.left(n).toULong());
        } else {
          node_selection_mode->setEnabled(false);
          root->setEnabled(false);
          reset();
          createList();
        }
      }

      else if (property == root) {
        reset();
        QString str_value = root->attributeValue("enumNames").toStringList().at(value.toInt());
        int n = str_value.indexOf(":");
        createTree(str_value.left(n).toULong());  
      }
    }

    void NodeSelectionTree::fill(unsigned long id, QTreeWidgetItem *current) {
      QStringList temp;

      if (current == NULL) // this is a top level node
        for (unsigned int i = 0; i < simNodes.size(); i++)
          if (simNodes[i].index == id) {
            bool found = false;
            for (unsigned int k = 0; k < present.size() && !found; k++)
              if (present[k] == id)
                found = true;
            if (found == false) {
              temp << QString::number(id) + ":" + QString::fromStdString(simNodes[i].name);
              current = new QTreeWidgetItem(temp);
              treeWidget->addTopLevelItem(current);
              present.push_back(id);
            }
          }
  
      vector<unsigned long> children = control->nodes->getConnectedNodes(id);
      vector<unsigned long> newNodes;
  
      for (unsigned int i = 0; i < children.size(); i++)  {
        bool found = false;
        for (unsigned int k = 0; k < present.size() && !found; k++)
          if (present[k] == children[i])
            found = true;
        if (found == false) {
          present.push_back(children[i]);
          newNodes.push_back(children[i]);
        }
      }

      for (unsigned int i = 0; i < newNodes.size(); i++) 
        for (unsigned int j = 0; j < simNodes.size(); j++)
          if (newNodes[i] == simNodes[j].index) {
            temp.clear();
            temp << QString::number(newNodes[i]) + ":" + QString::fromStdString(simNodes[j].name);
            QTreeWidgetItem *next = new QTreeWidgetItem(temp); 
            current->addChild(next);
            fill(newNodes[i], next);
          }
    }

    

    void NodeSelectionTree::createTree(unsigned long root)  {
      present.clear();
      control->nodes->getListNodes(&simNodes);
      unsigned int v_index = 0;
  
      for (v_index = 0; v_index < simNodes.size(); v_index++)
        if (simNodes[v_index].index == root)
          break;

      for (unsigned int i = v_index; i < simNodes.size(); i++)
        fill(simNodes[i].index);

      for (unsigned int i = 0; i < v_index; i++)
        fill(simNodes[i].index);

      treeWidget->expandAll();
    }


    void NodeSelectionTree::createList(void)
    {
      control->nodes->getListNodes(&simNodes);
      for (unsigned int i = 0; i < simNodes.size(); i++) {
        QStringList temp;
        temp << QString::number(simNodes[i].index) + ":" + QString::fromStdString(simNodes[i].name);
        QTreeWidgetItem *current = new QTreeWidgetItem(temp);
        treeWidget->addTopLevelItem(current);
      }
    }


    void NodeSelectionTree::reset(void)
    {
      while (treeWidget->topLevelItemCount() > 0)
        (void)treeWidget->takeTopLevelItem(0);
      present.clear();
    }


    void NodeSelectionTree::selectRecursively(QTreeWidgetItem *current, bool mode)
    {
      if (current != NULL)
        for (int i = 0; i < current->childCount(); i++) {
          current->child(i)->setSelected(mode);
          int n = current->child(i)->text(0).indexOf(":");
          unsigned long node_id = current->child(i)->text(0).left(n).toULong();
          control->graphics->setDrawObjectSelected(control->nodes->getDrawID(node_id), mode);
          selectRecursively(current->child(i), mode);
        }
    }


    QTreeWidgetItem* NodeSelectionTree::findByNodeId(unsigned long id, QTreeWidgetItem* parent) {
      if (parent == NULL) { // top level
        for (int i = 0; i < treeWidget->topLevelItemCount(); i++) {
          if (treeWidget->topLevelItem(i)->text(0).startsWith(QString::number(id) + ":"))
            return treeWidget->topLevelItem(i);
          else {
            QTreeWidgetItem *retval = findByNodeId(id, treeWidget->topLevelItem(i));
            if (retval != NULL)
              return retval;
          }
        }
      } else {
        for (int i = 0; i < parent->childCount(); i++) {
          if (parent->child(i)->text(0).startsWith(QString::number(id) + ":"))
            return parent->child(i);
          else {
            QTreeWidgetItem *retval = findByNodeId(id, parent->child(i));
            if (retval != NULL)
              return retval;
          }
        }
      }
  
      return NULL;
    }



    void NodeSelectionTree::selectEvent(unsigned long int id, bool mode) {
      if (select_allowed == false)
        return;
  
      select_allowed = false;
      QTreeWidgetItem *chosen = findByNodeId(id);
      chosen->setSelected(mode);
      if (node_selection_mode->value().toInt() == 1)
        selectRecursively(chosen, mode);
      select_allowed = true;
    }

    void NodeSelectionTree::selectNodes(void) {
      if (select_allowed == false)
        return;
      select_allowed = false;
  
      QList<QTreeWidgetItem*> selectedItems = treeWidget->selectedItems();

      if (node_selection_mode->value().toInt() == 1)   
        for (unsigned int i = 0; i < (unsigned int)selectedItems.size(); i++) 
          selectRecursively(selectedItems[i], true);
  
      vector<unsigned long> selectedIds = selectedNodes();
  
      for (unsigned int i = 0; i < simNodes.size(); i++) {
        bool found = false;
        int draw_id = control->nodes->getDrawID(simNodes[i].index);
        if (!draw_id)
          continue;
        for (unsigned int j = 0; j < selectedIds.size() && !found; j++) 
          if (simNodes[i].index == selectedIds[j])
            found = true;
        control->graphics->setDrawObjectSelected(draw_id, found);
      }
      select_allowed = true;
    }


    vector<unsigned long> NodeSelectionTree::selectedNodes(void)
    {
      QList<QTreeWidgetItem*> selectedItems = treeWidget->selectedItems();
      vector<unsigned long> selectedIds;
      for (unsigned int i = 0; i < (unsigned int)selectedItems.size(); i++) {
        int n = selectedItems[i]->text(0).indexOf(":");
        selectedIds.push_back(selectedItems[i]->text(0).left(n).toULong());
      }
      return selectedIds;
    }


    void NodeSelectionTree::closeEvent(QCloseEvent* event) {
      (void)event;
      emit closeSignal(this);
    }

  } // end of namespace gui
} // end of namespace mars
