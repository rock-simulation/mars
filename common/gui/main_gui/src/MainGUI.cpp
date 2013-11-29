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


#include "MainGUI.h"

#ifdef WIN32
  #include <sys/timeb.h>
  #include <windows.h>
#else
  #include <sys/time.h>
  #include <dlfcn.h>
#endif

#include <iostream>
#include <sstream>

#include <QMenuBar>
#include <QToolBar>
#include <QMessageBox>

using namespace std;

namespace mars {
  namespace main_gui {

    MainGUI::MainGUI(lib_manager::LibManager *theManager) :
      GuiInterface(theManager) {
      allow_toolbar = true;

      mainWindow = new MyQMainWindow();
      mainWindow->setUnifiedTitleAndToolBarOnMac(true);
      mainWindow->setWindowTitle(tr("MARS"));

      menuBar = mainWindow->menuBar();

#ifdef __APPLE__
      menuBar->setParent(0);
#endif

      // generate an action to show the Qt about dialog
      helpMenu = menuBar->addMenu("?");
      actionAboutQt = helpMenu->addAction("About Qt");

      //genericMenus.push_back(helpMenu);

      connect(actionAboutQt, SIGNAL(triggered()), this, SLOT(aboutQt()));
    }


    MainGUI::~MainGUI(void) {
      fprintf(stderr, "Delete main_gui\n");
    }

    void MainGUI::show(void) {
      mainWindow->show();
    }

    void MainGUI::setBackgroundImage(const std::string &path) {
      if(path == "") {
        return;
      }

      mdiArea = new MyQMdiArea(path);
      mainWindow->setCentralWidget(mdiArea);

      // create the background image
      mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
      mainWindow->adjustSize();
      mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }


    void MainGUI::genericAction(bool checked) {

      vector<genericMenu>::iterator iter;

      for(iter = genericMenus.begin(); iter != genericMenus.end(); iter++) {
        if((*iter).genericAction == sender()) {
          (*iter).menu->menuAction((*iter).action, checked);
          break;
        }
      }
    }


    void MainGUI::addGenericMenuAction(const std::string &path, int action,
                                       MenuInterface *menu, int qtKey,
                                       const std::string &icon,
                                       bool toolbar, int checkable) {
      vector<string> menuPath;
      QAction *csMenu = 0;
      QMenu *qmenu = 0;
      QToolBar *qtoolbar = 0;
      genericMenu g_menu;
      menuStruct new_menu;
      unsigned int j = 0;
      unsigned int i;
      bool menu_exists = false;

      g_menu.menu = menu;
      g_menu.action = action;

      for(i = 0; i < path.length()+1; i++) {
        if(path[i] == '/') {
          //copy part until '/'
          menuPath.push_back(path.substr(j, i-j));
          j = i + 1;
        }
        //save last entry part
        if(i == path.length() && menuPath.size() > 0) {
          menuPath.push_back(path.substr(j, i-j));
        }
      }

      //if a '/' is in the parentName it seems to be a menu structure
      i = 0; //extract the .. if its in the filename
      if(menuPath.size() > 0) {
        //build main menu entry if first element is ".."
        if(menuPath[0] == "..") {
          i = 1;
          vector<menuStruct>::iterator iter;
          for(iter = v_qmenu.begin(); iter != v_qmenu.end(); iter++) {
            if((*iter).label == menuPath[i]) {
              qmenu = (*iter).menu;
              qtoolbar = (*iter).toolbar;
              if(toolbar && !qtoolbar) {
                qtoolbar = mainWindow->addToolBar(QString::fromStdString(menuPath[i]));
                (*iter).toolbar = qtoolbar;
              }
              menu_exists = true;
              break;
            }
          }
          if(!menu_exists) {
            qmenu = new QMenu(QString::fromStdString(menuPath[i]));
            menuBar->insertMenu(helpMenu->menuAction(), qmenu);
            new_menu.menu = qmenu;
            new_menu.label = menuPath[i];
            if(toolbar && allow_toolbar) {
              qtoolbar = mainWindow->addToolBar(QString::fromStdString(menuPath[i]));
              new_menu.toolbar = qtoolbar;
            } else {
              new_menu.toolbar = 0;
            }
            v_qmenu.push_back(new_menu);
          }
          i++;
        }
        //else use the menuWindows
        else {
          //qmenu = mw_form.menuWindows;
        }


        //build menu structure
        for(; i < menuPath.size(); i++) {
          //if last entry
          if(i == menuPath.size()-1) {

            if(checkable < 0) { // this is only a separator
              csMenu = qmenu->addSeparator();
              return;
            }

            if(icon != "") {
              csMenu = qmenu->addAction(QIcon(icon.data()),
                                        QString::fromStdString(menuPath[i]));
            } else {
              csMenu = qmenu->addAction(QString::fromStdString(menuPath[i]));
            }

            if(toolbar && qtoolbar) {
              qtoolbar->addAction(csMenu);
            }

            if(qtKey != 0) {
              csMenu->setShortcut(QKeySequence((Qt::Key)qtKey));
              csMenu->setShortcutContext(Qt::ApplicationShortcut);
              //shortcut = new QShortcut((Qt::Key)qtKey, mainWindow, 0, 0, Qt::ApplicationShortcut);
              //connect(shortcut, SIGNAL(activated()), csMenu, SLOT(trigger()));
            }
            if(checkable > 0) {
              csMenu->setCheckable(true);
              if(checkable > 1) {
                csMenu->setChecked(true);
              } else {
                csMenu->setChecked(false);
              }
              connect(csMenu, SIGNAL(toggled(bool)), this, SLOT(genericAction(bool)));
            } else {
              connect(csMenu, SIGNAL(triggered()), this, SLOT(genericAction()));
            }
          }
          //else continue building menu groups
          else {
            menu_exists = false;
            vector<menuStruct>::iterator iter;
            for(iter = v_qmenu.begin(); iter != v_qmenu.end(); iter++) {
              if((*iter).label == menuPath[i]) {
                qmenu = (*iter).menu;
                qtoolbar = (*iter).toolbar;
                if(toolbar && !qtoolbar) {
                  qtoolbar = mainWindow->addToolBar(QString::fromStdString(menuPath[i]));
                  (*iter).toolbar = qtoolbar;
                }
                menu_exists = true;
                break;
              }
            }
            if(!menu_exists) {
              qmenu = new QMenu(QString::fromStdString(menuPath[i]));
              menuBar->insertMenu(helpMenu->menuAction(), qmenu);
              new_menu.menu = qmenu;
              new_menu.label = menuPath[i];
              if (toolbar && allow_toolbar) {
                qtoolbar = mainWindow->addToolBar(QString::fromStdString(menuPath[i]));
                new_menu.toolbar = qtoolbar;
              } else {
                new_menu.toolbar = 0;
              }
              v_qmenu.push_back(new_menu);
            }
          }
        }
      } else {
        //if no menu structure is given just create a menu in the window menu
        //csMenu = mw_form.menuWindows->addAction(QString::fromStdString(path));
        if(qtKey != 0) {
          csMenu->setShortcut(QKeySequence((Qt::Key)qtKey));
          csMenu->setShortcutContext(Qt::ApplicationShortcut);
          //shortcut = new QShortcut((Qt::Key)qtKey, mainWindow, 0, 0, Qt::ApplicationShortcut);
          //connect(shortcut, SIGNAL(activated()), csMenu, SLOT(trigger()));
        }
        if(checkable > 0) {
          csMenu->setCheckable(true);
          if(checkable > 1) {
            csMenu->setChecked(true);
          } else {
            csMenu->setChecked(false);
          }
          connect(csMenu, SIGNAL(toggled(bool)), this, SLOT(genericAction(bool)));
        } else {
          connect(csMenu, SIGNAL(triggered()), this, SLOT(genericAction()));
        }
      }
      g_menu.genericAction = csMenu;
      genericMenus.push_back(g_menu);
    }


    void MainGUI::dock(bool checked) {
      if(checked != mainWindow->dockView) {
        mainWindow->dockView = checked;
        mainWindow->dock();
      }
    }


    MyQMainWindow* MainGUI::mainWindow_p() {
      return mainWindow;
    }

    void MainGUI::addDockWidget(void *window, int p, int a) {
      if(window) {
        mainWindow->addDock((QWidget*)window, p, a);
      }
    }


    void MainGUI::removeDockWidget(void *window, int p) {
      if(window) {
        mainWindow->removeDock((QWidget*)window, p);
      }
    }

    int MainGUI::getLibVersion() const {
      return 1;
    }

    const std::string MainGUI::getLibName() const {
      return "main_gui";
    }

    void MainGUI::aboutQt() const {
      QMessageBox::aboutQt(mainWindow, "About Qt");
    }


  } // end namespace main_gui
} // end namespace mars

DESTROY_LIB(mars::main_gui::MainGUI);
CREATE_LIB(mars::main_gui::MainGUI);
