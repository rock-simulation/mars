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

#ifndef DIALOG_MOTOR_CONTROL_H
#define DIALOG_MOTOR_CONTROL_H

#ifdef _PRINT_HEADER_
#warning "Dialog_Motor_Control.h"
#endif

#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/interfaces/core_objects_exchange.h>
#include <mars/main_gui/BaseWidget.h>

#include <QSlider>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>

namespace mars {
  namespace gui {

    /**
     * \brief Dialog_Motor_Control is a widget that creates MotorSet widgets for each
     * available motor to change its values
     */
    class Dialog_Motor_Control : public main_gui::BaseWidget {

      Q_OBJECT

      public:
      /**\brief creates the dialog */
      Dialog_Motor_Control(interfaces::ControlCenter *c);
      ~Dialog_Motor_Control();

    private:
      std::vector<interfaces::core_objects_exchange> motors;
      std::vector<QDoubleSpinBox*> radspinboxes;
      std::vector<QDoubleSpinBox*> degspinboxes;
      std::vector<QSlider*> sliders;
      double scale;

      bool slideractive;
      bool radboxactive;
      bool degboxactive;

      QGridLayout* grLayout;
      QVBoxLayout* mainLayout;
      interfaces::ControlCenter* control;

      void closeEvent(QCloseEvent *event);

   signals:
      void closeSignal(void*);

    private slots:
      void zerobuttonclicked();
      void sliderValueChanged(int);
      void radspinboxValueChanged(double);
      void degspinboxValueChanged(double);
    };

  } // end of namespace gui
} // end of namespace mars

#endif
