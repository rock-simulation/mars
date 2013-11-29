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

#include "DialogNodes.h"

#include <mars/main_gui/GuiInterface.h>

#include <mars/interfaces/graphics/GraphicsManagerInterface.h>
#include <mars/interfaces/sim/NodeManagerInterface.h>

#include <QPushButton>
#include <QMessageBox>
#include <QHBoxLayout>

namespace mars {
  namespace gui {

    DialogNodes::DialogNodes(interfaces::ControlCenter *c, main_gui::GuiInterface *gui) :
      main_gui::BaseWidget(0, c->cfg, "DialogNodes"),
      pDialog(new main_gui::PropertyDialog(NULL)), mainGui(gui) {

      control = c;
      filled = false;
      oldFocus = NULL;

      if(control->graphics) {
        control->graphics->addEventClient((interfaces::GraphicsEventClient*)this);
      }

      this->setWindowTitle(tr("Nodes"));

      pDialog->setPropCallback(this);
      pDialog->setViewButtonVisibility(false);
      pDialog->setViewMode(main_gui::TreeViewMode);
      pDialog->clearButtonBox();

      QHBoxLayout *hLayout = new QHBoxLayout;
      this->setLayout(hLayout);
      hLayout->addWidget(pDialog);

      stateButton = pDialog->addGenericButton("State", this, SLOT(on_node_state()));
      addButton = pDialog->addGenericButton("Add", this, SLOT(on_add_node()));
      removeButton = pDialog->addGenericButton("Remove", this, 
                                               SLOT(on_remove_node()));
      pDialog->addGenericButton("New", this, SLOT(on_new_node()));
      stateButton->hide();
      removeButton->hide();
      addButton->hide();

      control->nodes->getListNodes(&allNodes);
      for (unsigned int i = 0; i<allNodes.size(); i++) {
        QtVariantProperty *tmp = 
          pDialog->addGenericProperty("../"+allNodes[i].name,
                                      QtVariantPropertyManager::groupTypeId(),
                                      0);
        allNodes_p.push_back(tmp);
        NodeHandler *den = new NodeHandler(tmp, allNodes[i].index, pDialog, control, NodeTree::EditMode);
        allDialogs.push_back(den);
      }						  
      filled = true;
    }

    DialogNodes::~DialogNodes() {
      if(control->graphics) {
        control->graphics->removeEventClient((GraphicsEventClient*)this);
      }

      for (unsigned int i = 0; i < newDialogs.size(); i++) 
        delete newDialogs[i];
      for (unsigned int i = 0; i < allDialogs.size(); i++) 
        delete allDialogs[i];
  
      allNodes_p.clear();
      allDialogs.clear();
      newNodes_p.clear();
      newDialogs.clear();
      delete pDialog;
    }

    void DialogNodes::valueChanged(QtProperty* property, const QVariant& value)
    {
      if (filled == false) 
        return;

      for (unsigned int i = 0; i < newNodes_p.size(); i++)
        if (pDialog->currentItem() == newNodes_p[i]) {
          newDialogs[i]->valueChanged(property, value);
          return;
        }
      for (unsigned int i = 0; i < allNodes_p.size(); i++)
        if (pDialog->currentItem() == allNodes_p[i]) {
          allDialogs[i]->valueChanged(property, value);
          return;
        }  

    }

    void DialogNodes::topLevelItemChanged(QtProperty *property) {
      if (filled == false)
        return;

      if (property == oldFocus)
        return;
      oldFocus = property;

      if (allNodes_p.size() == 0 && newNodes_p.size() == 0) {
        removeButton->hide();
        addButton->hide();
        stateButton->hide();
      } else     
        removeButton->show();

      for (unsigned int i = 0; i < newNodes_p.size(); i++)
        newDialogs[i]->focusOut();

      for (unsigned int i = 0; i < allNodes_p.size(); i++)
        if (!(allDialogs[i]->isSelected()))
          allDialogs[i]->focusOut();
  

      for (unsigned int i = 0; i < newNodes_p.size(); i++)
        if (property == newNodes_p[i]) {
          newDialogs[i]->focusIn();
          stateButton->hide();
          addButton->show();
          return;
        }
      for (unsigned int i = 0; i < allNodes_p.size(); i++)
        if (property == allNodes_p[i] && !(allDialogs[i]->isSelected())) {
          allDialogs[i]->focusIn();
          stateButton->show();
          addButton->hide();
          return;
        }
      addButton->hide();
      stateButton->hide();
    }


    void DialogNodes::on_new_node() {
      filled = false;
      static int index = 0;
      std::string newName = "NewNode" + QString::number(index++).toStdString();
      QtVariantProperty *tmp = 
        pDialog->addGenericProperty("../" + newName,
                                    QtVariantPropertyManager::groupTypeId(),
                                    0);
      newNodes_p.push_back(tmp);
      NodeHandler *dnn = new NodeHandler(tmp, 0, pDialog, control, NodeTree::PreviewMode);
      newDialogs.push_back(dnn);
      filled = true;

      pDialog->setCurrentItem(tmp);
    }

    void DialogNodes::on_remove_node() {
      filled = false;

      for (unsigned int i = 0; i < newNodes_p.size(); i++)
        if (pDialog->currentItem() == newNodes_p[i]) {
          delete newDialogs[i];
          newDialogs.erase(newDialogs.begin() + i);
          newNodes_p.erase(newNodes_p.begin() + i);
          break;
        }
      
      for (unsigned int i = 0; i < allNodes_p.size(); i++)
        if (allNodes_p[i] == pDialog->currentItem()) {
          control->nodes->removeNode(allNodes[i].index);
          allNodes.erase(allNodes.begin() + i);
          delete allDialogs[i];
          allDialogs.erase(allDialogs.begin() + i);
          allNodes_p.erase(allNodes_p.begin() + i);
          break;
        }

      filled = true;

      if (allNodes_p.size() == 0 && newNodes_p.size() == 0) {
        removeButton->hide();
        addButton->hide();
        stateButton->hide();
      } else if (allNodes_p.size() > 0) {
        pDialog->setCurrentItem(allNodes_p[0]);
      } else if (newNodes_p.size() > 0) {
        pDialog->setCurrentItem(newNodes_p[0]);
      }
 
    }

    void DialogNodes::on_add_node() {
      filled = false;
      for (unsigned int i = 0; i < newNodes_p.size(); i++)
        if (pDialog->currentItem() == newNodes_p[i]) {
          if ((newDialogs[i]->accept()) != 0) {
            on_remove_node();
          } else {
            allNodes_p.push_back(newNodes_p[i]);
            allDialogs.push_back(newDialogs[i]);
            oldFocus = NULL;
            newNodes_p.erase(newNodes_p.begin() + i);
            newDialogs.erase(newDialogs.begin() + i);
            topLevelItemChanged(allNodes_p.back());
          }
          break;
        }
      control->nodes->getListNodes(&allNodes);

      filled = true;
      addButton->hide();
      stateButton->show();
    }


    void DialogNodes::selectEvent(unsigned long int id, bool mode) {
      for (unsigned int i = 0; i < allNodes.size() ; i++)
        if (allNodes[i].index == id) {
          if (mode) {
            allDialogs[i]->setSelected(true);
            allDialogs[i]->focusIn();
            pDialog->expandTree(allNodes_p[i]);
            pDialog->expandTree(allDialogs[i]->getGeometryProp());
            pDialog->expandTree(allDialogs[i]->getPositionProp());
            pDialog->expandTree(allDialogs[i]->getRotationProp());	
          }	else {
            allDialogs[i]->focusOut();
            allDialogs[i]->setSelected(false);
          }
          break;
        }
    }

    void DialogNodes::on_node_state() {
      filled = false;
      for (unsigned int i = 0; i < allNodes_p.size(); i++)
        if (allNodes_p[i] == pDialog->currentItem()) {
          allDialogs[i]->showState();
          break;
        }
      filled = true;
    }

    void DialogNodes::closeEvent(QCloseEvent* event) {
      (void)event;
      emit closeSignal(this);
    }

  } // end of namespace gui
} // end of namespace mars
