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
 * \file "JoystickWidget.h"
 * \brief Provides an interactive joystick for node movement
 */

#ifndef _JOYSTICK_WIDGET_H
#define _JOYSTICK_WIDGET_H

#include <QtGui>
#include <QLabel>
#include <mars/main_gui/PropertyDialog.h>

#define AX2 0 // 2 axis movement (up-down and left-right)
#define AX4 1 // 4 axis movement (diagonals also)
#define AXI 2 // free movement

namespace mars {
  namespace gui {

    /**
     * \brief Provides an interactive joystick for node movement
     */
    class JoystickWidget : public QWidget {

      Q_OBJECT

      public:

      JoystickWidget(main_gui::PropertyDialog* pd);
      ~JoystickWidget();

      void valueChanged(QtProperty* property, const QVariant& value);
  
    signals:
      void moveSignal(double, double, double); // axis 1, axis 2, strength 

    private:
      bool filled;
      main_gui::PropertyDialog *pDialog;
      QtVariantProperty *type_p, *x_scale_p, *y_scale_p, *str_p;
      QtVariantProperty* axm_p, *showMR;

      void mouseMoveEvent(QMouseEvent* event);
      void mousePressEvent(QMouseEvent* event);
      void mouseReleaseEvent(QMouseEvent* event);
      void timerEvent(QTimerEvent* event);
      void paintEvent(QPaintEvent* event);
      void paintBase(QPainter &painter);
      void paintStick(QPainter &painter);
      void paintCross(QPainter &painter);

      void drawMouseRects(QPainter &painter);
      void calculateCoordinates(void);

      double distance(QPointF A, QPointF B);
      double angle(QPointF A, QPointF B);

      QPointF ballCenter;
      double painterAngle;
      QPointF painterCenter;
      QPointF widgetCenter;
  
      QLinearGradient baseGrad;
      QColor baseColor;
      int axisMovement;

      int type; // 0 for 3d joystick; 1 for cross
      double x_scale, y_scale;
      double height, maxHeight;
      double strength;
      double maxStrength;
      double x_coor, y_coor;
      bool drawMRects;
      bool followMouse;
      bool pressed;
      int mouseRect;
      /* mouseRect
         +---+---+---+
         | 1 | 2 | 3 |
         +---+---+---+
         | 4 | 5 | 6 |
         +---+---+---+
         | 7 | 8 | 9 |
         +---+---+---+
      */

      QLabel *title;
      QLabel *axis1;
      QLabel *axis2;
      QLabel *str_title;
    };

  } // end of namespace gui
} // end of namespace mars

#endif // _JOYSTICK_WIDGET_H
