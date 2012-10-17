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

#include "ConnexionWidget.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QFileDialog>
#include <QComboBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QMutex>

#include <mars/interfaces/graphics/GraphicsManagerInterface.h>
#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/core_objects_exchange.h>

#include <cstdio>

namespace mars {
  namespace plugins {
    namespace connexion_plugin {

      ConnexionWidget::ConnexionWidget(mars::interfaces::ControlCenter* c) {
        control = c;

        //setAttribute(Qt::WA_DeleteOnClose);
  
        win_id = 0;
        object_id = 0;
        take_events = false;

        // first setup font
        standardFont.setFamily(QString::fromUtf8("Lucida Sans"));
        standardFont.setPointSize(9);
        setFont(standardFont);

        resize(10, 10);

        mainLayout = new QGridLayout(this);
        mainLayout->setSpacing(10);
  

        QLabel *label = new QLabel(tr("Control Mode:"), this);
        label->setFont(standardFont);
        objectCombo = new QComboBox(this);
        objectCombo->setObjectName(QString::fromUtf8("objectCombo"));
        objectCombo->setMinimumSize(QSize(140, 0));
        objectCombo->setMaximumSize(QSize(200, 18));
        objectCombo->setFont(standardFont);
        objectCombo->setStyleSheet(QString::fromUtf8("background-color: rgb(217, 217, 217);"));
        objectCombo->setFrame(false);
        objectCombo->addItem(QString("Camera Control"));
        objectCombo->addItem(QString("Object Control"));
        connect(objectCombo, SIGNAL(currentIndexChanged(int)),
                this, SLOT(modeSelected(int)), Qt::DirectConnection);

        mainLayout->addWidget(label, 0, 0);
        mainLayout->addWidget(objectCombo, 0, 1);
  

        label = new QLabel(tr("Select Window:"), this);
        label->setFont(standardFont);
        winIDCombo = new QComboBox(this);
        winIDCombo->setObjectName(QString::fromUtf8("winIDCombo"));
        winIDCombo->setMinimumSize(QSize(140, 0));
        winIDCombo->setMaximumSize(QSize(200, 18));
        winIDCombo->setFont(standardFont);
        winIDCombo->setStyleSheet(QString::fromUtf8("background-color: rgb(217, 217, 217);"));
        winIDCombo->setFrame(false);
        connect(winIDCombo, SIGNAL(currentIndexChanged(int)),
                this, SLOT(windowSelected(int)), Qt::DirectConnection);
        mainLayout->addWidget(label, 1, 0);
        mainLayout->addWidget(winIDCombo, 1, 1);


        label = new QLabel(tr("Select Node:"), this);
        label->setFont(standardFont);
        objectIDCombo = new QComboBox(this);
        objectIDCombo->setObjectName(QString::fromUtf8("objectIDCombo"));
        objectIDCombo->setMinimumSize(QSize(140, 0));
        objectIDCombo->setMaximumSize(QSize(200, 18));
        objectIDCombo->setFont(standardFont);
        objectIDCombo->setStyleSheet(QString::fromUtf8("background-color: rgb(217, 217, 217);"));
        objectIDCombo->setFrame(false);
        connect(objectIDCombo, SIGNAL(currentIndexChanged(int)),
                this, SLOT(objectSelected(int)), Qt::DirectConnection);
        mainLayout->addWidget(label, 2, 0);
        mainLayout->addWidget(objectIDCombo, 2, 1);


        generateDoF(mainLayout, checkLockX, sensitivityX, 
                    SLOT(checkX(bool)), SLOT(sensXChanged(double)), "X", 3);
        generateDoF(mainLayout, checkLockY, sensitivityY, 
                    SLOT(checkY(bool)), SLOT(sensYChanged(double)), "Y", 4);
        generateDoF(mainLayout, checkLockZ, sensitivityZ, 
                    SLOT(checkZ(bool)), SLOT(sensZChanged(double)), "Z", 5);
        generateDoF(mainLayout, checkLockRX, sensitivityRX, 
                    SLOT(checkRX(bool)), SLOT(sensRXChanged(double)), "RX", 6);
        generateDoF(mainLayout, checkLockRY, sensitivityRY, 
                    SLOT(checkRY(bool)), SLOT(sensRYChanged(double)), "RY", 7);
        generateDoF(mainLayout, checkLockRZ, sensitivityRZ, 
                    SLOT(checkRZ(bool)), SLOT(sensRZChanged(double)), "RZ", 8);


        startTimer(500);

        updateGUI();
        take_events = true;
      }

      ConnexionWidget::~ConnexionWidget(void) {
      }


      void ConnexionWidget::hideEvent(QHideEvent* event) {
        (void)event;
        emit hideSignal();
      }

      void ConnexionWidget::closeEvent(QCloseEvent* event) {
        (void)event;
        emit closeSignal();
      }

      void ConnexionWidget::windowSelected(int index) {
        if(take_events) {
          win_id = index;
  
          if(win_id < windowIDs.size())
            emit windowSelected(windowIDs[win_id]);
        }
      }

      void ConnexionWidget::objectSelected(int index) {
        if(take_events) {
          object_id = index;
  
          if(object_id < objectIDs.size())
            emit objectSelected(objectIDs[object_id]);
        }
      }

      void ConnexionWidget::modeSelected(int index) {
        if(take_events) {
          take_events = false;
          updateGUI();
          if(win_id >= windowIDs.size())
            win_id = 0;
          if(object_id >= objectIDs.size())
            object_id = 0;
          winIDCombo->setCurrentIndex(win_id);
          objectIDCombo->setCurrentIndex(object_id);
          if(windowIDs.size() > 0)
            emit windowSelected(windowIDs[win_id]);
          if(objectIDs.size() > 0)
            emit objectSelected(objectIDs[object_id]);
          emit setObjectMode(index+1);
          take_events = true;
        }
      }

      void ConnexionWidget::setWindowID(unsigned long id) {
        unsigned int i;

        for(i=0; i<windowIDs.size(); i++)
          if(windowIDs[i] == id) break;
        if(i<windowIDs.size()) {
          take_events = false;
          winIDCombo->setCurrentIndex(i);
          take_events = true;
          win_id = i;
        }
      }

      void ConnexionWidget::setObjectID(unsigned long id) {
        unsigned int i;

        for(i=0; i<objectIDs.size(); i++)
          if(objectIDs[i] == id) break;
        if(i<objectIDs.size()) {
          take_events = false;
          objectIDCombo->setCurrentIndex(i);
          take_events = true;
          object_id = i;
        }
      }

      void ConnexionWidget::updateGUI(void) {
        char text[255];
        winIDCombo->clear();
        windowIDs.clear();
        if(control->graphics) {
          control->graphics->getList3DWindowIDs(&windowIDs);
          for(unsigned int i=0; i<windowIDs.size(); i++) {
            sprintf(text, "WinID: %lu", windowIDs[i]);
            winIDCombo->addItem(QString(text));
          }
        }

        std::vector<interfaces::core_objects_exchange> objectList;
        std::vector<interfaces::core_objects_exchange>::iterator iter;
        control->nodes->getListNodes(&objectList);
  
        objectIDCombo->clear();
        objectIDs.clear();
        for(iter=objectList.begin(); iter!=objectList.end(); ++iter) {
          sprintf(text, "Node: %s", (*iter).name.data());
          objectIDCombo->addItem(QString(text));        
          objectIDs.push_back((*iter).index);
        }
      }

      void ConnexionWidget::checkX(bool val) {
        emit setLockAxis(1, val);
      }

      void ConnexionWidget::checkY(bool val) {
        emit setLockAxis(2, val);
      }

      void ConnexionWidget::checkZ(bool val) {
        emit setLockAxis(3, val);
      }

      void ConnexionWidget::checkRX(bool val) {
        emit setLockAxis(4, val);
      }

      void ConnexionWidget::checkRY(bool val) {
        emit setLockAxis(5, val);
      }

      void ConnexionWidget::checkRZ(bool val) {
        emit setLockAxis(6, val);
      }

      void ConnexionWidget::sensXChanged(double val) {
        emit sigSensitivity(1, val);
      }

      void ConnexionWidget::sensYChanged(double val) {
        emit sigSensitivity(2, val);
      }

      void ConnexionWidget::sensZChanged(double val) {
        emit sigSensitivity(3, val);
      }

      void ConnexionWidget::sensRXChanged(double val) {
        emit sigSensitivity(4, val);
      }

      void ConnexionWidget::sensRYChanged(double val) {
        emit sigSensitivity(5, val);
      }

      void ConnexionWidget::sensRZChanged(double val) {
        emit sigSensitivity(6, val);
      }


      void ConnexionWidget::setLockAxis(bool lock[6]) {
        checkLockX->setChecked(lock[0]);
        checkLockY->setChecked(lock[1]);
        checkLockZ->setChecked(lock[2]);
        checkLockRX->setChecked(lock[3]);
        checkLockRY->setChecked(lock[4]);
        checkLockRZ->setChecked(lock[5]);
      }

      void ConnexionWidget::setSensitivity(double sens[6]) {
        sensitivityX->setValue(sens[0]);
        sensitivityY->setValue(sens[1]);
        sensitivityZ->setValue(sens[2]);
        sensitivityRX->setValue(sens[3]);
        sensitivityRY->setValue(sens[4]);
        sensitivityRZ->setValue(sens[5]);
      }

      void ConnexionWidget::generateDoF(QGridLayout *mainLayout,
                                        QCheckBox *&checkBox, QDoubleSpinBox *&spinBox,
                                        const char *checkSlot,const char *spinSlot,
                                        const QString &label, int row) {

        checkBox = new QCheckBox(this);
        checkBox->setObjectName(QString("checkBox") + label);
        checkBox->setFont(standardFont);
        checkBox->setText(label);

        spinBox = new QDoubleSpinBox(this);
        spinBox->setObjectName(QString("spinBox") + label);
        spinBox->setFont(standardFont);
        spinBox->setValue(1.);
        spinBox->setStyleSheet(QString::fromUtf8("background-color: rgb(217, 217, 217);"));
        spinBox->setFrame(false);
        spinBox->setSingleStep(0.1);
        spinBox->setRange(-1000., 1000.);

        connect(checkBox, SIGNAL(toggled(bool)),
                this, checkSlot, Qt::DirectConnection);
        connect(spinBox, SIGNAL(valueChanged(double)),
                this, spinSlot, Qt::DirectConnection);
        connect(checkBox, SIGNAL(toggled(bool)),
                spinBox, SLOT(setEnabled(bool)));

        mainLayout->addWidget(checkBox, row, 0);
        mainLayout->addWidget(spinBox, row, 1);
      }

    } // end of namespace connexion_plugin
  } // end of namespace plugins
} // end of namespace mars
