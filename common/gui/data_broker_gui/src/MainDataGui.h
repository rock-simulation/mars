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
 * \file MainDataGui.h
 * \author Malte Roemmermann
 * \brief 
 **/

#ifndef MAIN_DATA_GUI_H
#define MAIN_DATA_GUI_H

#ifdef _PRINT_HEADER_
#warning "MainDataGui.h"
#endif

#include "DataWidget.h"
#include "DataConnWidget.h"
#include <mars/lib_manager/LibInterface.h>
#include <mars/cfg_manager/CFGManagerInterface.h>
#include <mars/cfg_manager/CFGClient.h>
#include <mars/main_gui/GuiInterface.h>
#include <mars/main_gui/MenuInterface.h>

#include <string>

#include <QMutex>
#include <QTimerEvent>


namespace mars {

  namespace data_broker {
    class DataBrokerInterface;
  }
  
  namespace data_broker_gui {
    
    class MainDataGui : public QObject, public mars::lib_manager::LibInterface,
                        public mars::main_gui::MenuInterface {
      Q_OBJECT;
      
    public:
      MainDataGui(mars::lib_manager::LibManager* theManager);
      void setupGUI(std::string path = std::string("."));
      
      virtual ~MainDataGui(void);
      
      // MenuInterface methods
      virtual void menuAction(int action, bool checked = false);
      
      // LibInterface methods
      int getLibVersion() const
      { return 1; }
      const std::string getLibName() const
      { return std::string("data_broker_gui"); }
      CREATE_MODULE_INFO();
      
    private:
      mars::main_gui::GuiInterface* gui;
      mars::cfg_manager::CFGManagerInterface *cfg;
      mars::data_broker::DataBrokerInterface *dataBroker;
      
      DataWidget* dataWidget;
      DataConnWidget* dataConnWidget;

    protected slots:
      void timerEvent(QTimerEvent* event);
      
    }; // end of class MainDataGui
    
  } // end of namespace data_broker_gui
  
} // end of namespace mars

#endif // _MAIN_DATA_GUI_H
