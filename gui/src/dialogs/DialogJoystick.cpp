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


#include "DialogJoystick.h"
#include <iostream>

#include <mars/interfaces/sim/NodeManagerInterface.h>

#include <QHBoxLayout>

namespace mars {
  namespace gui {
    DialogJoystick::DialogJoystick(interfaces::ControlCenter* c)
      : main_gui::BaseWidget(0, c->cfg, "DialogJoystick"),
        pDialog(new main_gui::PropertyDialog(this)) {
      filled = false;
      control = c;

      this->setWindowTitle(tr("Joystick"));
      pDialog->setPropCallback(this);
      pDialog->hideAllButtons();

      control->nodes->getListNodes(&simNodes);

      QStringList enumNames;
      enumNames << "XY Plane" << "YZ Plane" << "XZ Plane" << "Custom";
      plane = pDialog->addGenericProperty("../Plane of movement", QtVariantPropertyManager::enumTypeId(),
                                          0, NULL, &enumNames);
      move_all = pDialog->addGenericProperty("../Move connected nodes", QVariant::Bool, true);

      joy = new JoystickWidget(pDialog);
      connect(joy, SIGNAL(moveSignal(double, double, double)),
              this, SLOT(moveSlot(double, double, double)));

      QHBoxLayout *layout = new QHBoxLayout;
      layout->setAlignment(Qt::AlignTop);
      layout->addWidget(joy);
      layout->addWidget(pDialog);
      this->setLayout(layout);

      filled = true;
    }

    DialogJoystick::~DialogJoystick() {
      delete pDialog;
    }

    void DialogJoystick::valueChanged(QtProperty *property,
                                      const QVariant &value) {
      if (!filled)
        return;

      if (property == plane) {
        if (value.toInt() == 3) {
          // custom plane : not implemented
        }
      }
      else
        joy->valueChanged(property, value);
    }

    void DialogJoystick::moveSlot(double x, double y, double strength) {
      if (x == 0 && y == 0)
        return;

      std::vector<unsigned long> selectedIds = nodeList->selectedNodes();

      int changes = interfaces::EDIT_NODE_POS;
      if (move_all->value().toBool())
        changes |= interfaces::EDIT_NODE_MOVE_ALL;
      interfaces::NodeData node;
      for (unsigned int i = 0; i < selectedIds.size(); i++) {
        node = control->nodes->getFullNode(selectedIds[i]);
        switch (plane->value().toInt()) {
        case 0:
          node.pos.x() += x*strength;
          node.pos.y() += y*strength;
          break;
        case 1:
          node.pos.z() += y*strength;
          node.pos.y() += x*strength;
          break;
        case 2:
          node.pos.x() += x*strength;
          node.pos.z() += y*strength;
          break;
        case 3: // custom plane
          break;
        default:
          break;
        }
        control->nodes->editNode(&node, changes);
      }
    }

    void DialogJoystick::closeEvent(QCloseEvent* event) {
      (void)event;
      emit closeSignal(this);
    }

  } // end of namespace gui
} // end of namespace mars
