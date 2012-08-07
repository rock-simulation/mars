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


#include "DialogSensors.h"

#include <mars/main_gui/GuiInterface.h>

#include <mars/interfaces/sim/SensorManagerInterface.h>

namespace mars {
  namespace gui {

    DialogSensors::DialogSensors(interfaces::ControlCenter *c,
                                 main_gui::GuiInterface *gui) :
      main_gui::BaseWidget(0, c->cfg, "DialogSensors"),
      pDialog(new main_gui::PropertyDialog(NULL)), mainGui(gui) {

      control = c;
      filled = false;
      oldFocus = NULL;

      pDialog->setAttribute(Qt::WA_DeleteOnClose);
      pDialog->setWindowTitle(tr("Sensors"));
      pDialog->setPropCallback(this);
      QObject::connect(pDialog, SIGNAL(closeSignal()),
                       this, SLOT(closeDialog()));
      pDialog->setViewButtonVisibility(false);
      pDialog->setViewMode(main_gui::TreeViewMode);
      pDialog->clearButtonBox();
      objectsButton = pDialog->addGenericButton("Objects", this, SLOT(on_objects_selection()));
      addButton = pDialog->addGenericButton("Add", this, SLOT(on_add_sensor()));
      removeButton = pDialog->addGenericButton("Remove", this, 
                                               SLOT(on_remove_sensor()));
      pDialog->addGenericButton("New", this, SLOT(on_new_sensor()));

      removeButton->hide();
      addButton->hide();
      objectsButton->hide();

      control->sensors->getListSensors(&allSensors);
      for (unsigned int i = 0; i<allSensors.size(); i++) {
        QtVariantProperty *tmp = 
          pDialog->addGenericProperty("../"+allSensors[i].name,
                                      QtVariantPropertyManager::groupTypeId(),
                                      0);
        allSensors_p.push_back(tmp);
        SensorHandler *den = new SensorHandler(tmp, allSensors[i].index, pDialog, control, SensorTree::EditMode);
        allDialogs.push_back(den);
      }						  
      filled = true;
    }

    DialogSensors::~DialogSensors() {
    }

    void DialogSensors::closeDialog() {

      for (unsigned int i = 0; i < newDialogs.size(); i++) 
        delete newDialogs[i];
      for (unsigned int i = 0; i < allDialogs.size(); i++) 
        delete allDialogs[i];
  
      allSensors_p.clear();
      allDialogs.clear();
      newSensors_p.clear();
      newDialogs.clear();
    
      if (pDialog) {
        mainGui->removeDockWidget((void*)pDialog);
        delete pDialog;
        pDialog = NULL;
      }
    }

    void DialogSensors::valueChanged(QtProperty* property, const QVariant& value)
    {
      if (filled == false) 
        return;

      for (unsigned int i = 0; i < newSensors_p.size(); i++)
        if (pDialog->currentItem() == newSensors_p[i]) {
          newDialogs[i]->valueChanged(property, value);
          return;
        }
      for (unsigned int i = 0; i < allSensors_p.size(); i++)
        if (pDialog->currentItem() == allSensors_p[i]) {
          allDialogs[i]->valueChanged(property, value);
          return;
        }  

    }

    void DialogSensors::topLevelItemChanged(QtProperty *property) {
      if (filled == false)
        return;
  
      if (property == oldFocus)
        return;
      oldFocus = property;

      if (allSensors_p.size() == 0 && newSensors_p.size() == 0) {
        removeButton->hide();
        addButton->hide();
        objectsButton->hide();
        return;
      } else {
        objectsButton->show();
        removeButton->show();
      }

      for (unsigned int i = 0; i < newSensors_p.size(); i++)
        if (property == newSensors_p[i]) {
          addButton->show();
          return;
        }
      for (unsigned int i = 0; i < allSensors_p.size(); i++)
        if (property == allSensors_p[i]) {
          addButton->hide();
          return;
        }

    }


    void DialogSensors::on_new_sensor() {
      filled = false;
      static int index = 0;
      std::string newName = "NewSensor" + QString::number(index++).toStdString();
      QtVariantProperty *tmp = 
        pDialog->addGenericProperty("../" + newName,
                                    QtVariantPropertyManager::groupTypeId(),
                                    0);
      newSensors_p.push_back(tmp);
      SensorHandler *dnn = new SensorHandler(tmp, 0, pDialog, control, SensorTree::PreviewMode);
      newDialogs.push_back(dnn);
      filled = true;

      pDialog->setCurrentItem(tmp);
    }

    void DialogSensors::on_remove_sensor() {
      filled = false;

      for (unsigned int i = 0; i < newSensors_p.size(); i++)
        if (pDialog->currentItem() == newSensors_p[i]) {
          delete newDialogs[i];
          newDialogs.erase(newDialogs.begin() + i);
          newSensors_p.erase(newSensors_p.begin() + i);
          break;
        }
      
      for (unsigned int i = 0; i < allSensors_p.size(); i++)
        if (allSensors_p[i] == pDialog->currentItem()) {
          control->sensors->removeSensor(allSensors[i].index);
          allSensors.erase(allSensors.begin() + i);
          delete allDialogs[i];
          allDialogs.erase(allDialogs.begin() + i);
          allSensors_p.erase(allSensors_p.begin() + i);
          break;
        }

      filled = true;

      if (allSensors_p.size() == 0 && newSensors_p.size() == 0) {
        removeButton->hide();
        addButton->hide();
      } else if (allSensors_p.size() > 0) {
        pDialog->setCurrentItem(allSensors_p[0]);
      } else if (newSensors_p.size() > 0) {
        pDialog->setCurrentItem(newSensors_p[0]);
      }
 
    }

    void DialogSensors::on_add_sensor() {
      filled = false;
      for (unsigned int i = 0; i < newSensors_p.size(); i++) 
        if (pDialog->currentItem() == newSensors_p[i]) {
          newDialogs[i]->accept();
          allSensors_p.push_back(newSensors_p[i]);
          allDialogs.push_back(newDialogs[i]);
          newSensors_p.erase(newSensors_p.begin() + i);
          newDialogs.erase(newDialogs.begin() + i);
          topLevelItemChanged(allSensors_p.back());
          break;
        }
    
      control->sensors->getListSensors(&allSensors);

      filled = true;
      addButton->hide();
    }

    void DialogSensors::on_objects_selection() {
      for (unsigned int i = 0; i < allSensors_p.size(); i++) 
        if (pDialog->currentItem() == allSensors_p[i]) 
          allDialogs[i]->selection();
      for (unsigned int i = 0; i < newSensors_p.size(); i++) 
        if (pDialog->currentItem() == newSensors_p[i]) 
          newDialogs[i]->selection();
    }

  } // end of namespace gui
} // end of namespace mars
