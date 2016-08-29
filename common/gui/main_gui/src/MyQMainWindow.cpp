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


#include "MyQMainWindow.h"

#include <iostream>
#include <fstream>

namespace mars {
  namespace main_gui {

    MyQMainWindow::MyQMainWindow(QWidget *parent) : QMainWindow(parent) {
      closing = false;
      dockView = false;
      hideChild = false;
      myCentralWidget = 0;
      p = 0;

      widgetState mWin = {this, Qt::NoDockWidgetArea, this->geometry()};
      widgetStates.push_back(mWin);
      dockGeometry = mWin.rect;
      timerAllowed = true;
      startTimer(500);
    }


    MyQMainWindow::~MyQMainWindow() {
    }

    void MyQMainWindow::setCentralWidget(QWidget *widget) {
      handleState(myCentralWidget, true, true, true); // remove the previous central widget
      widgetState central = {widget, Qt::NoDockWidgetArea, widget->geometry()};
      widgetStates.push_back(central);

      widget->setWindowIcon(windowIcon());
      widget->show();
      myCentralWidget = widget;
      p = (void*)widget->parentWidget();
      myCentralWidget->setMinimumSize(720, 405);
      if(dockView) {
        QMainWindow::setCentralWidget(widget);
      }
    }


    void MyQMainWindow::dock() {
      timerAllowed = false;
      if(!dockView) {
        saveDockGeometry();
        dockGeometry = this->geometry(); // save the geometry when docked

        // undock central widget
        if(myCentralWidget == centralWidget()) {
          myCentralWidget->setParent((QWidget*)p);
          handleState(myCentralWidget, false, true); // restore geometry
          myCentralWidget->show();
          QMainWindow::setCentralWidget(0);
        }
        // undock unclosable widgets
        for(dockit = stDockWidgets.begin(); dockit != stDockWidgets.end(); dockit++) {
          QWidget *temp = (*dockit)->widget();
          handleState(temp, true, false); // save area
          handleState(temp, false, true); // restore geometry
          stSubWindows.push_back(temp);
          temp->setParent(0);
          temp->setVisible((*dockit)->isVisible());
          removeDockWidget(*dockit);
        }
        // undock closable widgets
        for(dockit = dyDockWidgets.begin(); dockit != dyDockWidgets.end(); dockit++) {
          QWidget *temp = (*dockit)->widget();
          handleState(temp, true, false); // save area
          handleState(temp, false, true); // restore geometry
          dySubWindows.push_back(temp);
          temp->setParent(0);
          temp->show();
          removeDockWidget(*dockit);
        }
        stDockWidgets.clear();
        dyDockWidgets.clear();

        //this->hide();
        this->setMinimumSize(0, 0);
        //handleState(this, false, true); // restore main window geometry
        this->setGeometry(dockGeometry); // restore the dock geometry
        //this->show();
      } else {
        dockGeometry = this->geometry(); // save the geometry when docked
        hideChild = true;
        handleState(this, true, true); // save main window gometry
        // dock central widget
        if(myCentralWidget) {
          handleState(myCentralWidget, true, true); // save geometry
          QMainWindow::setCentralWidget(myCentralWidget);
        }
        // dock unclosable widgets
        for(subit = stSubWindows.begin(); subit != stSubWindows.end(); subit++) {
          int visible = (*subit)->isVisible();
          handleState(*subit, true, true); // save geometry
          MyQDockWidget *toDock = new MyQDockWidget(this, *subit, 1, 0);
          handleState(*subit, false, false); // restore area
          stDockWidgets.push_back(toDock);
          toDock->setVisible(visible);
        }
        // dock closable widgets
        for(subit = dySubWindows.begin(); subit != dySubWindows.end(); subit++) {
          if((*subit)->isVisible()) {
            handleState(*subit, true, true); // save geometry
            MyQDockWidget *toDock = new MyQDockWidget(this, *subit, 0, 1);
            handleState(*subit, false, false); // restore area
            dyDockWidgets.push_back(toDock);
          } else {
            (*subit)->close();
          }
        }
        hideChild = false;
        stSubWindows.clear();
        dySubWindows.clear();
        this->setGeometry(dockGeometry); // restore the main window geometry
        restoreDockGeometry();
      }
      timerAllowed = true;

    }


    void MyQMainWindow::addDock(QWidget *window, int priority, int area,
                                bool possibleCentralWidget) {
      if(!myCentralWidget && possibleCentralWidget) {
        setCentralWidget(window);
        return;
      }
      timerAllowed = false;
      //  static int objectName = 42;
      Qt::DockWidgetArea qtarea;
      bool floating = false;
      switch (area) {
      case 1:
        area = qtarea = Qt::LeftDockWidgetArea;
        break;
      case 2:
        area = qtarea = Qt::RightDockWidgetArea;
        break;
      case 3:
        area = qtarea = Qt::TopDockWidgetArea;
        break;
      case 4:
        area = qtarea = Qt::BottomDockWidgetArea;
        break;
      case 5:
        area = qtarea = Qt::LeftDockWidgetArea;
        floating = true;
        break;
      default:
        area = qtarea = Qt::LeftDockWidgetArea;
        break;
      }

      window->setWindowIcon(windowIcon());

      if(dockView) {
        if(priority) {
          for(dockit = stDockWidgets.begin(); dockit != stDockWidgets.end(); dockit++) {
            if ((*dockit)->widget() == window) {
              (*dockit)->show();
              return;
            }
          }
          //window->setObjectName(QString::number(objectName++));
          widgetState st = {window, qtarea, window->geometry()};
          widgetStates.push_back(st); // save the state
          MyQDockWidget *toDock = new MyQDockWidget(this, window, 1, area);
          stDockWidgets.push_back(toDock);
          toDock->setFloating(floating);
        } else {
          for (dockit = dyDockWidgets.begin(); dockit != dyDockWidgets.end(); dockit++) {
            if ((*dockit)->widget() == window) {
              removeDockWidget(*dockit);
              dyDockWidgets.erase(dockit);
              break;
            }
          }
          //window->setObjectName(QString::number(objectName++));
          widgetState st = {window, qtarea, window->geometry()};
          widgetStates.push_back(st); // save the state
          MyQDockWidget *toDock = new MyQDockWidget(this, window, 0, area);
          dyDockWidgets.push_back(toDock);
          toDock->setFloating(floating);
        }
      } else {
        if(priority) {
          for (subit = stSubWindows.begin(); subit != stSubWindows.end(); subit++) {
            if (*subit == window) {
              return;
            }
          }
          //window->setObjectName(QString::number(objectName++));
          widgetState st = {window, qtarea, window->geometry()};
          widgetStates.push_back(st); // save the state
          stSubWindows.push_back(window);
        } else {
          for(subit = dySubWindows.begin(); subit != dySubWindows.end(); subit++) {
            if (*subit == window) {
              (*subit)->close();
              dySubWindows.erase(subit);
              handleState(window, true, true, true); // remove from the states
              break;
            }
          }
          //window->setObjectName(QString::number(objectName++));
          widgetState st = {window, qtarea, window->geometry()};
          widgetStates.push_back(st); // save the state
          dySubWindows.push_back(window);
        }
      }
      restoreDockGeometry();
      timerAllowed = true;
    }


    void MyQMainWindow::removeDock(QWidget *window, int priority) {
      if(closing) {
        return;
      }

      if(dockView) {
        if(priority) {
          for(dockit = stDockWidgets.begin(); dockit != stDockWidgets.end(); dockit++) {
            if((*dockit)->widget() == window) {
              (*dockit)->widget()->hide();
              (*dockit)->hide();
              break;
            }
          }
        } else {
          for(dockit = dyDockWidgets.begin(); dockit != dyDockWidgets.end(); dockit++) {
            if((*dockit)->widget() == window) {
              handleState(window, true, true, true); // remove from the states
              removeDockWidget(*dockit);
              dyDockWidgets.erase(dockit);
              break;
            }
          }
        }
      } else {
        if(priority) {
          for(subit = stSubWindows.begin(); subit != stSubWindows.end(); subit++) {
            if(*subit == window) {
              window->hide();
              break;
            }
          }
        } else {
          for(subit = dySubWindows.begin(); subit != dySubWindows.end(); subit++) {
            if(*subit == window) {
              handleState(window, true, true, true); // remove from the states
              window->close();
              dySubWindows.erase(subit);
              break;
            }
          }
        }
      }
    }


    void MyQMainWindow::closeEvent(QCloseEvent *event) {
      timerAllowed = false;
      (void)event;
      closing = true;
      if(myCentralWidget) {
        myCentralWidget->close();
      }
      for(dockit = stDockWidgets.begin(); dockit != stDockWidgets.end(); dockit++) {
        (*dockit)->widget()->close();
        (*dockit)->close();
      }
      for(dockit = dyDockWidgets.begin(); dockit != dyDockWidgets.end(); dockit++) {
        (*dockit)->widget()->close();
        (*dockit)->close();
      }
      for(subit = stSubWindows.begin(); subit != stSubWindows.end(); subit++) {
        (*subit)->close();
      }
      for(subit = dySubWindows.begin(); subit != dySubWindows.end(); subit++) {
        (*subit)->close();
      }
    }


    void MyQMainWindow::handleState(QWidget *w, bool save, bool rect, bool remove) {
      for(unsigned int i = 0; i < widgetStates.size(); i++) {
        if(widgetStates[i].widget == w) {
          if(remove) {
            widgetStates.erase(widgetStates.begin() + i);
          } else if(save) {
            if(rect) {
              widgetStates[i].rect = w->geometry();
            } else {
              widgetStates[i].area = dockWidgetArea((MyQDockWidget*)(w->parentWidget()));
            }
          } else {
            if(rect) {
              w->setGeometry(widgetStates[i].rect);
            } else {
              addDockWidget(widgetStates[i].area, (MyQDockWidget*)(w->parentWidget()));
            }
          }
          break;
        }
      } // for
    }

    void MyQMainWindow::timerEvent(QTimerEvent *event) {
      (void)event;
      if(dockView && timerAllowed) {
        saveDockGeometry();
      }
    }

    void MyQMainWindow::saveDockGeometry() {
      int flag = 0;
      for(unsigned int j = 0; j < stDockWidgets.size(); j++) {
        for(unsigned int i = 0; !flag && i < dockStates.size(); i++) {
          if(stDockWidgets[j]->widget()->windowTitle() == dockStates[i].title) {
            dockStates[i].floating = stDockWidgets[j]->isFloating();
            if(!dockStates[i].floating) {
              dockStates[i].area = dockWidgetArea(stDockWidgets[j]);
            }
            dockStates[i].rect = stDockWidgets[j]->geometry();

            flag = 1;
          }
        }
        if(flag == 0) {
          dockState justAdded = {stDockWidgets[j]->widget()->windowTitle(),
                                 dockWidgetArea(stDockWidgets[j]),
                                 stDockWidgets[j]->isFloating(),
                                 stDockWidgets[j]->geometry()};
          dockStates.push_back(justAdded);
        }
        flag = 0;
      }
      flag = 0;
      for(unsigned int j = 0; j < dyDockWidgets.size(); j++) {
        for(unsigned int i = 0; !flag && i < dockStates.size(); i++) {
          if(dyDockWidgets[j]->widget()->windowTitle() == dockStates[i].title) {
            dockStates[i].floating = dyDockWidgets[j]->isFloating();
            if (!dockStates[i].floating) {
              dockStates[i].area = dockWidgetArea(dyDockWidgets[j]);
            }
            dockStates[i].rect = dyDockWidgets[j]->geometry();

            flag = 1;
          }
        }
        if(flag == 0) {
          dockState justAdded = {dyDockWidgets[j]->widget()->windowTitle(),
                                 dockWidgetArea(dyDockWidgets[j]),
                                 dyDockWidgets[j]->isFloating(),
                                 dyDockWidgets[j]->geometry()};
          dockStates.push_back(justAdded);
        }
        flag = 0;
      }

    }


    void MyQMainWindow::restoreDockGeometry() {
      for(unsigned int j = 0; j < stDockWidgets.size(); j++) {
        for(unsigned int i = 0; i < dockStates.size(); i++) {
          if(stDockWidgets[j]->widget()->windowTitle() == dockStates[i].title) {
            addDockWidget(dockStates[i].area, stDockWidgets[j]);
            stDockWidgets[j]->setFloating(dockStates[i].floating);
            stDockWidgets[j]->setGeometry(dockStates[i].rect);
          }
        }
      }
      for(unsigned int j = 0; j < dyDockWidgets.size(); j++) {
        for(unsigned int i = 0; i < dockStates.size(); i++) {
          if(dyDockWidgets[j]->widget()->windowTitle() == dockStates[i].title) {
            addDockWidget(dockStates[i].area, dyDockWidgets[j]);
            dyDockWidgets[j]->setFloating(dockStates[i].floating);
            dyDockWidgets[j]->setGeometry(dockStates[i].rect);
          }
        }
      }
    }

  } // end namespace main_gui
} // end namespace mars

