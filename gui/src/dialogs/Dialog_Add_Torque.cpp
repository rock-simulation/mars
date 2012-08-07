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


#include "Dialog_Add_Torque.h"

#include <mars/main_gui/GuiInterface.h>

#include <mars/interfaces/sim/NodeManagerInterface.h>

namespace mars {
  namespace gui {
    /**
     * Adds a QDialog to set torque to nodes
     */
    Dialog_Add_Torque::Dialog_Add_Torque(interfaces::ControlCenter* c,
                                         main_gui::GuiInterface *gui)
      : main_gui::BaseWidget(0, c->cfg, "Dialog_Add_Torque"),
        pDialog( new main_gui::PropertyDialog(0)), mainGui(gui) {
  
      control = c;
      pDialog->setAttribute(Qt::WA_DeleteOnClose);
      pDialog->setWindowTitle(tr("Apply Torque"));
      pDialog->setPropCallback(dynamic_cast<main_gui::PropertyCallback*>(this));
      QObject::connect(pDialog, SIGNAL(closeSignal()), this, SLOT(closeDialog()));

      control->nodes->getListNodes(this->getNodeListPtr());

      std::map<QString, QVariant> attr;
      attr.insert(std::pair<QString, QVariant>(QString("decimals"), 9));
      attr.insert(std::pair<QString, QVariant>(QString("singleStep"), 0.1));
      QStringList enumNames;
      for (unsigned int i = 0; i <myNodes.size(); i++)
        enumNames << QString::fromStdString(myNodes[i].name);

      node = pDialog->addGenericProperty("../Node", QtVariantPropertyManager::enumTypeId(),
                                         QVariant(1), NULL, &enumNames);
      vector_x = pDialog->addGenericProperty("../Vector/x", QVariant::Double, 1.0, &attr);
      vector_y = pDialog->addGenericProperty("../Vector/y", QVariant::Double, 0.0, &attr);
      vector_z = pDialog->addGenericProperty("../Vector/z", QVariant::Double, 0.0, &attr);
      magnitude = pDialog->addGenericProperty("../Magnitude [N]", QVariant::Int, 10, &attr);

    }

    Dialog_Add_Torque::~Dialog_Add_Torque() {
    }

    /**
     * apply torque to selected node
     */
    void Dialog_Add_Torque::accept() {
      utils::Vector torquevec;
      utils::Vector at;
      at.x() = 0;at.y()=0,at.z()=0;

      torquevec.x() = vector_x->value().toDouble();
      torquevec.y() = vector_y->value().toDouble();
      torquevec.z() = vector_z->value().toDouble();
      torquevec.normalize();
  
      torquevec *= magnitude->value().toDouble();
      int index = node->value().toInt();
  
      control->nodes->applyTorque(myNodes[index].index, torquevec);
    }


    void Dialog_Add_Torque::reject() {
      if (pDialog)  pDialog->close();
    }


    void Dialog_Add_Torque::closeDialog() {
      if (pDialog) {
        mainGui->removeDockWidget((void*)pDialog);
        delete pDialog;
        pDialog = NULL;
      }
    }

    /** 
     * returns the pointer to the nodeList vector, so that the
     * vector can be filled by the MyMainWidnow
     */
    std::vector<interfaces::core_objects_exchange>* Dialog_Add_Torque::getNodeListPtr(void) {
      return &myNodes;
    }

  } // end of namespace gui
} // end of namespace mars

