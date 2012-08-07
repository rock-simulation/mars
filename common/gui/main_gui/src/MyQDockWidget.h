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
 * \file MyQDockWidget.h
 * \author Vladimir Komsiyski
 **/

#ifndef MYQDOCKWIDGET_H
#define MYQDOCKWIDGET_H

#ifdef _PRINT_HEADER_
  #warning "MyQDockWidget.h"
#endif

#include <QDockWidget>
#include <QCloseEvent>
#include <QHideEvent>

namespace mars {
  namespace main_gui {

    /** \brief Implementation of a dock widget taking care of its widget */
    class MyQDockWidget : public QDockWidget {

      Q_OBJECT

      public:
      /**
       * \brief A constructor.
       * \param parent The parent widget.
       * \param child The widget to be docked.
       * \param priority Indicator if the widget is closable or not.
       * \param area The QDockWidgetArea that is to be used initialy.
       */
      MyQDockWidget(QWidget *parent, QWidget *child, int priority,
                    int area = 0, Qt::WindowFlags flags = 0);

      /**
       * \brief A destrutor.
       */
      ~MyQDockWidget();

      /**
       * Holds the docking area attribute.
       * 0: Qt::LeftDockWidgetArea;
       * 1: Qt::RightDockWidgetArea;
       * 2: Qt::BottomDockWidgetArea;
       * 3: Qt::TopDockWidgetArea.
       **/
      Qt::DockWidgetArea area;

      /**
       * Holds the attribute for closing the widget.
       * 0: Closable;
       * 1: Not closable.
       */
      int priority;

    private:
      void closeEvent(QCloseEvent *event);
      void hideEvent(QHideEvent *event);

    }; // end class MyQDockWidget

  } // end namespace main_gui
} // end namespace mars

#endif /* MYQDOCKWIDGET_H */
