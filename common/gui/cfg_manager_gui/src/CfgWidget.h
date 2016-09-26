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
 * \file CfgWidget.h
 * \author Malte Römmermann
 * \brief 
 **/

#ifndef CFG_WIDGET_H
#define CFG_WIDGET_H

#ifdef _PRINT_HEADER_
#warning "CfgWidget.h"
#endif

#include <QWidget>
#include <QCloseEvent>
#include <QMutex>

#include <mars/cfg_manager/CFGManagerInterface.h>
#include <mars/main_gui/PropertyDialog.h>
#include <mars/main_gui/BaseWidget.h>
#include <vector>


namespace mars {
  namespace cfg_manager_gui {

    struct paramWrapper {
      cfg_manager::cfgParamInfo theParam;
      cfg_manager::cfgPropertyStruct theProp;
      QtVariantProperty *guiElem;
    };

    class MainCfgGui;

    class CfgWidget : public main_gui::BaseWidget,
      public main_gui::PropertyCallback {
      Q_OBJECT

      public:
      CfgWidget(MainCfgGui *mainCfgGui,
                cfg_manager::CFGManagerInterface *_cfg, QWidget *parent = 0);
      ~CfgWidget();

      void addParam(const cfg_manager::cfgParamInfo &newParam);
      void changeParam(const cfg_manager::cfgParamId _id);
      void removeParam(const cfg_manager::cfgParamId _id);
      virtual void valueChanged(QtProperty *property, const QVariant &value);

      main_gui::PropertyDialog *pDialog;

    private:
      MainCfgGui *mainCfgGui;
      QMutex listMutex;
      QMutex addMutex;
      QMutex changeMutex;
      QMutex removeMutex;

      std::vector<cfg_manager::cfgParamId> removeList;
      std::vector<cfg_manager::cfgParamId> changeList;
      std::vector<cfg_manager::cfgParamInfo> addList;
      std::vector<paramWrapper> paramList;

      bool ignore_change;

      paramWrapper* getWrapperById(cfg_manager::cfgParamId _id);
  
    protected slots:
      void timerEvent(QTimerEvent *event);
      virtual void accept();
      virtual void reject();

    };

  } // end of namespace cfg_manager_gui
} // end of namespace mars

#endif // CFG_WIDGET_H
