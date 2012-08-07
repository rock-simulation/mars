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

#include <QtGui>

#include "MarsStyle.h"
#include <string>
#include <iostream>

namespace mars {
  namespace gui {

    void MarsStyle::polish(QPalette &palette)
    {
      QColor brown(155, 155, 155);
      QColor reddish(200, 200, 200, 128);
      QColor slightlyOpaqueBlack(100, 100, 100, 63);
      QLinearGradient grad(0, 0, 0, 20);
      grad.setColorAt(0.1, QColor(241, 161, 110));
      grad.setColorAt(0.9, QColor(188, 125, 86));
      grad.setSpread(QGradient::RepeatSpread);

      std::string tmp = stylePath;
      tmp.append("/marsbackground.jpeg");
      QPixmap backgroundImage(QString::fromStdString(tmp));

      tmp = stylePath;
      tmp.append("/marsbutton.jpeg");
      QPixmap buttonImage(QString::fromStdString(tmp));
      QPixmap midImage = buttonImage;
     
      /*     QPainter painter;
             painter.begin(&midImage);
             painter.setPen(Qt::NoPen);
             painter.fillRect(midImage.rect(), slightlyOpaqueBlack);
             painter.end();
      */
      palette = QPalette(brown);

      palette.setBrush(QPalette::BrightText, QColor(205,175,149));
      palette.setBrush(QPalette::Base, reddish);
      palette.setBrush(QPalette::Highlight, QColor(241,161,110));
      setTexture(palette, QPalette::Button, buttonImage);
      setTexture(palette, QPalette::Mid, midImage);
      setTexture(palette, QPalette::Window, backgroundImage);

      QBrush brush = palette.background();
      brush.setColor(brush.color().dark());

      palette.setBrush(QPalette::Disabled, QPalette::WindowText, brush);
      palette.setBrush(QPalette::Disabled, QPalette::Text, brush);
      palette.setBrush(QPalette::Disabled, QPalette::ButtonText, brush);
      palette.setBrush(QPalette::Disabled, QPalette::Base, brush);
      palette.setBrush(QPalette::Disabled, QPalette::Button, brush);
      palette.setBrush(QPalette::Disabled, QPalette::Mid, brush);
    }

    void MarsStyle::polish(QWidget *widget)
    {
      if (qobject_cast<QPushButton *>(widget)
          || qobject_cast<QComboBox *>(widget))
        widget->setAttribute(Qt::WA_Hover, true);
    }

    void MarsStyle::unpolish(QWidget *widget)
    {
      if (qobject_cast<QPushButton *>(widget)
          || qobject_cast<QComboBox *>(widget))
        widget->setAttribute(Qt::WA_Hover, false);
    }

    int MarsStyle::pixelMetric(PixelMetric metric,
                               const QStyleOption *option,
                               const QWidget *widget) const
    {
      switch (metric) {
        /*     case PM_ComboBoxFrameWidth:
               return 8;
               case PM_ScrollBarExtent:
               return QPlastiqueStyle::pixelMetric(metric, option, widget) + 4;
        */  default:
          return QPlastiqueStyle::pixelMetric(metric, option, widget);
      }
    }

    int MarsStyle::styleHint(StyleHint hint, const QStyleOption *option,
                             const QWidget *widget,
                             QStyleHintReturn *returnData) const
    {
      switch (hint) {
        /*     case SH_DitherDisabledText:
               return int(false);
               case SH_EtchDisabledText:
               return int(true);
        */default:
          return QPlastiqueStyle::styleHint(hint, option, widget, returnData);
      }
    }

    void MarsStyle::drawPrimitive(PrimitiveElement element,
                                  const QStyleOption *option,
                                  QPainter *painter,
                                  const QWidget *widget) const
    {
      const static QPen pen(QColor(69, 30, 05, 100), 2);
      QLinearGradient grad(0, 0, 0, 20);
      grad.setColorAt(0.1, QColor(241, 161, 110));
      grad.setColorAt(0.9, QColor(188, 125, 86));
      grad.setSpread(QGradient::RepeatSpread);
   
      // ((QPalette)(option->palette)).setBrush(QPalette::Highlight, grad);
   
      switch (element) {
      case PE_IndicatorBranch:
        if (option->state & QStyle::State_Open)
          {
            QString tmp = QString::fromStdString(stylePath + "/branch-open.png");
            painter->drawImage((option->rect.topLeft()+option->rect.center())/2, QImage(tmp));
            break;
          }
        if (option->state & QStyle::State_Children)
          {
            QString tmp = QString::fromStdString(stylePath +  "/branch-closed.png");
            painter->drawImage((option->rect.topLeft()+option->rect.center())/2, QImage(tmp));
            break;
          }
        if (option->state & QStyle::State_Item) {
          painter->save();
          QPoint stop = (option->rect.bottomLeft()+option->rect.bottomRight())/2 - 
            QPoint(0, option->rect.height()/2);
          painter->setPen(pen);
          painter->drawLine((option->rect.topRight()+option->rect.bottomRight())/2,
                            (option->rect.topRight()+option->rect.bottomRight())/2 - QPoint(option->rect.width()/2 - 2, 0));
          if (option->state & QStyle::State_Sibling) 
            stop += QPoint(0, option->rect.height()/2);
          painter->drawLine((option->rect.topLeft()+option->rect.topRight())/2, stop);
          painter->restore();
          break;
        }
        if (option->state & QStyle::State_Sibling) {
          painter->save();
          painter->setPen(pen);
          painter->drawLine((option->rect.topLeft()+option->rect.topRight())/2, 
                            (option->rect.bottomLeft()+option->rect.bottomRight())/2);	   
          painter->restore();
          break;
        }    
       
        /*
          if (option->state & QStyle::State_Item)
          {
          QImage tmp;
          if (option->state & QStyle::State_Sibling) { 
          tmp = QImage(QString::fromStdString(Pathes::getGuiPath() +  "styles/branch-more.png")).
          scaled(26, 20);
          painter->drawImage(option->rect.topLeft()+QPoint(-4,0), tmp);
          } else { 
          tmp = QImage(QString::fromStdString(Pathes::getGuiPath() +  "styles/branch-end.png")).
          scaled(26, 23);	     
          painter->drawImage(option->rect.topLeft()+QPoint(-4,0), tmp);
          }
          break;
          }
          if (option->state & QStyle::State_Sibling)
          {
          QString tmp = QString::fromStdString(Pathes::getGuiPath() +  "styles/vline.png");
          painter->drawImage(option->rect.topLeft()+QPoint(-2,0), QImage(tmp).scaled(26,20));
          break;
          }*/

      default:
        QPlastiqueStyle::drawPrimitive(element, option, painter, widget);
      }
    }

    void MarsStyle::drawControl(ControlElement element,
                                const QStyleOption *option,
                                QPainter *painter,
                                const QWidget *widget) const
    {
      switch (element) {
        /*   case CE_PushButtonLabel:
             {
             QStyleOptionButton myButtonOption;
             const QStyleOptionButton *buttonOption =
             qstyleoption_cast<const QStyleOptionButton *>(option);
             if (buttonOption) {
             myButtonOption = *buttonOption;
             if (myButtonOption.palette.currentColorGroup()
             != QPalette::Disabled) {
             if (myButtonOption.state & (State_Sunken | State_On)) {
             myButtonOption.palette.setBrush(QPalette::ButtonText,
             myButtonOption.palette.brightText());
             }
             }
             }
             QPlastiqueStyle::drawControl(element, &myButtonOption, painter, widget);
             }
             break;*/
      default:
        QPlastiqueStyle::drawControl(element, option, painter, widget);
      }
    }

    void MarsStyle::setTexture(QPalette &palette, QPalette::ColorRole role,
                               const QPixmap &pixmap)
    {
      for (int i = 0; i < QPalette::NColorGroups; ++i) {
        QColor color = palette.brush(QPalette::ColorGroup(i), role).color();
        palette.setBrush(QPalette::ColorGroup(i), role, QBrush(color, pixmap));
      }
    }

    QPainterPath MarsStyle::roundRectPath(const QRect &rect)
    {
      int radius = qMin(rect.width(), rect.height()) / 4;
      int diam = 2 * radius;

      int x1, y1, x2, y2;
      rect.getCoords(&x1, &y1, &x2, &y2);

      QPainterPath path;
      path.moveTo(x2, y1 + radius);
      path.arcTo(QRect(x2 - diam, y1, diam, diam), 0.0, +90.0);
      path.lineTo(x1 + radius, y1);
      path.arcTo(QRect(x1, y1, diam, diam), 90.0, +90.0);
      path.lineTo(x1, y2 - radius);
      path.arcTo(QRect(x1, y2 - diam, diam, diam), 180.0, +90.0);
      path.lineTo(x1 + radius, y2);
      path.arcTo(QRect(x2 - diam, y2 - diam, diam, diam), 270.0, +90.0);
      path.closeSubpath();
      return path;
    }

  } // end of namespace gui
} // end of namespace mars
