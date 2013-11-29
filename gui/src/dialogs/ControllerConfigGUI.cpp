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

#include "ControllerConfigGUI.h"

#include <mars/main_gui/GuiInterface.h>

#include <mars/interfaces/sim/ControllerManagerInterface.h>

#include <QHBoxLayout>

namespace mars {
  namespace gui {

    ControllerConfigGUI::ControllerConfigGUI(interfaces::ControlCenter* c,
                                             main_gui::GuiInterface *gui) :
      main_gui::BaseWidget(0, c->cfg, "ControllerConfigGUI"),
      pDialog(new main_gui::PropertyDialog(NULL)) {

      control = c;
      mainGui = gui;

      this->setWindowTitle(tr("Controller Configuration"));

      QHBoxLayout *hLayout = new QHBoxLayout;
      this->setLayout(hLayout);
      hLayout->addWidget(pDialog);

      pDialog->setPropCallback(this);

      take_events = false;
      filled = false;
    
      controllerElem = new ControllerElem;
      controllerElem->id = 0;  
      controllerElem->name = std::string("");
      controllerElem->index = 0;
      controllers.push_back(controllerElem);

      QStringList enumNames;
      state = pDialog->addGenericProperty("../State", QVariant::String, "No Controller Selected!");
      state->setEnabled(false);
      enumNames << "None selected";
      controllerIDCombo = pDialog->addGenericProperty("../Controller", QtVariantPropertyManager::enumTypeId(),
                                                      QVariant(0), NULL, &enumNames);
      autoModeCheck = pDialog->addGenericProperty("../Auto Connect", QVariant::Bool, false);
      ip_edit = pDialog->addGenericProperty("../IP", QVariant::String, "");
      port_edit = pDialog->addGenericProperty("../Port", QVariant::String, "");

      pDialog->clearButtonBox();
      pDialog->addGenericButton("Set IP", this, SLOT(setControllerIP()));
      pDialog->addGenericButton("Set Port", this, SLOT(setControllerPort()));
      pDialog->addGenericButton("Disconnect", this, SLOT(disconnectController()));
      pDialog->addGenericButton("Connect", this, SLOT(connectController()));

      startTimer(500);
      take_events = true;
      filled = true;
    }

    ControllerConfigGUI::~ControllerConfigGUI(void) {
      delete pDialog;
    }

    void ControllerConfigGUI::valueChanged(QtProperty *property, const QVariant &value)
    {
      if (filled == false) return;

      if (property == controllerIDCombo)
        updateGUI();
      else if (property == autoModeCheck)
        toggleControllerAutoMode(value.toBool());
    }



    void ControllerConfigGUI::toggleControllerAutoMode(bool checked) {
      if(take_events && controllerIDCombo->value().toInt() > 0) {
        unsigned long id = controllers[controllerIDCombo->value().toInt()]->id;
        control->controllers->setControllerAutoMode(id, checked);
      }
      else
        state->setValue("Select a controller first!");
    }

    void ControllerConfigGUI::setControllerIP(void) {
      if(take_events && controllerIDCombo->value().toInt() > 0) {
        unsigned long id = controllers[controllerIDCombo->value().toInt()]->id;
        control->controllers->setControllerIP(id, ip_edit->value().toString().toStdString());
        state->setValue("IP set!");
      }
      else
        state->setValue("Select a controller first!");
    }


    void ControllerConfigGUI::setControllerPort(void) {
      if(take_events && controllerIDCombo->value().toInt() > 0) {
        unsigned long id = controllers[controllerIDCombo->value().toInt()]->id;
        control->controllers->setControllerPort(id, port_edit->value().toInt());
        state->setValue("Port set!");
      }
      else
        state->setValue("Select a controller first!");
    }

    void ControllerConfigGUI::connectController(void) {
      if(take_events && controllerIDCombo->value().toInt() > 0) {
        unsigned long id = controllers[controllerIDCombo->value().toInt()]->id;
        if(autoModeCheck->value().toBool() == true) autoModeCheck->setValue(false);
        control->controllers->connectController(id);
        state->setValue("Connected!");
      }
      else
        state->setValue("Select a controller first!");
    }

    void ControllerConfigGUI::disconnectController(void) {
      if(take_events && controllerIDCombo->value().toInt() > 0) {
        unsigned long id = controllers[controllerIDCombo->value().toInt()]->id;
        if(autoModeCheck->value().toBool() == true) autoModeCheck->setValue(false);
        control->controllers->disconnectController(id);
        state->setValue("Disconnected!");
      }
      else
        state->setValue("Select a controller first!");
    }

    void ControllerConfigGUI::timerEvent(QTimerEvent* event) {
      (void) event;
      std::vector<interfaces::core_objects_exchange> objectList;
      std::vector<interfaces::core_objects_exchange>::iterator iter;
      std::vector<ControllerElem*>::iterator jter;
      std::vector<ControllerElem*> addList;
      std::vector<ControllerElem*> deleteList;
      ControllerElem* newElem;
      unsigned long current_id = controllers[controllerIDCombo->value().toInt()]->id;
      int index = 1, switch_id = 0, count_delete = 0;
      bool found, first = 1;
      QStringList enumNames = controllerIDCombo->attributeValue("enumNames").toStringList();

      addList.push_back(controllerElem);

      control->controllers->getListController(&objectList);

      // first we delete the nodes that are not in the list
      for(jter=controllers.begin(); jter!=controllers.end(); ++jter) {
        if(first) first = 0;
        else {
          found = false;
          for(iter=objectList.begin(); iter!=objectList.end(); ++iter) {
            if((*jter)->id == (*iter).index) {
              found = true;
              (*jter)->index = index++;
              addList.push_back((*jter));
              objectList.erase(iter);          
              break;
            }
          }
          if(!found) {
            if((*jter)->id == current_id) switch_id = 1;
            // wir brauchen eine delete list weil wir von hinten löschen müssen
            enumNames.removeAt((*jter)->index - count_delete++);
            controllerIDCombo->setAttribute("enumNames", enumNames);
            delete (*jter);
          }
        }
      }

      // then we add the nodes that are still in the object list
      for(iter=objectList.begin(); iter!=objectList.end(); ++iter) {
        newElem = new ControllerElem;
        newElem->id = (*iter).index;
        newElem->name = (*iter).name;
        newElem->index = index++;
        addList.push_back(newElem);
        enumNames << QString::fromStdString((*iter).name);
        controllerIDCombo->setAttribute("enumNames", enumNames);
      }
      objectList.clear();

      controllers = addList;
      if(switch_id) controllerIDCombo->setValue(0);
    }

    void ControllerConfigGUI::updateGUI(void) {
      if(take_events && controllerIDCombo->value().toInt() > 0) {
        state->setValue("Controller selected!");
        unsigned long id = controllers[controllerIDCombo->value().toInt()-1]->id;
  
        take_events = false;
    
        autoModeCheck->setValue(control->controllers->getControllerAutoMode(id));

        ip_edit->setValue(QString::fromStdString((control->controllers->getControllerIP(id))));
        port_edit->setValue(QString::number(control->controllers->getControllerPort(id)));
    
        take_events = true;
      }
    }

    void ControllerConfigGUI::closeEvent(QCloseEvent *event) {
      (void)event;

      emit closeSignal(this);
    }

  } // end of namespace gui
} // end of namespace mars
