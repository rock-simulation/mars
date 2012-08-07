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

/** \file MyQMainWindow.h
 * \author Vladimir Komsiyski
 */

#ifndef MYQMAINWINDOW_H
#define MYQMAINWINDOW_H


#include <QMainWindow>
#include "MyQDockWidget.h"
#include <vector>
#include <map>


namespace mars {
  namespace main_gui {


    struct widgetState {
      QWidget *widget;
      Qt::DockWidgetArea area;
      QRect rect;
    };

    struct dockState {
      QString title;
      Qt::DockWidgetArea area;
      bool floating;
      QRect rect;
    };


    /**
     * \brief The main window of the GUI.
     */
    class MyQMainWindow : public QMainWindow {

      Q_OBJECT

      public:
      //! A constructor.
      MyQMainWindow(QWidget *parent = 0);

      //! A destructor.
      ~MyQMainWindow();

      //! Indicates the current mode - docked or not.
      int dockView;
      //! Used when manually undocking.
      int hideChild;
      //! Indicates if the window is being closed.
      int closing;

      std::vector<dockState> getDockGeometry() {
        return dockStates;
      }

      void setDockGeometry(std::vector<dockState> states) {
        timerAllowed = false;
        dockStates = states;
        restoreDockGeometry();
        timerAllowed = true;
      }


    public slots:
      //! Docks/Undocks all widgets.
      void dock();

      //! Sets a central widget of the main window.
      void setCentralWidget(QWidget *widget);

      /**
       * \brief Adds a widget to the dockables.
       * \sa GuiInterface::addDockWidget(void*, int, int)
       */
      void addDock(QWidget *window, int priority = 0, int area = 0);

      /**
       * \brief Removes a widget from the dockables.
       * \sa GuiInterface::removeDockWidget(void*, int)
       */
      void removeDock(QWidget *window, int priority = 0);


    private slots:

      void saveDockGeometry();


    private:

      void closeEvent(QCloseEvent *event);

      /* save -> save or restore;
       * rect -> rect or area;
       * remove -> remove or not
       */
      void handleState(QWidget *w, bool save, bool rect, bool remove = false);

      void timerEvent(QTimerEvent *event);

      QWidget *myCentralWidget;
      void *p; // the parent of the central widget
      QRect dockGeometry;
      bool timerAllowed;
      void restoreDockGeometry();

      std::vector<dockState> dockStates;
      std::vector<widgetState> widgetStates;
      std::vector<MyQDockWidget*> dyDockWidgets; // dynamic - deleted on close
      std::vector<MyQDockWidget*> stDockWidgets; // static - never deleted, just hidden
      std::vector<MyQDockWidget*>::iterator dockit;
      std::vector<QWidget*> dySubWindows; // dynamic
      std::vector<QWidget*> stSubWindows; // static
      std::vector<QWidget*>::iterator subit;

    }; // end class MyQMainWindow

  } // end namespace main_gui
} // end namespace mars

#endif /* MYQMAINWINDOW_H */
