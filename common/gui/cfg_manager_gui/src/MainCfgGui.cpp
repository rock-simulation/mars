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
 * \file MainCfgGui.cpp
 * \author Malte Römmermann
 * \brief 
 **/

#include "MainCfgGui.h"
#include <cstdio>
#include <iostream>

namespace mars {
  namespace cfg_manager_gui {

    using namespace cfg_manager;
    using std::vector;
    using std::string;

    MainCfgGui::MainCfgGui(lib_manager::LibManager* theManager) :
      lib_manager::LibInterface(theManager), gui(NULL), 
      cfg(NULL), cfgWidget(NULL) {
      set_window_prop = false;
      ignore_next = false;

      // setup GUI with default path
      setupGUI("../resources/");
    }

    void MainCfgGui::setupGUI(std::string path) {
 
      if(libManager == NULL) return;

      lib_manager::LibInterface *lib = libManager->getLibrary(std::string("cfg_manager"));

      if(lib) {
        cfg = dynamic_cast<CFGManagerInterface*>(lib);
      }
  
      if(cfg) {
        cfg->registerToCFG(dynamic_cast<CFGClient*>(this));
  
        cfgPropertyStruct r_path;
        r_path = cfg->getOrCreateProperty("Preferences", "resources_path",
                                          std::string("../resources"));
        path = r_path.sValue;
      }
      else fprintf(stderr, "******* cfg_manager_gui: couldn't find cfg_manager\n");
  
      lib = libManager->getLibrary(std::string("main_gui"));

      if(lib) {
        //CFG *cfgLib = dynamic_cast<CFG*>(lib);
        //if(cfgLib) {
        gui = dynamic_cast<main_gui::GuiInterface*>(lib);
        //}
      }

      if (gui == NULL)
        return;

      path.append("/mars/cfg_manager_gui/resources/images/preferences.png");
      gui->addGenericMenuAction("../Windows/GuiCfg", 1,
                                dynamic_cast<main_gui::MenuInterface*>(this),0,
                                path, true);

      cfgWidget = new CfgWidget(cfg);

      connect(cfgWidget->pDialog, SIGNAL(geometryChanged()), this, SLOT(geometryChanged()));

      if (cfg)
        setupCFG();

      vector<cfgParamInfo> allParams;
      cfg->getAllParams(&allParams);
      for (unsigned int i = 0; i < allParams.size(); i++)
        cfgParamCreated(allParams[i].id);
    }

  
    MainCfgGui::~MainCfgGui() {
      if(libManager == NULL) return;

      if(cfg) libManager->unloadLibrary(std::string("cfg_manager"));
      if(gui) libManager->unloadLibrary(std::string("main_gui"));
      fprintf(stderr, "Delete cfg_manager_gui\n");
    }


    void MainCfgGui::menuAction(int action, bool checked) {
      (void)checked;

      if (gui == NULL || cfgWidget == NULL)
        return;

      switch(action) {
      case 1:
        if(cfgWidget->isHidden()) {
          gui->addDockWidget((void*)cfgWidget->pDialog, 1);
          cfgWidget->show();
        }
        else {
          cfgWidget->hide();
          gui->removeDockWidget((void*)cfgWidget->pDialog, 1);
        }
        break;
      }
    }

    void MainCfgGui::timerEvent(QTimerEvent *event) {
      (void)event;
    }



    void MainCfgGui::setupCFG(void) {
  
      cfgW_top = cfg->getOrCreateProperty("Windows", "CFG/Window Top", (int)400,
                                          dynamic_cast<CFGClient*>(this));
  
      cfgW_left = cfg->getOrCreateProperty("Windows", "CFG/Window Left", (int)400,
                                           dynamic_cast<CFGClient*>(this));
  
      cfgW_width = cfg->getOrCreateProperty("Windows", "CFG/Window Width", (int)400,
                                            dynamic_cast<CFGClient*>(this));
  
      cfgW_height = cfg->getOrCreateProperty("Windows", "CFG/Window Height", (int)200,
                                             dynamic_cast<CFGClient*>(this));

      cfgWidget->pDialog->setGeometry(cfgW_left.iValue, cfgW_top.iValue,
                                      cfgW_width.iValue, cfgW_height.iValue);
  
    }


    void MainCfgGui::cfgUpdateProperty(cfgPropertyStruct _property) {
      bool change_view = 0;

      if(!cfgWidget)
        return;

      cfgWidget->changeParam(_property.paramId);  

      if(set_window_prop) return;

      if(_property.paramId == cfgW_top.paramId) {
        cfgW_top.iValue = _property.iValue;
        change_view = 1;
      }
  
      else if(_property.paramId == cfgW_left.paramId) {
        cfgW_left.iValue = _property.iValue;
        change_view = 1;
      }

      else if(_property.paramId == cfgW_width.paramId) {
        cfgW_width.iValue = _property.iValue;
        change_view = 1;
      }

      else if(_property.paramId == cfgW_height.paramId) {
        cfgW_height.iValue = _property.iValue;
        change_view = 1;
      }
  
      if(change_view) {
        ignore_next = true;
        cfgWidget->pDialog->setGeometry(cfgW_left.iValue, cfgW_top.iValue,
                                        cfgW_width.iValue, cfgW_height.iValue);
      }
    }


    void MainCfgGui::geometryChanged() {
      bool update_cfg = false;
 
      if (ignore_next) {
        ignore_next = false;
        return;
      }
 
      int top = cfgWidget->pDialog->geometry().y();
      int left = cfgWidget->pDialog->geometry().x();
      int width = cfgWidget->pDialog->geometry().width();
      int height = cfgWidget->pDialog->geometry().height();

      if(top != cfgW_top.iValue) {
        cfgW_top.iValue = top;
        update_cfg = true;
      }
      if(left != cfgW_left.iValue) {
        cfgW_left.iValue = left;
        update_cfg = true;
      }
      if(width != cfgW_width.iValue) {
        cfgW_width.iValue = width;
        update_cfg = true;
      }
      if(height != cfgW_height.iValue) {
        cfgW_height.iValue = height;
        update_cfg = true;
      }
      if(update_cfg && cfg) {
        set_window_prop = true;
        cfg->setProperty(cfgW_top);
        cfg->setProperty(cfgW_left);
        cfg->setProperty(cfgW_width);
        cfg->setProperty(cfgW_height);
        set_window_prop = false;
      }
    }





    void MainCfgGui::cfgParamCreated(cfgParamId _id) {
      const cfgParamInfo newParam = cfg->getParamInfo(_id);

      if(cfgWidget) cfgWidget->addParam(newParam);
      // register to get param updates
      cfg->registerToParam(_id, dynamic_cast<CFGClient*>(this));
    }

    void MainCfgGui::cfgParamRemoved(cfgParamId _id) {
      cfgWidget->removeParam(_id);
    }

    void MainCfgGui::show() {
      if (cfgWidget) {
        cfgWidget->show();
      }
    }

    void MainCfgGui::hide() {
      if (cfgWidget) {
        cfgWidget->hide();
      }
    }

    bool MainCfgGui::isHidden() const {
      if (cfgWidget) {
        return cfgWidget->isHidden();
      }
      return false;
    }


  } // end of namespace cfg_manager_gui
} // end of namespace mars

DESTROY_LIB(mars::cfg_manager_gui::MainCfgGui);
CREATE_LIB(mars::cfg_manager_gui::MainCfgGui);
