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
 * \file DataWidget.h
 * \author Malte Roemmermann
 * \brief 
 **/

#ifndef DATA_WIDGET_H
#define DATA_WIDGET_H

#ifdef _PRINT_HEADER_
#warning "DataWidget.h"
#endif

#include <mars/main_gui/PropertyDialog.h>
#include <mars/main_gui/BaseWidget.h>
#include <mars/data_broker/ReceiverInterface.h>
#include <mars/data_broker/DataInfo.h>
#include <mars/data_broker/DataPackage.h>

#include <vector>
#include <set>

#include <QWidget>
#include <QCloseEvent>
#include <QMutex>

namespace mars {

  using namespace std;

  namespace data_broker {
    class DataBrokerInterface;
  }
  
  namespace data_broker_gui {

    struct paramWrapper {
      data_broker::DataInfo info;
      data_broker::DataPackage dataPackage;
      std::vector<QtVariantProperty*> guiElements;
    };
  
    class DataWidget : public main_gui::BaseWidget,
                       public data_broker::ReceiverInterface, 
                       public main_gui::PropertyCallback {
    
      Q_OBJECT;
      
    public:
      DataWidget(mars::data_broker::DataBrokerInterface *_dataBroker,
                 mars::cfg_manager::CFGManagerInterface *cfg, QWidget *parent = 0);
      ~DataWidget();
    
      void addParam(const mars::data_broker::DataInfo _info);
      virtual void valueChanged(QtProperty *property, const QVariant &value);
    
      main_gui::PropertyDialog *pDialog;
      void receiveData(const mars::data_broker::DataInfo &info,
                       const mars::data_broker::DataPackage &data_package,
                       int callbackParam);
      //    void receiveAddProducer(const data_broker::data_info &info) {}
      //    void receiveRemoveProducer(const data_broker::data_info &info) {}
    
    private:
      mars::data_broker::DataBrokerInterface *dataBroker;
      QtProperty *showAllProperty;
      bool showAll;
      QMutex addMutex;
      QMutex listMutex;
      QMutex changeMutex;

      set<unsigned long> changeList;
      map<unsigned long, paramWrapper> addList;
      map<unsigned long, paramWrapper> paramList;
      //    map<std::vector<QtVariantProperty*>*, paramWrapper> guiToWrapper;
      map<QtVariantProperty*, paramWrapper*> guiToWrapper;
      bool ignore_change;
    
    protected slots:
      void timerEvent(QTimerEvent* event);
      virtual void accept();
      virtual void reject();
    
    };
  
  } // end of namespace data_broker_gui

} // end of namespace mars 

#endif // DATA_WIDGET_H
  
