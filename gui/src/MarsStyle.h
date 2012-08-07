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

#ifndef MARSSTYLE_H
#define MARSSTYLE_H

#include <QPlastiqueStyle>
#include <QPalette>
#include <string>

 class QPainterPath;

namespace mars {
  namespace gui {

    /**
     * \brief A custom GUI style
     */
    class MarsStyle : public QPlastiqueStyle
    {
      Q_OBJECT

      public:
      MarsStyle(std::string stylePath_) : stylePath(stylePath_) {}

      void polish(QPalette &palette);
      void polish(QWidget *widget);
      void unpolish(QWidget *widget);
      int pixelMetric(PixelMetric metric, const QStyleOption *option,
                      const QWidget *widget) const;
      int styleHint(StyleHint hint, const QStyleOption *option,
                    const QWidget *widget, QStyleHintReturn *returnData) const;
      void drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                         QPainter *painter, const QWidget *widget) const;
      void drawControl(ControlElement control, const QStyleOption *option,
                       QPainter *painter, const QWidget *widget) const;
    private:
      std::string stylePath;
     
      static void setTexture(QPalette &palette, QPalette::ColorRole role,
                             const QPixmap &pixmap);
      static QPainterPath roundRectPath(const QRect &rect);
    };

  } // end of namespace gui
} // end of namespace mars

#endif //MARSSTYLE_H
