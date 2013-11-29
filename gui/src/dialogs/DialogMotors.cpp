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


#include "DialogMotors.h"

#include <mars/main_gui/GuiInterface.h>

#include <mars/interfaces/sim/MotorManagerInterface.h>
#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/sim/JointManagerInterface.h>

#include <QPushButton>
#include <QMessageBox>

namespace mars {
  namespace gui {

    DialogMotors::DialogMotors(interfaces::ControlCenter *c, main_gui::GuiInterface *gui) :
      main_gui::BaseWidget(0, c->cfg, "DialogMotors"),
      pDialog(new main_gui::PropertyDialog(NULL)), mainGui(gui) {

      control = c;
      filled = false;
      oldFocus = NULL;

      pDialog->setAttribute(Qt::WA_DeleteOnClose);
      pDialog->setWindowTitle(tr("Motors"));
      pDialog->setPropCallback(dynamic_cast<main_gui::PropertyCallback*>(this));
      QObject::connect(pDialog, SIGNAL(closeSignal()),
                       this, SLOT(closeDialog()));
      pDialog->setViewButtonVisibility(false);
      pDialog->setViewMode(main_gui::TreeViewMode);
      pDialog->clearButtonBox();
      addButton = pDialog->addGenericButton("Add", this, SLOT(on_add_motor()));
      removeButton = pDialog->addGenericButton("Remove", this, 
                                               SLOT(on_remove_motor()));
      pDialog->addGenericButton("New", this, SLOT(on_new_motor()));
      removeButton->hide();
      addButton->hide();

      control->motors->getListMotors(&allMotors);
      for (unsigned int i = 0; i<allMotors.size(); i++) {
        QtVariantProperty *tmp = 
          pDialog->addGenericProperty("../"+allMotors[i].name,
                                      QtVariantPropertyManager::groupTypeId(),
                                      0);
        allMotors_p.push_back(tmp);
        MotorHandler *den = new MotorHandler(tmp, allMotors[i].index, pDialog, control, MotorTree::EditMode);
        allDialogs.push_back(den);
      }						  
      filled = true;
    }

    DialogMotors::~DialogMotors() {
    }

    void DialogMotors::closeDialog() {

      for (unsigned int i = 0; i < newDialogs.size(); i++) 
        delete newDialogs[i];
      for (unsigned int i = 0; i < allDialogs.size(); i++) 
        delete allDialogs[i];
  
      allMotors_p.clear();
      allDialogs.clear();
      newMotors_p.clear();
      newDialogs.clear();
    
      if (pDialog) {
        mainGui->removeDockWidget((void*)pDialog);
        delete pDialog;
        pDialog = NULL;
      }
    }

    void DialogMotors::valueChanged(QtProperty* property, const QVariant& value) {
      if (filled == false) 
        return;

      for (unsigned int i = 0; i < newMotors_p.size(); i++)
        if (pDialog->currentItem() == newMotors_p[i]) {
          newDialogs[i]->valueChanged(property, value);
          return;
        }
      for (unsigned int i = 0; i < allMotors_p.size(); i++)
        if (pDialog->currentItem() == allMotors_p[i]) {
          allDialogs[i]->valueChanged(property, value);
          return;
        }  

    }

    void DialogMotors::topLevelItemChanged(QtProperty *property) {
      if (filled == false)
        return;
  
      if (property == oldFocus)
        return;
      oldFocus = property;

      if (allMotors_p.size() == 0 && newMotors_p.size() == 0) {
        removeButton->hide();
        addButton->hide();
        return;
      } else     
        removeButton->show();

      for (unsigned int i = 0; i < newMotors_p.size(); i++)
        newDialogs[i]->focusOut();
      for (unsigned int i = 0; i < allMotors_p.size(); i++)
        allDialogs[i]->focusOut();

      for (unsigned int i = 0; i < newMotors_p.size(); i++)
        if (property == newMotors_p[i]) {
          newDialogs[i]->focusIn();
          addButton->show();
          return;
        }
      for (unsigned int i = 0; i < allMotors_p.size(); i++)
        if (property == allMotors_p[i]) {
          allDialogs[i]->focusIn();
          addButton->hide();
          return;
        }
      addButton->hide();
    }

    void DialogMotors::on_new_motor() {
      if (control->nodes->getNodeCount() == 0 || 
          control->joints->getJointCount() == 0) {
        QMessageBox::information( 0, "Simulation", "Please create a node and a joint first",
                                  "OK", 0); // ok == button 0
        return;
      }

      filled = false;
      static int index = 0;
      std::string newName = "NewMotor" + QString::number(index++).toStdString();
      QtVariantProperty *tmp = 
        pDialog->addGenericProperty("../" + newName,
                                    QtVariantPropertyManager::groupTypeId(),
                                    0);
      newMotors_p.push_back(tmp);
      MotorHandler *dnn = new MotorHandler(tmp, 0, pDialog, control, MotorTree::PreviewMode);
      newDialogs.push_back(dnn);
      filled = true;

      pDialog->setCurrentItem(tmp);
    }

    void DialogMotors::on_remove_motor() {
      filled = false;

      for (unsigned int i = 0; i < newMotors_p.size(); i++)
        if (pDialog->currentItem() == newMotors_p[i]) {
          delete newDialogs[i];
          newDialogs.erase(newDialogs.begin() + i);
          newMotors_p.erase(newMotors_p.begin() + i);
          break;
        }
      
      for (unsigned int i = 0; i < allMotors_p.size(); i++)
        if (allMotors_p[i] == pDialog->currentItem()) {
          control->motors->removeMotor(allMotors[i].index);
          allMotors.erase(allMotors.begin() + i);
          delete allDialogs[i];
          allDialogs.erase(allDialogs.begin() + i);
          allMotors_p.erase(allMotors_p.begin() + i);
          break;
        }

      filled = true;

      if (allMotors_p.size() == 0 && newMotors_p.size() == 0) {
        removeButton->hide();
        addButton->hide();
      } else if (allMotors_p.size() > 0) {
        pDialog->setCurrentItem(allMotors_p[0]);
      } else if (newMotors_p.size() > 0) {
        pDialog->setCurrentItem(newMotors_p[0]);
      }
 
    }

    void DialogMotors::on_add_motor() {
      filled = false;
      for (unsigned int i = 0; i < newMotors_p.size(); i++) 
        if (pDialog->currentItem() == newMotors_p[i]) {
          newDialogs[i]->accept();
          allMotors_p.push_back(newMotors_p[i]);
          allDialogs.push_back(newDialogs[i]);
          newMotors_p.erase(newMotors_p.begin() + i);
          newDialogs.erase(newDialogs.begin() + i);
          topLevelItemChanged(allMotors_p.back());
          break;
        }
    
      control->motors->getListMotors(&allMotors);

      filled = true;
      addButton->hide();
    }

  } // end of namespace gui
} // end of namespace mars
