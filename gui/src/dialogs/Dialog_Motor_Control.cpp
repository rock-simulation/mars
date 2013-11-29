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

#include <QVBoxLayout>
#include <QScrollArea>

using namespace std;

namespace mars {
  namespace gui {

    Dialog_Motor_Control::Dialog_Motor_Control(interfaces::ControlCenter* c)
      : main_gui::BaseWidget(0, c->cfg, "Dialog_Motor_Control"),
        pDialog(new main_gui::PropertyDialog(this)) {

      filled = false;
      control = c;
      this->setWindowTitle(tr("Motor Control"));

      pDialog->setPropCallback(this);
      pDialog->hideAllButtons();

      control->motors->getListMotors(&myMotors);
      vLayout = new QVBoxLayout;  
      vLayout->addSpacing(21);

      for (uint i = 0;i<myMotors.size();i++) 
        addMotor(&(myMotors[i]));

      pDialog->setFixedWidth(pDialog->width());

      vLayout->setAlignment(Qt::AlignTop);
      QHBoxLayout *hLayout = new QHBoxLayout;
      hLayout->addWidget(pDialog);
      hLayout->addLayout(vLayout);
      QFrame *frame = new QFrame(this);
      frame->setLayout(hLayout);
      QScrollArea *scrollArea = new QScrollArea(0);
      scrollArea->setWidget(frame);
      scrollArea->setWidgetResizable(true);
      QHBoxLayout *thisLayout = new QHBoxLayout;
      thisLayout->addWidget(scrollArea);
      this->setLayout(thisLayout);

      filled = true;
    }

    Dialog_Motor_Control::~Dialog_Motor_Control() {
      delete pDialog;
    }

    void Dialog_Motor_Control::addMotor(interfaces::core_objects_exchange *motor) {

      QtVariantProperty *aMotor;
      QSlider *slider = new QSlider(Qt::Horizontal);
      std::map<QString, QVariant> attr;
      attr.insert(pair<QString, QVariant>(QString::fromStdString("decimals"), 16));
      attr.insert(pair<QString, QVariant>(QString::fromStdString("singleStep"), 0.1));
      attr.insert(pair<QString, QVariant>(QString::fromStdString("minimum"), -10.0));
      attr.insert(pair<QString, QVariant>(QString::fromStdString("maximum"),  10.0));

      aMotor = pDialog->addGenericProperty("../"+QString::number(motor->index).toStdString()+":"+motor->name, 
                                           QVariant::Double, motor->value, &attr);
      motorWidgets.push_back(aMotor);
      slider->setRange(-100000, 100000);
      slider->setValue((int)(motor->value*10000));
      vLayout->addWidget(slider);
      vLayout->addSpacing(6);
      sliders.push_back(slider);
 
      connect(slider, SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged(int)));
    }

    void Dialog_Motor_Control::sliderValueChanged(int value) {
      unsigned int i;
      for (i = 0; i < sliders.size(); i++)
        if (value == sliders[i]->value()) {
          myMotors[i].value = (double)value/10000.0;
          break;
        }
      motorWidgets[i]->setValue(myMotors[i].value);
    }

    void Dialog_Motor_Control::valueChanged(QtProperty *property, const QVariant &value) 
    {
      if (filled == false) return;
      filled = false;
      unsigned int i;
  
      for (i = 0; i < motorWidgets.size(); i++)
        if (property == motorWidgets[i]) {
          myMotors[i].value = value.toDouble();
          sliders[i]->setValue(static_cast<int>(value.toDouble()*10000));
          break;
        }
  
      control->motors->moveMotor(myMotors[i].index, myMotors[i].value);
      filled = true;
    }

    void Dialog_Motor_Control::closeEvent(QCloseEvent *event) {
      (void)event;
      emit closeSignal(this);
    }

  } // end of namespace gui
} // end of namespace mars

