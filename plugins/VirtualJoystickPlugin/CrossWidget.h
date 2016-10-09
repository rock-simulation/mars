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

#ifndef CROSSWIDGET_H
#define CROSSWIDGET_H

#include <QWidget>
#include <QString>
#include <QEvent>
#include <QKeyEvent>
#include <QtDesigner/QDesignerExportWidget>

#include "ui_CrossWidget.h"

class CrossWidget : public QWidget
{
    Q_OBJECT

public:
    CrossWidget(QWidget *parent = 0);
    ~CrossWidget();

public slots:

    void setZeroZoneX(int pixel);
    void setZeroZoneY(int pixel);
    void setZeroZone0(int pixel);

private:
    Ui::CrossWidgetClass ui;

    // area (pixel) around the X, Y axis where the value is set to 0
    int zeroZoneX;
    int zeroZoneY;
    // area (pixel) around the 0-point where the value is set to 0
    int zeroZone0;

    // value of the cursor position in the cross
    int valueX, valueY;
    // value of the cursor position normed to 1 respectively -1
    double doubleX, doubleY;
    // true if the left mouse button is down
    bool mouseLeftButton;
    // the dimension of the cross
    int crossWidth, crossHeight;
    // the displacement of the zero point in the cross
    int crossX, crossY;

    /*
     * calculate from the mouse position the relative position in the cross
     */
    void calcMousePos(QMouseEvent *e);

    /*
     * if the mouse position is in the zeroZone set the value to zero
     */
    void doZeroZone(void);

    /*
     *
     */
    void setDoubleValues(void);

signals:

    void newValue(double doubleX, double doubleY, bool mouseLeftButton);
    void zeroZoneXChanged(int value);
    void zeroZoneYChanged(int value);
    void zeroZone0Changed(int value);

protected:
 virtual void mouseMoveEvent(QMouseEvent *e);
 virtual void mousePressEvent(QMouseEvent *e);
 virtual void mouseReleaseEvent(QMouseEvent *e);

};

#endif // CROSSWIDGET_H
