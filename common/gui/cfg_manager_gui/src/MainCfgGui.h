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
 * \file MainCfgGui.h
 * \author Malte Römmermann
 * \brief 
 **/

#ifndef _MAIN_CFG_GUI_H
#define _MAIN_CFG_GUI_H

#ifdef _PRINT_HEADER_
#warning "MainCfgGui.h"
#endif

#include "CfgWidget.h"
#include <mars/main_gui/GuiInterface.h>
#include <mars/main_gui/MenuInterface.h>
#include <lib_manager/LibInterface.hpp>
#include <mars/cfg_manager/CFGManagerInterface.h>
#include <mars/cfg_manager/CFGClient.h>
#include <string>
#include <list>

#include <QMutex>
#include <QTimerEvent>

namespace mars {
  namespace cfg_manager_gui {

    class MainCfgGui : public QObject,
                       public lib_manager::LibInterface,
                       public main_gui::MenuInterface,
                       public cfg_manager::CFGClient {
      Q_OBJECT

      public:
      MainCfgGui(lib_manager::LibManager* theManager);
      void setupGUI(std::string path = ".");
   
      virtual ~MainCfgGui(void);
  
      // MenuInterface methods
      virtual void menuAction(int action, bool checked = false);

      // LibInterface methods
      int getLibVersion() const {return 1;}
      const std::string getLibName() const {return std::string("cfg_manager_gui");}
      CREATE_MODULE_INFO();

      virtual void cfgUpdateProperty(cfg_manager::cfgPropertyStruct _propertyS);
      virtual void cfgParamCreated(cfg_manager::cfgParamId _id);
      virtual void cfgParamRemoved(cfg_manager::cfgParamId _id);
      void unsetWidget() { cfgWidget = NULL;}

    private:
      main_gui::GuiInterface *gui;
      cfg_manager::CFGManagerInterface *cfg;
      CfgWidget *cfgWidget;
      cfg_manager::cfgPropertyStruct cfgW_top, cfgW_left;
      cfg_manager::cfgPropertyStruct cfgW_height, cfgW_width;

      bool set_window_prop;
      bool ignore_next;
      std::list<cfg_manager::cfgParamId> registeredParams;

    protected slots:
      void timerEvent(QTimerEvent* event);
      void geometryChanged();
    };

  } // end of namespace cfg_manager_gui
} // end of namespace mars


#endif // _MAIN_CFG_GUI_H
