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
 * \file CaptureWindow.cpp
 * \author Malte Römmermann
 * \brief "CaptureWindow" provides some options to setup the camera of a 3d window
 **/

#include "CaptureWindow.h"
#include <mars/main_gui/GuiInterface.h>
#include<cstdio>

#include <mars/interfaces/graphics/GraphicsManagerInterface.h>

namespace mars {
  namespace gui {

    CaptureWindow::CaptureWindow(interfaces::ControlCenter* c,
                                 main_gui::GuiInterface *gui) :
      control(c), mainGui(gui) {
  
      mainGui->addGenericMenuAction("../File/Export/Movie", 1,
                                    (main_gui::MenuInterface*)this, 0);
      myWidget = NULL;
      init = false;
      start(1000);
    }


    CaptureWindow::~CaptureWindow() {

    }
  
    void CaptureWindow::menuAction(int action, bool checked) {
      (void)checked;
      switch(action) {
      case 1:
        if (myWidget != NULL && myWidget->pDialog) {
          myWidget->pDialog->close();
          delete myWidget;
          myWidget = NULL;
        }
        myWidget = new CaptureGUI(control);
        //mainGui->addDockWidget((void*)myWidget->pDialog);
        myWidget->show();
        std::vector<CaptureConfig*>::iterator iter;
        myMutex.lock();
        for(iter=cameras.begin(); iter!=cameras.end(); ++iter) {
          myWidget->addCamera((*iter));
        }
        myMutex.unlock();
        connect(myWidget->pDialog, SIGNAL(closeSignal()), this, SLOT(closeWidget()));
        break;
      }
    }

    void CaptureWindow::closeWidget(void) {
      if(myWidget) {
        //mainGui->removeDockWidget((void*)myWidget->pDialog);
        delete myWidget;
        myWidget = NULL;
      }
    }

    void CaptureWindow::timerEvent(QTimerEvent* event) {
      (void)event;
      std::vector<CaptureConfig*>::iterator iter;
      std::vector<unsigned long>::iterator jter;
      std::vector<unsigned long> ids;
      myMutex.lock();
      std::vector<CaptureConfig*> tmpList = cameras;
      myMutex.unlock();
      CaptureConfig *tmpCamera;
      bool found;
  
      if(!init) {
        if(control->graphics) {
          init = true;
          //control->graphics->addGraphicsUpdateInterface((GraphicsUpdateInterface*)this);
        }
      }
      else {
        control->graphics->getList3DWindowIDs(&ids);
    
        for(jter=ids.begin(); jter!=ids.end(); ++jter) {
          found = false;
          for(iter=tmpList.begin(); iter!=tmpList.end(); ++iter) {
            if((*iter)->getWindowID() == (*jter)) {
              found = true;
              tmpList.erase(iter);
              break;
            }
          }
      
          if(!found) {
            tmpCamera = new CaptureConfig(control);
            tmpCamera->setWindowID((*jter));
            myMutex.lock(); 
            cameras.push_back(tmpCamera);
            myMutex.unlock();
            if(myWidget)
              myWidget->addCamera(tmpCamera);
          }
      
          //ids.erase(jter);
        }
    
        myMutex.lock();
        std::vector<CaptureConfig*> tmpList2 = cameras;
        myMutex.unlock();
        tmpList.clear();
        for(iter=tmpList2.begin(); iter!=tmpList2.end(); ++iter) {
          found = false;
          for(jter=ids.begin(); jter!=ids.end(); ++jter) {
            if((*iter)->getWindowID() == (*jter)) {
              found = true;
              ids.erase(jter);
              tmpList.push_back((*iter));
              break;
            }
          }
      
          if(!found) {
            if(myWidget)
              myWidget->removeCamera((*iter));
            delete (*iter);
          }
        }
        myMutex.lock();
        cameras = tmpList;
        myMutex.unlock();
      }
    }

    void CaptureWindow::preGraphicsUpdate(void) {
    }

  } // end of namespace gui
} // end of namespace mars
