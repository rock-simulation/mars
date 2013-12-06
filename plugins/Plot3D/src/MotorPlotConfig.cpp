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

#include "MotorPlotConfig.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QMutex>

#include <mars/interfaces/graphics/GraphicsManagerInterface.h>
#include <mars/interfaces/sim/MotorManagerInterface.h>
#include <mars/interfaces/core_objects_exchange.h>

#include <cstdio>

namespace mars {
  namespace plugins {
    namespace Plot3D {

      MotorPlotConfig::MotorPlotConfig(mars::interfaces::ControlCenter* c) {
        control = c;

        //setAttribute(Qt::WA_DeleteOnClose);
  
        take_events = false;

        // first setup font
        standardFont.setFamily(QString::fromUtf8("Lucida Sans"));
        standardFont.setPointSize(9);
        setFont(standardFont);

        resize(10, 10);

        mainLayout = new QGridLayout(this);
        mainLayout->setSpacing(10);
  

        QLabel *label = new QLabel(tr("Control Mode:"), this);

        label = new QLabel(tr("Select Motor:"), this);
        label->setFont(standardFont);
        motorIDCombo = new QComboBox(this);
        motorIDCombo->setObjectName(QString::fromUtf8("motorIDCombo"));
        motorIDCombo->setMinimumSize(QSize(140, 0));
        motorIDCombo->setMaximumSize(QSize(200, 18));
        motorIDCombo->setFont(standardFont);
        motorIDCombo->setStyleSheet(QString::fromUtf8("background-color: rgb(217, 217, 217);"));
        motorIDCombo->setFrame(false);
        connect(motorIDCombo, SIGNAL(currentIndexChanged(int)),
                this, SLOT(motorSelected(int)), Qt::DirectConnection);
        mainLayout->addWidget(label, 0, 0);
        mainLayout->addWidget(motorIDCombo, 0, 1);

        QPushButton *button = new QPushButton("addPlot", this);
        connect(button, SIGNAL(clicked(bool)),
                this, SLOT(addPlot(bool)), Qt::DirectConnection);
        mainLayout->addWidget(button, 1, 1);

        button = new QPushButton("removePlot", this);
        connect(button, SIGNAL(clicked(bool)),
                this, SLOT(removePlot(bool)), Qt::DirectConnection);
        mainLayout->addWidget(button, 2, 1);

        startTimer(500);

        updateGUI();
        take_events = true;
      }

      MotorPlotConfig::~MotorPlotConfig(void) {
      }


      void MotorPlotConfig::hideEvent(QHideEvent* event) {
        (void)event;
        emit hideSignal();
      }

      void MotorPlotConfig::closeEvent(QCloseEvent* event) {
        (void)event;
        emit closeSignal();
      }

      void MotorPlotConfig::motorSelected(int index) {
        if(take_events) {
          if(index < motorIDs.size())
            emit motorSelected(motorIDs[index]);
        }
      }

      void MotorPlotConfig::addPlot(bool b) {
        emit addPlot();
      }

      void MotorPlotConfig::removePlot(bool b) {
        emit removePlot();
      }

      void MotorPlotConfig::updateGUI(void) {
        char text[255];
        motorIDCombo->clear();

        std::vector<interfaces::core_objects_exchange> objectList;
        std::vector<interfaces::core_objects_exchange>::iterator iter;
        control->motors->getListMotors(&objectList);
  
        motorIDs.clear();
        for(iter=objectList.begin(); iter!=objectList.end(); ++iter) {
          sprintf(text, "Motor: %s", (*iter).name.data());
          motorIDCombo->addItem(QString(text));        
          motorIDs.push_back((*iter).index);
        }
      }

    } // end of namespace connexion_plugin
  } // end of namespace plugins
} // end of namespace mars
