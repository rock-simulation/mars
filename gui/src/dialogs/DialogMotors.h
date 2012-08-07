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


#ifndef DIALOGMOTORS_H
#define DIALOGMOTORS_H

#ifdef _PRINT_HEADER_
#warning "DialogMotors.h"
#endif

#include <mars/main_gui/BaseWidget.h>
#include <mars/main_gui/PropertyDialog.h>
#include <mars/interfaces/sim/ControlCenter.h>
#include "MotorHandler.h"

namespace mars {

  namespace main_gui {
    class GuiInterface;
  }

  namespace gui {

    class DialogMotors : public main_gui::BaseWidget,
                         public main_gui::PropertyCallback {
      Q_OBJECT
    
      public:
      DialogMotors(interfaces::ControlCenter *c, main_gui::GuiInterface *gui);
      ~DialogMotors();
  
      main_gui::PropertyDialog *pDialog;  
  
      void show(void) {pDialog->show();}
      void hide(void) {pDialog->hide();}
      bool isHidden(void) {return pDialog->isHidden();}
      void close(void) {pDialog->close();}
    
    private:
      virtual void topLevelItemChanged(QtProperty* current);
      virtual void valueChanged(QtProperty *property, const QVariant &value);

      QtProperty *oldFocus; 
      bool filled;
      QPushButton *addButton;
      QPushButton *removeButton;
      std::vector<MotorHandler*> allDialogs;
      std::vector<MotorHandler*> newDialogs;
      std::vector<QtProperty*> allMotors_p;
      std::vector<QtProperty*> newMotors_p;
      std::vector<interfaces::core_objects_exchange> allMotors;
      interfaces::ControlCenter* control;
      main_gui::GuiInterface *mainGui;

    private slots:
      void on_new_motor();
      void on_remove_motor();
      void on_add_motor();
      void closeDialog();
    };
  
  } // end of namespace gui
} // end of namespace mars

#endif // DIALOGMOTORS_H
