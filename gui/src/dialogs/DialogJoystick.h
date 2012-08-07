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
 * \file DialogJoystick.h
 * \author Vladimir Komsiyski
 */

#ifndef DIALOG_JOYSTICK_H
#define DIALOG_JOYSTICK_H

#ifdef _PRINT_HEADER_
#warning "DialogJoystick.h"
#endif

#include "JoystickWidget.h"
#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/main_gui/BaseWidget.h>
#include <mars/main_gui/PropertyDialog.h>
#include "NodeSelectionTree.h"

namespace mars {
  namespace gui {

    /**
     * \brief DialogJoystick is a widget that allows node movement functionality
     * with different selection and movement options
     */
    class DialogJoystick : public main_gui::BaseWidget,
                           public main_gui::PropertyCallback {

      Q_OBJECT

      public:
      /**\brief creates the dialog */
      DialogJoystick(interfaces::ControlCenter *c);
      ~DialogJoystick();

      main_gui::PropertyDialog *pDialog;

      void linkSelectionDialog(NodeSelectionTree* nst) {
        this->nodeList = nst;
      }

    private:
      void closeEvent(QCloseEvent *event);

      std::vector<interfaces::core_objects_exchange> simNodes;
      NodeSelectionTree *nodeList;
      JoystickWidget *joy;
      interfaces::ControlCenter* control;
      bool filled;
      QtVariantProperty *move_all, *plane;

    signals:
      void closeSignal(void* widget);

    private slots:
      virtual void valueChanged(QtProperty *property, const QVariant &value);
      void moveSlot(double x, double y, double strength);

    };

  } // end of namespace gui
} // end of namespace mars

#endif
