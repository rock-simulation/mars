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

/**
 * \file ControllerConfigGUI.h
 * \author Malte Römmermann
 * \brief "ControllerConfigGUI" is a template for the widget interface of the MARS GUI
 **/

#ifndef CONTROLLER_CONFIG_GUI_H
#define CONTROLLER_CONFIG_GUI_H

#ifdef _PRINT_HEADER_
#warning "ControllerConfigGUI.h"
#endif

#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/main_gui/BaseWidget.h>
#include <mars/main_gui/PropertyDialog.h>

namespace mars {

  namespace main_gui {
    class GuiInterface;
  }

  namespace gui {
  
    class ControllerConfigGUI : public main_gui::BaseWidget,
                                public main_gui::PropertyCallback {
      Q_OBJECT

      public:
      ControllerConfigGUI(interfaces::ControlCenter* c, main_gui::GuiInterface* gui);
      ~ControllerConfigGUI();

      main_gui::PropertyDialog *pDialog;  

   signals:
      void closeSignal(void* widget);

    public slots:
      void disconnectController(void);
      void connectController(void);
      void setControllerIP(void);
      void setControllerPort(void);
      void toggleControllerAutoMode(bool checked);

    private:
      main_gui::GuiInterface *mainGui;

      struct ControllerElem {
        unsigned long id;
        std::string name;
        int index;
      };

      ControllerElem *controllerElem;
      interfaces::ControlCenter *control;
      unsigned long frame;
      bool take_events, filled;

      QtVariantProperty *controllerIDCombo;
      QtVariantProperty *autoModeCheck, *state;
      QtVariantProperty *ip_edit, *port_edit;

      std::vector<ControllerElem*> controllers;
      void updateGUI(void);
      void closeEvent(QCloseEvent *event);

    protected slots:
      virtual void valueChanged(QtProperty *property, const QVariant &value);
      void timerEvent(QTimerEvent* event);

    };

  } // end of namespace gui
} // end of namespace mars

#endif // CONTROLLER_CONFIG_GUI_H
