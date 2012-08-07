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
 * \file MyQMdiArea.h
 * \author Malte Römmermann
 * \author Vladimir Komsiyski
 */

#ifndef MY_QMDIAREA_H
#define MY_QMDIAREA_H

#ifdef _PRINT_HEADER_
#warning "MyQMdiArea.h"
#endif

#include <QMdiArea>
#include <QPixmap>
#include <string>

namespace mars {
  namespace main_gui {

    /**
     * \brief "MyQMdiArea" is a hack to support a background
     *        image in the MainWindow
     * */
    class MyQMdiArea : public QMdiArea {

      Q_OBJECT

      public:

      /**
       * \brief A constructor.
       * \param path The file path to the image.
       */
      MyQMdiArea(const std::string &path);

      /**
       * \brief A destructor.
       */
      ~MyQMdiArea();


    protected:
      void paintEvent(QPaintEvent *paintEvent);
      void resizeEvent(QResizeEvent *resizeEvent);

    private:
      QPixmap *pixmap;
      QPixmap backgroundPixmap;
      int x, y;

    }; // end class MyQMdiArea

  } // end namespace main_gui
} // end namespace mars

#endif /* MY_QMDIAREA_H */
