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

#ifndef MOTOR_HANDLER_H
#define MOTOR_HANDLER_H

#ifdef _PRINT_HEADER_
#warning "MotorHandler.h"
#endif

#include <mars/main_gui/PropertyDialog.h>
#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/interfaces/core_objects_exchange.h>
#include <mars/interfaces/MotorData.h>

namespace mars {
  namespace gui {

    namespace MotorTree {

      enum Mode {
        PreviewMode,
        EditMode
      };
    }

    ///
    class MotorHandler : public QObject {
      Q_OBJECT
      public:

      MotorHandler(QtVariantProperty *property, unsigned long ind,
                   main_gui::PropertyDialog *pd, interfaces::ControlCenter *c, 
                   MotorTree::Mode m);
      ~MotorHandler();

      void valueChanged(QtProperty *property, const QVariant &value);
      void focusIn();
      void focusOut();
      void accept();

      MotorTree::Mode mode;
  
    private:  
      std::vector<interfaces::core_objects_exchange> joints;
      interfaces::ControlCenter *control;
  
      interfaces::MotorData myMotor;
      bool filled;
  
      QtVariantProperty *joint, *motor, *axis; 
      QtVariantProperty *j_options, *j_type, *type, *name;
      QtVariantProperty *velocity, *force, *p_value, *i_value, *d_value; 

      QtVariantProperty *topLevelMotor;

      std::vector<interfaces::core_objects_exchange> allMotors;
      std::string motorName;
      std::string actualName;
      int myMotorIndex;
      main_gui::PropertyDialog *pDialog;
      QColor previewColor;
      QColor editColor;

  
      void fill();
      void on_change_joint(int index);
      void on_change_type(int index);
      void updateMotor(); 

    };

  } // end of namespace gui
} // end of namespace mars

#endif // MOTOR_HANDLER_H
