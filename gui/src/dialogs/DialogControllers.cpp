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

#include "DialogControllers.h"

#include <mars/main_gui/GuiInterface.h>

#include <mars/interfaces/sim/ControllerManagerInterface.h>

namespace mars {
  namespace gui {

    DialogControllers::DialogControllers(interfaces::ControlCenter *c,
                                         main_gui::GuiInterface *gui) :
      main_gui::BaseWidget(0, c->cfg, "DialogControllers"),
      pDialog(new main_gui::PropertyDialog(NULL)), mainGui(gui) {

      control = c;
      filled = false;
      oldFocus = NULL;

      pDialog->setAttribute(Qt::WA_DeleteOnClose);
      pDialog->setWindowTitle(tr("Controllers"));
      pDialog->setPropCallback(dynamic_cast<main_gui::PropertyCallback*>(this));
      QObject::connect(pDialog, SIGNAL(closeSignal()),
                       this, SLOT(closeDialog()));
      pDialog->setViewButtonVisibility(false);
      pDialog->setViewMode(main_gui::TreeViewMode);
      pDialog->clearButtonBox();
      addButton = pDialog->addGenericButton("Add", this, SLOT(on_add_controller()));
      removeButton = pDialog->addGenericButton("Remove", this, 
                                               SLOT(on_remove_controller()));
      pDialog->addGenericButton("New", this, SLOT(on_new_controller()));
      pDialog->addGenericButton("Nodes", this, SLOT(on_nodes_selection()));
      pDialog->addGenericButton("Sensors", this, SLOT(on_sensors_selection()));
      pDialog->addGenericButton("Motors", this, SLOT(on_motors_selection()));
      removeButton->hide();
      addButton->hide();

      control->controllers->getListController(&allControllers);
      for (unsigned int i = 0; i<allControllers.size(); i++) {
        QtVariantProperty *tmp = 
          pDialog->addGenericProperty("../"+allControllers[i].name,
                                      QtVariantPropertyManager::groupTypeId(),
                                      0);
        allControllers_p.push_back(tmp);
        ControllerHandler *den = new ControllerHandler(tmp, allControllers[i].index, pDialog, control, ControllerTree::EditMode);
        allDialogs.push_back(den);
      }						  
      filled = true;
    }

    DialogControllers::~DialogControllers() {
    }

    void DialogControllers::closeDialog() {

      for (unsigned int i = 0; i < newDialogs.size(); i++) 
        delete newDialogs[i];
      for (unsigned int i = 0; i < allDialogs.size(); i++) 
        delete allDialogs[i];
  
      allControllers_p.clear();
      allDialogs.clear();
      newControllers_p.clear();
      newDialogs.clear();
    
      if (pDialog) {
        mainGui->removeDockWidget((void*)pDialog);
        delete pDialog;
        pDialog = NULL;
      }
    }  




    void DialogControllers::valueChanged(QtProperty* property,
                                         const QVariant& value) {
      if (filled == false) 
        return;

      for (unsigned int i = 0; i < newControllers_p.size(); i++)
        if (pDialog->currentItem() == newControllers_p[i]) {
          newDialogs[i]->valueChanged(property, value);
          return;
        }
      for (unsigned int i = 0; i < allControllers_p.size(); i++)
        if (pDialog->currentItem() == allControllers_p[i]) {
          allDialogs[i]->valueChanged(property, value);
          return;
        }  
    }

    void DialogControllers::topLevelItemChanged(QtProperty *property) {
      if (filled == false)
        return;
  
      if (property == oldFocus)
        return;
      oldFocus = property;

      if (allControllers_p.size() == 0 && newControllers_p.size() == 0) {
        removeButton->hide();
        addButton->hide();
        return;
      } else     
        removeButton->show();

      for (unsigned int i = 0; i < newControllers_p.size(); i++)
        if (property == newControllers_p[i]) {
          addButton->show();
          return;
        }
      for (unsigned int i = 0; i < allControllers_p.size(); i++)
        if (property == allControllers_p[i]) {
          addButton->hide();
          return;
        }

    }


    void DialogControllers::on_new_controller() {
      filled = false;
      static int index = 0;
      std::string newName = "NewController" + QString::number(index++).toStdString();
      QtVariantProperty *tmp = 
        pDialog->addGenericProperty("../" + newName,
                                    QtVariantPropertyManager::groupTypeId(),
                                    0);
      newControllers_p.push_back(tmp);
      ControllerHandler *dnn = new ControllerHandler(tmp, 0, pDialog, control, ControllerTree::PreviewMode);
      newDialogs.push_back(dnn);
      filled = true;

      pDialog->setCurrentItem(tmp);
    }

    void DialogControllers::on_remove_controller() {
      filled = false;

      for (unsigned int i = 0; i < newControllers_p.size(); i++)
        if (pDialog->currentItem() == newControllers_p[i]) {
          delete newDialogs[i];
          newDialogs.erase(newDialogs.begin() + i);
          newControllers_p.erase(newControllers_p.begin() + i);
          break;
        }
      
      for (unsigned int i = 0; i < allControllers_p.size(); i++)
        if (allControllers_p[i] == pDialog->currentItem()) {
          control->controllers->removeController(allControllers[i].index);
          allControllers.erase(allControllers.begin() + i);
          delete allDialogs[i];
          allDialogs.erase(allDialogs.begin() + i);
          allControllers_p.erase(allControllers_p.begin() + i);
          break;
        }

      filled = true;

      if (allControllers_p.size() == 0 && newControllers_p.size() == 0) {
        removeButton->hide();
        addButton->hide();
      } else if (allControllers_p.size() > 0) {
        pDialog->setCurrentItem(allControllers_p[0]);
      } else if (newControllers_p.size() > 0) {
        pDialog->setCurrentItem(newControllers_p[0]);
      }
 
    }

    void DialogControllers::on_add_controller() {
      filled = false;
      for (unsigned int i = 0; i < newControllers_p.size(); i++) 
        if (pDialog->currentItem() == newControllers_p[i]) {
          newDialogs[i]->accept();
          allControllers_p.push_back(newControllers_p[i]);
          allDialogs.push_back(newDialogs[i]);
          newControllers_p.erase(newControllers_p.begin() + i);
          newDialogs.erase(newDialogs.begin() + i);
          topLevelItemChanged(allControllers_p.back());
          break;
        }
    
      control->controllers->getListController(&allControllers);

      filled = true;
      addButton->hide();
    }




    void DialogControllers::on_nodes_selection() {
      for (unsigned int i = 0; i < allControllers_p.size(); i++) 
        if (pDialog->currentItem() == allControllers_p[i]) 
          allDialogs[i]->nodes_selection();
      for (unsigned int i = 0; i < newControllers_p.size(); i++) 
        if (pDialog->currentItem() == newControllers_p[i]) 
          newDialogs[i]->nodes_selection();
    }



    void DialogControllers::on_sensors_selection() {
      for (unsigned int i = 0; i < allControllers_p.size(); i++) 
        if (pDialog->currentItem() == allControllers_p[i]) 
          allDialogs[i]->sensors_selection();
      for (unsigned int i = 0; i < newControllers_p.size(); i++) 
        if (pDialog->currentItem() == newControllers_p[i]) 
          newDialogs[i]->sensors_selection();
    }



    void DialogControllers::on_motors_selection() {
      for (unsigned int i = 0; i < allControllers_p.size(); i++) 
        if (pDialog->currentItem() == allControllers_p[i]) 
          allDialogs[i]->motors_selection();
      for (unsigned int i = 0; i < newControllers_p.size(); i++) 
        if (pDialog->currentItem() == newControllers_p[i]) 
          newDialogs[i]->motors_selection();
    }

  } // end of namespace gui
} // end of namespace mars
