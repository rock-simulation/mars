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

#include "Dialog_Rescale_Environment.h"
#include <mars/main_gui/GuiInterface.h>
#include <mars/interfaces/sim/SimulatorInterface.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>

using namespace std;

namespace mars {
  namespace gui {

    Dialog_Rescale_Environment::Dialog_Rescale_Environment(interfaces::ControlCenter* c,
                                                           main_gui::GuiInterface *gui)
      : main_gui::BaseWidget(0, c->cfg, "Dialog_Rescale_Environment"),
        pDialog(new main_gui::PropertyDialog(0)), mainGui(gui) {

      control = c;
      pDialog->setAttribute(Qt::WA_DeleteOnClose);
      pDialog->setWindowTitle(tr("Rescale Environment"));
      pDialog->setPropCallback(dynamic_cast<PropertyCallback*>(this));
      QObject::connect(pDialog, SIGNAL(closeSignal()), this, SLOT(closeDialog()));

      map<QString, QVariant> attr;
      attr.insert(pair<QString, QVariant>(QString("minimum"), 1e-16));
      attr.insert(pair<QString, QVariant>(QString("singleStep"), 0.1));
      attr.insert(pair<QString, QVariant>(QString("decimals"), 9)); 

      x_factor = pDialog->addGenericProperty("../X Factor", QVariant::Double, 1, &attr);
      y_factor = pDialog->addGenericProperty("../Y Factor", QVariant::Double, 1, &attr);
      z_factor = pDialog->addGenericProperty("../Z Factor", QVariant::Double, 1, &attr);

    }


    Dialog_Rescale_Environment::~Dialog_Rescale_Environment() {
    }

    void Dialog_Rescale_Environment::accept() {
      control->sim->rescaleEnvironment(x_factor->value().toDouble(),
                                       y_factor->value().toDouble(),
                                       z_factor->value().toDouble());
      if (pDialog) pDialog->close();
    }

    void Dialog_Rescale_Environment::reject() {
      if (pDialog) pDialog->close();
    }

    void Dialog_Rescale_Environment::valueChanged(QtProperty* property, const QVariant& value) {
      (void)value;
      (void)property;
    }

    void Dialog_Rescale_Environment::closeDialog() {
      if (pDialog) {
        mainGui->removeDockWidget((void*)pDialog);
        delete pDialog;
        pDialog = NULL;
      }
      control->graphics->closeAxis();
    }

  } // end of namespace gui
} // end of namespace mars

