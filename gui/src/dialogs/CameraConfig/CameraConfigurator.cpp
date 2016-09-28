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
 * \file CameraConfigurator.cpp
 * \author Malte Römmermann
 * \brief "CameraConfigurator" provides some options to setup the camera of a 3d window
 **/

#include "CameraConfigurator.h"
#include <mars/main_gui/GuiInterface.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>
#include <cstdio>

namespace mars {
  namespace gui {

    CameraConfigurator::CameraConfigurator(interfaces::ControlCenter* c,
                                           main_gui::GuiInterface *gui) : 
      control(c), mainGui(gui) {
  
      mainGui->addGenericMenuAction("../View/Configure 3DView(s)", 1,
                                    (main_gui::MenuInterface*)this, 0);
      myWidget = NULL;
      init = false;
      start(200);
      //  control->dataBroker->registerTriggeredReceiver(this, "mars_sim", "simTime", "preGraphicsUpdate2");
    }


    CameraConfigurator::~CameraConfigurator() {

    }
  
    void CameraConfigurator::menuAction(int action, bool checked) {
      (void)checked;
      switch(action) {
      case 1:
        if (myWidget != NULL && myWidget->pDialog) {
          myWidget->close();
        }
        myWidget = new CameraConfiguratorGUI(control);
        //mainGui->addDockWidget((void*)myWidget);
        myWidget->show();
        std::vector<CameraConfig*>::iterator iter;
        myMutex.lock();
        for(iter=cameras.begin(); iter!=cameras.end(); ++iter)
          myWidget->addCamera((*iter));
        myMutex.unlock();
        connect(myWidget->pDialog, SIGNAL(closeSignal()), this, SLOT(closeWidget()));
        break;
      }
    }


    void CameraConfigurator::closeWidget(void) {
      if(myWidget) {
        //mainGui->removeDockWidget((void*)myWidget->pDialog);
        if(myWidget) {
          delete myWidget;
          myWidget = NULL;
        }
      }
    }

    void CameraConfigurator::timerEvent(QTimerEvent* event) {
      (void)event;
      std::vector<CameraConfig*>::iterator iter;
      std::vector<unsigned long>::iterator jter;
      std::vector<unsigned long> ids;
      myMutex.lock();
      std::vector<CameraConfig*> tmpList = cameras;
      myMutex.unlock();
      CameraConfig *tmpCamera;
      bool found;
  
      if(!init) {
        if(control->graphics) {
          init = true;
          control->graphics->addGraphicsUpdateInterface((GraphicsUpdateInterface*)this);
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
            tmpCamera = new CameraConfig(control);
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
        std::vector<CameraConfig*> tmpList2 = cameras;
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

    /*
      void CameraConfigurator::receiveData(const data_broker::DataInfo &info,
      const data_broker::DataPackage &package) {
      preGraphicsUpdate();
      }
    */

    void CameraConfigurator::preGraphicsUpdate(void) {
      std::vector<CameraConfig*>::iterator iter;

      myMutex.lock();
      for(iter=cameras.begin(); iter!=cameras.end(); ++iter)
        (*iter)->updateCamera();
      myMutex.unlock();
    }

  } // end of namespace gui
} // end of namespace mars
