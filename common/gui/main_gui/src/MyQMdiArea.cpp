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


#include "MyQMdiArea.h"

#include <QPainter>
#include <QResizeEvent>
#include <QScrollBar>

namespace mars {
  namespace main_gui {


    MyQMdiArea::MyQMdiArea(const std::string &path) : pixmap(NULL) {
      if(path.length() > 0) {
        pixmap = new QPixmap(path.data());
        backgroundPixmap = *pixmap;
      }
      x = 0;
      y = 0;
    }

    MyQMdiArea::~MyQMdiArea() {
    }

    void MyQMdiArea::paintEvent(QPaintEvent *paintEvent) {
      (void)paintEvent;
      if(!pixmap) {
        return;
      }
      QPainter painter(viewport());
      QRect rc = painter.viewport();

      painter.fillRect(rc, QBrush(QColor(192, 192, 192), Qt::SolidPattern));
      painter.drawPixmap(x, y, backgroundPixmap);
    }

    void MyQMdiArea::resizeEvent(QResizeEvent *resizeEvent) {
      QSize size = resizeEvent->size();

      if(verticalScrollBar()->isVisible()) {
        size += QSize(verticalScrollBar()->width(), 0);
      }

      if(horizontalScrollBar()->isVisible()) {
        size += QSize(0, horizontalScrollBar()->height());
      }

      if(!pixmap) {
        return;
      }
      backgroundPixmap = *pixmap;

      if(size.width() < backgroundPixmap.width()) {
        backgroundPixmap = backgroundPixmap.scaledToWidth(size.width(),
                                                          Qt::SmoothTransformation);
      }

      if(size.height() < backgroundPixmap.height()) {
        backgroundPixmap = backgroundPixmap.scaledToHeight(size.height(),
                                                           Qt::SmoothTransformation);
      }

      x = (size.width() - backgroundPixmap.width())/2;
      y = (size.height() - backgroundPixmap.height())/2;
    }

  } // end namespace main_gui
} // end namespace mars
