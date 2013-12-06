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

#include "JoystickWidget.h"
#include <cmath>

using namespace std;

#define JOYSTICK_PI 3.14159265

namespace mars {
  namespace gui {

    JoystickWidget::JoystickWidget(main_gui::PropertyDialog *pd) : QWidget(0) {
      filled = false;
      pDialog = pd;

      map<QString, QVariant> attr;
      attr.insert(pair<QString, QVariant>(QString("decimals"), 9));
      attr.insert(pair<QString, QVariant>(QString("singleStep"), 0.1));

      x_scale_p = pDialog->addGenericProperty("../Scale Axis 1", 
                                              QVariant::Double, 0.1, &attr);
      y_scale_p = pDialog->addGenericProperty("../Scale Axis 2",
                                              QVariant::Double, 0.1, &attr);
      str_p = pDialog->addGenericProperty("../Maximum Strength",
					  QVariant::Double, 1, &attr);
      x_scale = y_scale = 0.1;
      x_coor = y_coor = 0.0;
      maxStrength = 1;

      QStringList enumNames;
      enumNames << "3D Joystick" << "Cross";
      type_p = pDialog->addGenericProperty("../Interaction", 
                                           QtVariantPropertyManager::enumTypeId(), 
                                           0, NULL, &enumNames);
  
      enumNames.clear();
      enumNames << "2 axis" << "4 axis" << "Free Movement";
      axm_p = pDialog->addGenericProperty("../Movement", 
                                          QtVariantPropertyManager::enumTypeId(),
                                          2, NULL, &enumNames);
  
      showMR = pDialog->addGenericProperty("../Show Zones", 
                                           QVariant::Bool, true);

      title = new QLabel(tr("Joystick"), this);
      str_title = new QLabel("Strength: ", this);
      str_title->setGeometry(0, 13, str_title->width(), str_title->height());
      axis1 = new QLabel("Axis 1: ", this);
      axis1->setGeometry(150, 0, axis1->width(), axis1->height());
      axis2 = new QLabel("Axis 2: ", this);
      axis2->setGeometry(150, 13, axis1->width(), axis1->height());
      setFixedSize(250, 250);
      setMouseTracking(true);
      followMouse = false;
      pressed = false;
      setCursor(Qt::CrossCursor);
      axisMovement = AXI;
      type = 0;
      strength = 0;
      drawMRects = true;
      widgetCenter = rect().center() + QPointF(0,25);
      painterCenter = widgetCenter - QPointF(0,10);
      painterAngle = 0;
      ballCenter = painterCenter;
      baseGrad = QLinearGradient(widgetCenter-QPoint(38,0), widgetCenter+QPoint(38,0));
      baseGrad.setColorAt(0, QColor(160, 160, 160));
      baseGrad.setColorAt(1, QColor(20, 20, 20));
      baseGrad.setSpread(QGradient::ReflectSpread);
      baseColor = QColor(80, 80, 80);
      mouseRect = 5;
      update();
      filled = true;
      startTimer(100);
    }

    JoystickWidget::~JoystickWidget()
    {
    }


    void JoystickWidget::valueChanged(QtProperty* property, const QVariant& value) {
      if (filled == false)
        return;

      else if (property == type_p) {
        type = value.toInt();
        if (value.toInt() != 0) {
          axisMovement = AXI;
          axm_p->setValue(AXI);
        }
        axm_p->setEnabled(!value.toBool());
        showMR->setEnabled(!value.toBool());
      }
    
      else if (property == x_scale_p) {
        x_scale = value.toDouble();
    
      }

      else if (property == y_scale_p) {
        y_scale = value.toDouble();

      }
  
      else if (property == str_p) {
        maxStrength = value.toDouble();

      }

      else if (property == axm_p) {
        axisMovement = value.toInt();
      }

      else if (property == showMR) {
        drawMRects = value.toBool();
      }

      update();
    }


    void JoystickWidget::paintEvent(QPaintEvent* event)
    {
      (void)event;
      QPainter painter(this);
      painter.drawRect(0, 0, 250, 250);
  
      calculateCoordinates();
      if (type == 1) {
        paintCross(painter);
      }  else {
        if (drawMRects)
          drawMouseRects(painter);
        paintBase(painter);
        paintStick(painter);
      }
      str_title->setText("Strength: " + QString().setNum(strength*maxStrength));
      axis1->setText("Axis1: " + QString().setNum(x_coor*x_scale));
      axis2->setText("Axis2: " + QString().setNum(y_coor*y_scale));  
    }


    void JoystickWidget::calculateCoordinates(void) {
      x_coor = (double)(ballCenter.x()-125.0)/125.0;
      y_coor = (double)(125.0-ballCenter.y())/125.0;

      height = 0; 
      maxHeight = 1;
      if (ballCenter.y() < painterCenter.y()-90)
        maxHeight = 100;
      else if (ballCenter.y() > painterCenter.y()+70)
        maxHeight = 80;
      else
        maxHeight = 88 - (ballCenter.y()-painterCenter.y())/9;

      height = distance(painterCenter, ballCenter);
      if (height > maxHeight)
        height = maxHeight;
      strength = height/maxHeight;
    }

    void JoystickWidget::paintCross(QPainter &painter)
    {
      painter.drawLine(0, 125, 250, 125);
      painter.drawLine(125, 0, 125, 250);
    }



    void JoystickWidget::paintBase(QPainter& painter)
    {
      painter.setPen(Qt::transparent);
      painter.setBrush(baseGrad);
      painter.drawEllipse(widgetCenter, 45, 45);
      painter.setBrush(QColor(110, 110, 110));
      painter.drawEllipse(widgetCenter-QPoint(0, 5), 45, 45);
      painter.setBrush(baseColor);
      painter.drawEllipse(widgetCenter-QPoint(0, 5), 42, 42);
      painter.setBrush(QColor(50, 50, 50));
      painter.drawEllipse(widgetCenter-QPoint(0, 10), 16, 16);
    }

    void JoystickWidget::paintStick(QPainter& painter)
    {
 
      switch (axisMovement) {
      case AXI:
        painterAngle = angle(painterCenter, ballCenter);
        break;
    
      case AX2:
        height = maxHeight;
        strength = height/maxHeight;
        switch(mouseRect) {
        case 2: 
          painterAngle = 0; 
          x_coor = 0; y_coor = 1;
          break;
        case 4: 
          painterAngle = -80; 
          x_coor = -1; y_coor = 0;
          break;
        case 6: 
          painterAngle = 80; 
          x_coor = 1; y_coor = 0;
          break;
        case 8: 
          painterAngle = 180; 
          x_coor = 0; y_coor = -1;
          break;
        default: 
          painterAngle = 0; 
          height = 50; strength = 0;
          x_coor = 0; y_coor = 0;
          break;
        }
        break;

      case AX4:
        height = maxHeight;
        strength = height/maxHeight;
        switch(mouseRect) {
        case 1: painterAngle = -40; 
          x_coor = -1; y_coor = 1;      
          break; 
        case 3: painterAngle = 40; 
          x_coor = 1; y_coor = 1;   
          break;
        case 7: painterAngle = -120; 
          x_coor = -1; y_coor = -1;   
          break;
        case 9: painterAngle = 120;
          x_coor = 1; y_coor = -1;   
          break;
        case 2: painterAngle = 0;
          x_coor = 0; y_coor = 1;   
          break;
        case 4: painterAngle = -80;
          x_coor = -1; y_coor = 0;   
          break;
        case 6: painterAngle = 80; 
          x_coor = 1; y_coor = 0;   
          break;
        case 8: painterAngle = 180;
          x_coor = 0; y_coor = -1;   
          break;
        default: painterAngle = 0; 
          height = 50; strength = 0;
          x_coor = 0; y_coor = 0;   
          break;
        }
      }

      if (followMouse == false && pressed == false) {
        painterAngle = 0; 
        height = 50;
      } 
    
      painter.translate(painterCenter);
      painter.rotate(painterAngle);  

      QRect cylinder(-10, 10, 20, -height);//cylinderHeight());
      QLinearGradient grad(cylinder.topLeft()+QPoint(5,0), cylinder.topRight()-QPoint(5,0));
      grad.setColorAt(0, QColor(250, 250, 250));
      grad.setColorAt(1, QColor(80, 80, 80));
      grad.setSpread(QGradient::ReflectSpread);
      painter.setBrush(grad);
      painter.drawRoundedRect(cylinder, 9, 9);

      double radius = 80-height/5;
      QRect ball(cylinder.center()+QPoint(0, cylinder.height()/2)-QPoint(radius/2, radius/2),
                 QSize(radius, radius));
      QRadialGradient rad(ball.center(), radius, ball.center()-QPoint(10, 10));
      rad.setColorAt(0, QColor(255, 200, 200));
      rad.setColorAt(1, QColor(255, 63, 63));
      painter.setBrush(rad);
      painter.drawEllipse(ball);

    }


    double JoystickWidget::distance(QPointF A, QPointF B)
    {
      return sqrt(pow(A.x()-B.x(), 2) + pow(A.y()-B.y(), 2));
    }

    double JoystickWidget::angle(QPointF A, QPointF B)
    {
      return atan2(B.x()-A.x(), A.y()-B.y()) * 180 / JOYSTICK_PI;
    }




    void JoystickWidget::drawMouseRects(QPainter &painter)
    {
      painter.setPen(Qt::DashLine);
      painter.drawLine(250/3, 0, 250/3, 250);
      painter.drawLine(500/3, 0, 500/3, 250);
      painter.drawLine(0, 250/3, 250, 250/3);
      painter.drawLine(0, 500/3, 250, 500/3);
    }




    void JoystickWidget::mouseMoveEvent(QMouseEvent* event)
    {
#if USE_QT5
      ballCenter = event->localPos();
#else
      ballCenter = event->posF();
#endif
      mouseRect = (int)(ballCenter.x())*3/250+1;
      mouseRect += (int)(ballCenter.y())*3/250*3;
      update();
    }  



    void JoystickWidget::mousePressEvent(QMouseEvent *event)
    {
      switch (event->button()) {
      case Qt::RightButton:
        followMouse = !followMouse;
        pressed = !pressed;
    
        update();
        if (x_coor == 0 && y_coor == 0) return;
        switch (mouseRect) {
        case 5: if (axisMovement != AXI) break; 
        case 1:
        case 3:
        case 7:
        case 9: if (axisMovement == AX2) break;
        default: 
          emit moveSignal(x_coor*x_scale, y_coor*y_scale, strength*maxStrength);
        }
        break;
    
      case Qt::LeftButton:
        followMouse = true;
        update();
        break;
  
      default:
        break;
      }
  
    }


    void JoystickWidget::mouseReleaseEvent(QMouseEvent* event) {
      if (pressed)
        return;
      if (event->button() == Qt::LeftButton) {
        followMouse = false;
        update();
        if (x_coor == 0 && y_coor == 0) return;
        switch (mouseRect) {
        case 5: if (axisMovement != AXI) break; 
        case 1:
        case 3:
        case 7:
        case 9: if (axisMovement == AX2) break;
        default: 
          emit moveSignal(x_coor*x_scale, y_coor*y_scale, strength*maxStrength);
        }
      }
    }

    void JoystickWidget::timerEvent(QTimerEvent *event) 
    {
      if (pressed == false)
        return;
      else {
        if (x_coor == 0 && y_coor == 0) return;
        switch (mouseRect) {
        case 5: if (axisMovement != AXI) break;
        case 1:
        case 3:
        case 7:
        case 9: if (axisMovement == AX2) break;
        default: 
          emit moveSignal(x_coor*x_scale, y_coor*y_scale, strength*maxStrength);
        }
      }
    }

  } // end of namespace gui
} // end of namespace mars
