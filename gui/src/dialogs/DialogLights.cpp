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

#include "DialogLights.h"
#include <mars/main_gui/GuiInterface.h>

#include <mars/interfaces/graphics/GraphicsManagerInterface.h>

#include <QPushButton>
#include <QMessageBox>

using namespace std;

namespace mars {
  namespace gui {

    DialogLights::DialogLights(interfaces::ControlCenter *c,
                               main_gui::GuiInterface *gui) :
      main_gui::BaseWidget(0, c->cfg, "DialogLights"),
      pDialog(new main_gui::PropertyDialog(NULL)), mainGui(gui) {

      control = c;
      filled = false;
  
      pDialog->setAttribute(Qt::WA_DeleteOnClose);
      pDialog->setWindowTitle(tr("Lights"));
      pDialog->setPropCallback(dynamic_cast<main_gui::PropertyCallback*>(this));
      QObject::connect(pDialog, SIGNAL(closeSignal()),
                       this, SLOT(closeDialog()));
      pDialog->setViewButtonVisibility(false);
      pDialog->setViewMode(main_gui::TreeViewMode);
      pDialog->clearButtonBox();
      pDialog->addGenericButton("Add", this, SLOT(on_add_light()));
      removeButton = pDialog->addGenericButton("Remove", this, 
                                               SLOT(on_remove_light()));
      removeButton->hide();
  
      control->graphics->getLights(&allLights);
      for (unsigned int i =0; i < allLights.size(); i++) {
        QtVariantProperty *tmp = 
          pDialog->addGenericProperty("../NewLight", QtVariantPropertyManager::groupTypeId(), 0);
        allLights_p.push_back(tmp);
        LightHandler *lh = new LightHandler(tmp, allLights[i]->index, pDialog, control);
        allDialogs.push_back(lh);
      }
      filled = true;
    }


    DialogLights::~DialogLights() {
    }

    void DialogLights::closeDialog() {
      for (unsigned int i = 0; i < allDialogs.size(); i++) 
        delete allDialogs[i];
  
      allLights_p.clear();
      allDialogs.clear();
    
      if (pDialog) {
        mainGui->removeDockWidget((void*)pDialog);
        delete pDialog;
        pDialog = NULL;
      }
    }  

    void DialogLights::valueChanged(QtProperty* property, const QVariant& value)
    {
      if (filled == false) 
        return;

      for (unsigned int i = 0; i < allLights_p.size(); i++)
        if (pDialog->currentItem() == allLights_p[i]) {
          allDialogs[i]->valueChanged(property, value);
          return;
        }  

    }

    void DialogLights::topLevelItemChanged(QtProperty *property) {

    }

    void DialogLights::on_add_light() {
      if (control->graphics->getLightCount() >= 8) {
        QMessageBox::information( 0, "Simulation", "Maximum number of lights reached!",
                                  "OK", 0); // ok == buttton 0
        return;
      }

      filled = false;
      QtVariantProperty *tmp = 
        pDialog->addGenericProperty("../NewLight", QtVariantPropertyManager::groupTypeId(), 0);
      allLights_p.push_back(tmp);
      LightHandler *dnn = new LightHandler(tmp, -1, pDialog, control);
      allDialogs.push_back(dnn);
      control->graphics->getLights(&allLights);
      filled = true;
      removeButton->show();
      pDialog->setCurrentItem(tmp);
    }

    void DialogLights::on_remove_light() {
      filled = false;

      for (unsigned int i = 0; i < allLights_p.size(); i++)
        if (allLights_p[i] == pDialog->currentItem()) {
          control->graphics->removeLight(allLights[i]->index);
          allLights.erase(allLights.begin() + i);
          delete allDialogs[i];
          allDialogs.erase(allDialogs.begin() + i);
          allLights_p.erase(allLights_p.begin() + i);
          break;
        }

      filled = true;

      if (allLights_p.size() == 0) 
        removeButton->hide();
      else 
        pDialog->setCurrentItem(allLights_p[0]);
 
    }

  } // end of namespace gui
} // end of namespace mars
