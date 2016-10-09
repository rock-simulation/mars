/*
 *  Copyright 2016, DFKI GmbH Robotics Innovation Center
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

#ifndef JOYSTICKWIDGET_H
#define JOYSTICKWIDGET_H

// Qt includes
#include <QWidget>
#include <QKeyEvent>
#include "ui_JoystickWidget.h"
#include "CrossWidget.h"

class JoystickWidget : public QWidget {

    Q_OBJECT

public:
    JoystickWidget(QWidget *parent = 0);
    ~JoystickWidget();

    void keyPressEvent(QKeyEvent* e);
    void keyReleaseEvent(QKeyEvent* e);

public slots:
    void setRightSpeed(int speed);
    void setLeftSpeed(int speed);
    void setMaxSpeed(double speed);
    void updateCrossValue(double valueX, double valueY, bool leftMouseButton);

signals:
    void newSpeed(double speedLeft, double speedRight);
    void hideSignal(void);
    void closeSignal(void);

private:
    Ui::JoystickWidgetClass ui;

    double crossValueX, crossValueY;

    // speed which is represented by the mouse position
    double newSpeedLeft, newSpeedRight;
    // the speed which is actually set
    double actualSpeedLeft, actualSpeedRight;

    // max speed emitted by the signal
    double maxSpeed;

    int zeroZoneSlider;

    /*
     * calculate the new speed
     */
    void calcSpeed(void);

    /*
     *
     */
    void setSpeed(void);

    /*
     * makes an update of the new and set speed in the WirthControl
     */
    void updateWidget(void);

 protected slots:
  void hideEvent(QHideEvent* event);
  void closeEvent(QCloseEvent* event);

};

#endif // JOYSTICKWIDGET_H
