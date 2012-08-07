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

#include "Dialog_Add_Force.h"

#include <mars/main_gui/GuiInterface.h>

#include <mars/interfaces/sim/NodeManagerInterface.h>

using namespace std;

namespace mars {
  namespace gui {

    /**
     * Adds a QDialog to set force to nodes
     */
    Dialog_Add_Force::Dialog_Add_Force(interfaces::ControlCenter* c,
                                       main_gui::GuiInterface *gui)
      : main_gui::BaseWidget(0, c->cfg, "Dialog_Add_Force"),
        pDialog( new main_gui::PropertyDialog(0)), mainGui(gui) {
  
      control = c;
      pDialog->setAttribute(Qt::WA_DeleteOnClose);
      pDialog->setWindowTitle(tr("Apply Force"));
      pDialog->setPropCallback(dynamic_cast<main_gui::PropertyCallback*>(this));
      QObject::connect(pDialog, SIGNAL(closeSignal()), this, SLOT(closeDialog()));

      control->nodes->getListNodes(this->getNodeListPtr());

      std::map<QString, QVariant> attr;
      attr.insert(pair<QString, QVariant>(QString("decimals"), 9));
      attr.insert(pair<QString, QVariant>(QString("singleStep"), 0.1));
      QStringList enumNames;
      for (unsigned int i = 0; i <myNodes.size(); i++)
        enumNames << QString::fromStdString(myNodes[i].name);

      node = pDialog->addGenericProperty("../Node", QtVariantPropertyManager::enumTypeId(),
                                         QVariant(1), NULL, &enumNames);
      vector_x = pDialog->addGenericProperty("../Vector/x", QVariant::Double, 1.0, &attr);
      vector_y = pDialog->addGenericProperty("../Vector/y", QVariant::Double, 0.0, &attr);
      vector_z = pDialog->addGenericProperty("../Vector/z", QVariant::Double, 0.0, &attr);
      magnitude = pDialog->addGenericProperty("../Magnitude [N]", QVariant::Double, 10, &attr);

    }

    Dialog_Add_Force::~Dialog_Add_Force() {
    }

    /**
     * apply force to selected node
     */
    void Dialog_Add_Force::accept() {
      utils::Vector forcevec;
      utils::Vector at;
      at.x() = 0;at.y()=0,at.z()=0;

      forcevec.x() = vector_x->value().toDouble();
      forcevec.y() = vector_y->value().toDouble();
      forcevec.z() = vector_z->value().toDouble();
      forcevec.normalize();
  
      forcevec *= magnitude->value().toDouble();
      int index = node->value().toInt();
  
      control->nodes->applyForce(myNodes[index].index, forcevec);

    }


    void Dialog_Add_Force::reject() {
      if (pDialog)  pDialog->close();
    }


    void Dialog_Add_Force::closeDialog() {
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
    std::vector<interfaces::core_objects_exchange>* Dialog_Add_Force::getNodeListPtr(void) {
      return &myNodes;
    }

  } // end of namespace gui
} // end of namespace mars

