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

#include "DialogJoints.h"

#include <mars/main_gui/GuiInterface.h>
#include <mars/interfaces/sim/JointManagerInterface.h>
#include <mars/interfaces/sim/NodeManagerInterface.h>

#include <QPushButton>
#include <QHBoxLayout>
#include <QMessageBox>

using namespace std;

namespace mars {
  namespace gui {

    DialogJoints::DialogJoints(interfaces::ControlCenter* c,
                               main_gui::GuiInterface *gui,
                               string imagePath_)
      : main_gui::BaseWidget(0, c->cfg, "DialogJoints"),
        pDialog(new main_gui::PropertyDialog(NULL)), mainGui(gui),
        control(c),
        imagePath(imagePath_) {

      filled = false;  
      oldFocus = NULL;

      this->setWindowTitle(tr("Joints"));
      pDialog->setPropCallback(this);

      QHBoxLayout *hLayout = new QHBoxLayout;
      this->setLayout(hLayout);
      hLayout->addWidget(pDialog);

      pDialog->setViewButtonVisibility(false);
      pDialog->setViewMode(main_gui::TreeViewMode);
      pDialog->clearButtonBox();
      stateButton = pDialog->addGenericButton("State", this, SLOT(on_joint_state()));
      addButton = pDialog->addGenericButton("Add", this, SLOT(on_add_joint()));
      removeButton = pDialog->addGenericButton("Remove", this, 
                                               SLOT(on_remove_joint()));
      pDialog->addGenericButton("New", this, SLOT(on_new_joint()));

      previewButton = pDialog->addGenericButton("Preview", this, SLOT(on_preview()));
      stateButton->hide();
      previewButton->hide();
      removeButton->hide();
      addButton->hide();

      control->joints->getListJoints(&allJoints);
      for (unsigned int i = 0; i<allJoints.size(); i++) {
        QtVariantProperty *tmp = 
          pDialog->addGenericProperty("../"+allJoints[i].name,
                                      QtVariantPropertyManager::groupTypeId(),
                                      0);
        allJoints_p.push_back(tmp);
        JointHandler *dej = new JointHandler(tmp, allJoints[i].index, pDialog, control,
                                             JointTree::EditMode, imagePath);
        allDialogs.push_back(dej);
      }						  
      filled = true;
    }

    DialogJoints::~DialogJoints() {
      for (unsigned int i = 0; i < newDialogs.size(); i++) 
        delete newDialogs[i];
      for (unsigned int i = 0; i < allDialogs.size(); i++) 
        delete allDialogs[i];
  
      allJoints_p.clear();
      allDialogs.clear();
      newJoints_p.clear();
      newDialogs.clear();
    
      delete pDialog;
    }

    void DialogJoints::valueChanged(QtProperty* property, const QVariant &value) 
    {
      if (filled == false) 
        return;

      for (unsigned int i = 0; i < newJoints_p.size(); i++)
        if (pDialog->currentItem() == newJoints_p[i]) {
          newDialogs[i]->valueChanged(property, value);
          return;
        }
      for (unsigned int i = 0; i < allJoints_p.size(); i++)
        if (pDialog->currentItem() == allJoints_p[i]) {
          allDialogs[i]->valueChanged(property, value);
          return;
        }  

    }

    void DialogJoints::topLevelItemChanged(QtProperty* property) {
      if (filled == false)
        return;

      if (property == oldFocus)
        return;

      oldFocus = property;

      if (allJoints_p.size() == 0 && newJoints_p.size() == 0) {
        removeButton->hide();
        previewButton->hide();
        addButton->hide();
        stateButton->hide();
      } else {
        removeButton->show();
      }

      for (unsigned int i = 0; i < newJoints_p.size(); i++)
        newDialogs[i]->focusOut();

      for (unsigned int i = 0; i < allJoints_p.size(); i++)
        allDialogs[i]->focusOut();
  

      for (unsigned int i = 0; i < newJoints_p.size(); i++)
        if (property == newJoints_p[i]) {
          newDialogs[i]->focusIn();
          addButton->show();
          stateButton->hide();
          previewButton->show();
          return;
        }
      for (unsigned int i = 0; i < allJoints_p.size(); i++)
        if (property == allJoints_p[i]) {
          allDialogs[i]->focusIn();
          addButton->hide();
          stateButton->show();
          previewButton->hide();
          return;
        }
      stateButton->hide();
      addButton->hide();
      previewButton->hide();
    }

    void DialogJoints::on_new_joint() {

      if (control->nodes->getNodeCount() == 0) {
        QMessageBox::information( 0, "Simulation", "Please create a node first", "OK", 0); 
        return;
      }

      filled = false;
      static int index = 0;
      string newName = "NewJoint" + QString::number(index++).toStdString();
      QtVariantProperty *tmp = 
        pDialog->addGenericProperty("../" + newName,
                                    QtVariantPropertyManager::groupTypeId(),
                                    0);
      newJoints_p.push_back(tmp);
      JointHandler *dnn = new JointHandler(tmp, 0, pDialog, control,
                                           JointTree::PreviewMode, imagePath);
      newDialogs.push_back(dnn);
      filled = true;

      pDialog->setCurrentItem(tmp);
    }

    void DialogJoints::on_remove_joint() {
      filled = false;

      for (unsigned int i = 0; i < newJoints_p.size(); i++)
        if (pDialog->currentItem() == newJoints_p[i]) {
          delete newDialogs[i];
          newDialogs.erase(newDialogs.begin() + i);
          newJoints_p.erase(newJoints_p.begin() + i);
          break;
        }
      
      for (unsigned int i = 0; i < allJoints_p.size(); i++)
        if (allJoints_p[i] == pDialog->currentItem()) {
          control->joints->removeJoint(allJoints[i].index);
          allJoints.erase(allJoints.begin() + i);
          delete allDialogs[i];
          allDialogs.erase(allDialogs.begin() + i);
          allJoints_p.erase(allJoints_p.begin() + i);
          break;
        }

      filled = true;

      if (allJoints_p.size() == 0 && newJoints_p.size() == 0) {
        stateButton->hide();
        removeButton->hide();
        addButton->hide();
        previewButton->hide();
      } else if (allJoints_p.size() > 0) {
        pDialog->setCurrentItem(allJoints_p[0]);
      } else if (newJoints_p.size() > 0) {
        pDialog->setCurrentItem(newJoints_p[0]);
      }
 
    }

    void DialogJoints::on_add_joint() {
      filled = false;

      for (unsigned int i = 0; i < newJoints_p.size(); i++)
        if (pDialog->currentItem() == newJoints_p[i]) {
          if ((newDialogs[i]->accept()) != 0) {
            return;
          } else {
            allJoints_p.push_back(newJoints_p[i]);
            allDialogs.push_back(newDialogs[i]);
            oldFocus = NULL;
            newJoints_p.erase(newJoints_p.begin() + i);
            newDialogs.erase(newDialogs.begin() + i);
            topLevelItemChanged(allJoints_p.back());
          }
          break;
        }
      control->joints->getListJoints(&allJoints);

      filled = true;
      addButton->hide();
      previewButton->hide();

    }

    void DialogJoints::on_preview() {
      for (unsigned int i = 0; i < newJoints_p.size(); i++)
        if (pDialog->currentItem() == newJoints_p[i]) 
          newDialogs[i]->previewLabel->show();
    }

    void DialogJoints::on_joint_state() {
      filled = false;
      for (unsigned int i = 0; i < allJoints_p.size(); i++)
        if (pDialog->currentItem() == allJoints_p[i]) {
          allDialogs[i]->showState();
          break;
        }
      filled = true;
    }

    void DialogJoints::closeEvent(QCloseEvent* event) {
      (void)event;
      emit closeSignal(this);
    }

  } // end of namespace gui
} // end of namespace mars
