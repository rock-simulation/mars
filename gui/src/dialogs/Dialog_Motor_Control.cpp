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


#include "Dialog_Motor_Control.h"
#include <math.h>
#include <iostream>

#include <mars/interfaces/sim/MotorManagerInterface.h>
#include <mars/interfaces/MotorData.h>
#include <mars/interfaces/MARSDefs.h>

#include <QVBoxLayout>
#include <QScrollArea>
#include <QPushButton>

using namespace std;

namespace mars {
  namespace gui {

    Dialog_Motor_Control::Dialog_Motor_Control(interfaces::ControlCenter* c)
      : main_gui::BaseWidget(0, c->cfg, "Dialog_Motor_Control") {

      slideractive = false;
      degboxactive = false;
      radboxactive = false;
      scale = 10000.0;
      control = c;
      this->setWindowTitle(tr("Motor Control"));
      grLayout = new QGridLayout();
      mainLayout = new QVBoxLayout();
      this->setLayout(mainLayout);

      // add button to set all values to 0
      QPushButton *zerobutton = new QPushButton("Set all motors to 0", this);
      connect(zerobutton, SIGNAL(clicked()), this, SLOT(zerobuttonclicked()));
      mainLayout->addWidget(zerobutton);


      control->motors->getListMotors(&motors);
      for (uint i = 1; i<motors.size(); ++i) {
        interfaces::MotorData motordata = control->motors->getFullMotor(motors[i].index);

        // add label
        QLabel *label = new QLabel(QString::fromStdString(motors[i].name));
        grLayout->addWidget(label, i, 1);

        //add radiants spinbox
        QDoubleSpinBox *radspinbox = new QDoubleSpinBox();
        radspinbox->setDecimals(4);
        radspinbox->setRange(motordata.minValue*scale, motordata.maxValue*scale);
        radspinbox->setValue((int)(motors[i].value*scale));
        //radspinbox->setSingleStep((motordata.maxValue-motordata.minValue)/100.0);
        radspinbox->setSingleStep(0.008726646259971648);
        grLayout->addWidget(radspinbox, i, 2);
        radspinboxes.push_back(radspinbox);
        QLabel *radlabel;
        if (motordata.type == interfaces::MotorType::MOTOR_TYPE_POSITION) {
          radlabel = new QLabel("rad     ");
        }
        else {
          radlabel = new QLabel("rad/s   ");
        }
        grLayout->addWidget(radlabel, i, 3);

        //add degrees spinbox
        QDoubleSpinBox *degspinbox = new QDoubleSpinBox();
        degspinbox->setDecimals(3);
        degspinbox->setRange(motordata.minValue*scale, motordata.maxValue*scale);
        degspinbox->setValue((int)(motors[i].value*scale));
        degspinbox->setSingleStep(0.5);
        grLayout->addWidget(degspinbox, i, 4);
        degspinboxes.push_back(degspinbox);
        QLabel *deglabel;
        if (motordata.type == interfaces::MotorType::MOTOR_TYPE_POSITION) {
          deglabel = new QLabel("deg     ");
        }
        else {
          deglabel = new QLabel("deg/s   ");
        }
        grLayout->addWidget(deglabel, i, 5);

        // add slider
        QSlider *slider = new QSlider(Qt::Horizontal);
        slider->setRange(motordata.minValue*scale, motordata.maxValue*scale);
        slider->setValue((int)(motors[i].value*scale));
        slider->setTickPosition(QSlider::TicksBothSides);
        slider->setTickInterval(0.7853981633974483*scale);
        grLayout->addWidget(slider, i, 6);
        sliders.push_back(slider);

        connect(slider, SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged(int)), Qt::DirectConnection);
        connect(radspinbox, SIGNAL(valueChanged(double)), this, SLOT(radspinboxValueChanged(double)), Qt::DirectConnection);
        connect(degspinbox, SIGNAL(valueChanged(double)), this, SLOT(degspinboxValueChanged(double)), Qt::DirectConnection);
      }
      grLayout->setColumnMinimumWidth(6, 100);

      // Create window
      //QFrame *frame = new QFrame(this);
      QFrame *frame = new QFrame();
      frame->setMinimumWidth(600);
      frame->setLayout(grLayout);

      // Add scrolling functionality
      QScrollArea *scrollArea = new QScrollArea;
      scrollArea->setWidgetResizable(true);
      scrollArea->setWidget(frame);

      mainLayout->addWidget(scrollArea);
    }

    Dialog_Motor_Control::~Dialog_Motor_Control() {
      delete grLayout;
      delete mainLayout;
      //TODO: more to delete?
    }

    void Dialog_Motor_Control::sliderValueChanged(int value) {
      if (not radboxactive and not degboxactive) {
        slideractive = true;
        QObject* obj = sender();
        for (uint i = 0; i < sliders.size(); ++i)
          if (obj == sliders[i]) {
            motors[i].value = (double)value/scale;
            radspinboxes[i]->setValue(motors[i].value);
            degspinboxes[i]->setValue(motors[i].value/3.141593*180);
            control->motors->moveMotor(motors[i].index, motors[i].value);
            break;
          }
        slideractive = false;
      }

    }

    void Dialog_Motor_Control::radspinboxValueChanged(double value) {
      if (not slideractive and not degboxactive) {
        radboxactive = true;
        QObject* obj = sender();
        for (uint i = 0; i < radspinboxes.size(); ++i)
          if (obj == radspinboxes[i]) {
            motors[i].value = (double)value;
            sliders[i]->setValue(motors[i].value*scale);
            degspinboxes[i]->setValue(motors[i].value/3.141593*180);
            control->motors->moveMotor(motors[i].index, motors[i].value);
            break;
          }
        radboxactive = false;
      }
    }

    void Dialog_Motor_Control::degspinboxValueChanged(double value) {
      if (not radboxactive and not slideractive) {
        degboxactive = true;
        QObject* obj = sender();
        for (uint i = 0; i < degspinboxes.size(); ++i)
          if (obj == degspinboxes[i]) {
            motors[i].value = (double)value/180.0*3.141593;
            radspinboxes[i]->setValue(motors[i].value);
            sliders[i]->setValue(motors[i].value*scale);
            control->motors->moveMotor(motors[i].index, motors[i].value);
            break;
          }
        degboxactive = false;
      }
    }

    void Dialog_Motor_Control::zerobuttonclicked() {
      slideractive = true;
      for (uint i = 1; i<sliders.size(); ++i) {
        sliders[i]->setValue(0);
      }
      slideractive = false;
    }

    void Dialog_Motor_Control::closeEvent(QCloseEvent *event) {
      (void)event;
      emit closeSignal(this);
    }

  } // end of namespace gui
} // end of namespace mars
