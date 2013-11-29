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

#include "DataWidget.h"
#include <mars/data_broker/DataBrokerInterface.h>

#include <QVBoxLayout>

#include <cassert>

namespace mars {

  namespace data_broker_gui {
    
    using data_broker::DataItem;
    using data_broker::DataInfo;
    using data_broker::DataBrokerInterface;

    enum { CALLBACK_OTHER=0, CALLBACK_NEW_STREAM };

    DataWidget::DataWidget(DataBrokerInterface *_dataBroker,
                           cfg_manager::CFGManagerInterface *cfg,
                           QWidget *parent) : 
      main_gui::BaseWidget(parent, cfg, "DataBrokerWidget"),
      pDialog(new main_gui::PropertyDialog(parent)),
      dataBroker(_dataBroker),
      ignore_change(0) {

      startTimer(500);

      QVBoxLayout *vLayout = new QVBoxLayout();
      vLayout->addWidget(pDialog);
      setLayout(vLayout);

      pDialog->setButtonBoxVisibility(false);
      pDialog->setPropCallback(dynamic_cast<main_gui::PropertyCallback*>(this));
      showAll = false;
      showAllProperty = pDialog->addGenericProperty("data_broker/ShowAll", 
                                                    QVariant::Bool, showAll);
   
      if(dataBroker) {
        dataBroker->registerSyncReceiver(this, "data_broker", "newStream",
                                         CALLBACK_NEW_STREAM);
        std::vector<DataInfo> infoList;
        std::vector<DataInfo>::iterator it;
        infoList = dataBroker->getDataList();

        for(it=infoList.begin(); it!=infoList.end(); ++it) {
          if(it->flags & data_broker::DATA_PACKAGE_WRITE_FLAG) {
            addParam(*it);
            dataBroker->registerTimedReceiver(this, it->groupName, it->dataName,
                                              "_REALTIME_", 250);
            //          dataBroker->registerAsyncReceiver(this, it->groupName, it->dataName);
          }
        }
      }  
    }

    DataWidget::~DataWidget(void) {
      dataBroker->unregisterAsyncReceiver(this, "*", "*");
      dataBroker->unregisterTimedReceiver(this, "*", "*", "_REALTIME_");
      dataBroker->unregisterSyncReceiver(this, "data_broker", "newStream");
    }

    void DataWidget::addParam(const DataInfo _info) {
      addMutex.lock();
      paramWrapper newParam;
      newParam.info = _info;
      newParam.dataPackage = dataBroker->getDataPackage(_info.dataId);
      addList[_info.dataId] = newParam;
    
      addMutex.unlock();
    }

    void DataWidget::receiveData(const DataInfo &info,
                                 const data_broker::DataPackage &dataPackage,
                                 int callbackParam) {
      changeMutex.lock();
      if(callbackParam == CALLBACK_NEW_STREAM) {
        DataInfo newInfo;
        dataPackage.get("groupName", &newInfo.groupName);
        dataPackage.get("dataName", &newInfo.dataName);
        dataPackage.get("dataId", (long*)&newInfo.dataId);
        dataPackage.get("flags", (int*)&newInfo.flags);
        if(showAll || newInfo.flags & data_broker::DATA_PACKAGE_WRITE_FLAG) {
          addParam(newInfo);
          dataBroker->registerTimedReceiver(this, newInfo.groupName, 
                                            newInfo.dataName, "_REALTIME_", 250);
          //        dataBroker->registerAsyncReceiver(this, newInfo.groupName, newInfo.dataName);
        }
      } else {
        map<unsigned long, paramWrapper>::iterator it;
        it = paramList.find(info.dataId);
        if(it != paramList.end()) {
          listMutex.lock();
          //        assert(it->second.dataPackage.size());
          it->second.dataPackage = dataPackage;
          listMutex.unlock();
          changeList.insert(info.dataId);
        }
      }
      changeMutex.unlock();
    }

    void DataWidget::timerEvent(QTimerEvent* event) {
      (void)event;

      string path;

      // go throud the add list
      addMutex.lock();
      ignore_change = true;
      map<unsigned long, paramWrapper>::iterator it;
      map<unsigned long, paramWrapper> tmpList;
      DataItem *item;
      //DataItem *item2;

      for(it=addList.begin(); it!=addList.end(); ++it) {
        it->second.dataPackage = dataBroker->getDataPackage(it->second.info.dataId);
        for(unsigned int i = 0; i < it->second.dataPackage.size(); ++i) {
          std::map<QString, QVariant> attr;
          attr["singleStep"] = 0.01;
          attr["decimals"] = 7;
          path = "";
          path.append(it->second.info.groupName);
          path.append("/");
          path.append(it->second.info.dataName);
          if(it->second.dataPackage.size() > 1) {
            path.append("/");
            path.append(it->second.dataPackage[i].getName());
          }
          item = &it->second.dataPackage[i];
          QtVariantProperty *guiElem;
          switch(item->type) {
          case data_broker::FLOAT_TYPE:
            guiElem = pDialog->addGenericProperty(path,
                                                  QVariant::Double,
                                                  item->f, &attr);
            break;
          case data_broker::DOUBLE_TYPE:
            guiElem = pDialog->addGenericProperty(path,
                                                  QVariant::Double,
                                                  item->d, &attr);
            break;
          case data_broker::INT_TYPE:
            guiElem = pDialog->addGenericProperty(path, QVariant::Int,
                                                  item->i);
            break;
          case data_broker::LONG_TYPE:
            guiElem = pDialog->addGenericProperty(path, QVariant::Int,
                                                  (int)item->l);
            break;
          case data_broker::BOOL_TYPE:
            guiElem = pDialog->addGenericProperty(path, QVariant::Bool,
                                                  item->b);
            break;
          case data_broker::STRING_TYPE:
            guiElem = pDialog->addGenericProperty(path, QVariant::String,
                                                  QString::fromStdString(item->s));
            break;
          default:
            guiElem = 0;
          }
          if(guiElem && 
             !(it->second.info.flags & data_broker::DATA_PACKAGE_WRITE_FLAG)) {
            guiElem->setEnabled(false);
          }
          it->second.guiElements.push_back(guiElem);
          if(!it->second.guiElements.empty()) {
            listMutex.lock();
            paramList[it->first] = it->second;
            guiToWrapper[guiElem] = &paramList[it->first];//it->second;
            //guiToWrapper[&it->second.guiElements] = it->second;
            listMutex.unlock();
          }
        }
      
        if(it->second.guiElements.empty()) {
          tmpList[it->first] = it->second;
        }
      }
      addList.clear();
      addList = tmpList;
    
      ignore_change = false;
      addMutex.unlock();
      // and check for updates
      changeMutex.lock();
      ignore_change = true;
      while(changeList.size() > 0) {
        it = paramList.find(*changeList.begin());
        if(it != paramList.end() && !it->second.guiElements.empty()) {
          for(unsigned int i = 0; i < it->second.guiElements.size(); ++i) {
            QtVariantProperty *guiElem = it->second.guiElements[i];
            item = &it->second.dataPackage[i];
            //item2 = &guiToWrapper[it->second.guiElements.front()]->dataPackage[i];
            switch(item->type) {
            case data_broker::DOUBLE_TYPE:
              guiElem->setValue(QVariant(item->d));
              //item2->d = item->d;
              break;
            case data_broker::FLOAT_TYPE:
              guiElem->setValue(QVariant(item->f));
              //item2->f = item->f;
              break;
            case data_broker::INT_TYPE:
              guiElem->setValue(QVariant(item->i));
              //item2->i = item->i;
              break;
            case data_broker::LONG_TYPE:
              guiElem->setValue(QVariant((int)item->l));
              //item2->l = item->l;
              break;
            case data_broker::BOOL_TYPE:
              guiElem->setValue(QVariant(item->b));
              //item2->b = item->b;
              break;
            case data_broker::STRING_TYPE:
              guiElem->setValue(QVariant(QString::fromStdString(item->s)));
              //item2->s = item->s;
              break;
            case data_broker::UNDEFINED_TYPE:
              break;
            // don't supply a default case so that the compiler might warn
            // us if we forget to handle a new enum value.
            }
          }
        }
        changeList.erase(changeList.begin());
      }
      ignore_change = false;
      changeMutex.unlock();
    }

    void DataWidget::valueChanged(QtProperty *property, const QVariant &value) {
      if(ignore_change) return;
      map<QtVariantProperty*, paramWrapper>::iterator it;
      map<QtVariantProperty*, paramWrapper*>::iterator it3;
      double dValue;
      float fValue;
      long lValue;
      int iValue;
      bool bValue;
      string sValue;
      DataItem *item;
      //DataItem *item2;

      if(property == showAllProperty) {
        std::vector<DataInfo> infoList;
        std::vector<DataInfo>::iterator it;
        bool newShowAll = value.toBool();
        assert(newShowAll != showAll);
        dataBroker->unregisterAsyncReceiver(this, "*", "*");
        dataBroker->unregisterTimedReceiver(this, "*", "*", "_REALTIME_");
        changeMutex.lock();
        addMutex.lock();
        listMutex.lock();
        changeList.clear();
        addList.clear();
        //map<unsigned long, paramWrapper>::iterator foo;
        map<QtVariantProperty*, paramWrapper*>::iterator bar;
        for(bar = guiToWrapper.begin(); bar != guiToWrapper.end(); ++bar) {
          pDialog->removeGenericProperty(bar->first);
        }
        paramList.clear();
        guiToWrapper.clear();
        listMutex.unlock();
        addMutex.unlock();
        changeMutex.unlock();

        showAll = newShowAll;

        infoList = dataBroker->getDataList();
        for(it=infoList.begin(); it!=infoList.end(); ++it) {
          if(newShowAll || it->flags & data_broker::DATA_PACKAGE_WRITE_FLAG) {
            addParam(*it);
            dataBroker->registerTimedReceiver(this, it->groupName, it->dataName,
                                              "_REALTIME_", 250);
            //dataBroker->registerAsyncReceiver(this, it->groupName, it->dataName);
          }
        }
        return;
      }

      it3 = guiToWrapper.find((QtVariantProperty*)property);
      if(it3 != guiToWrapper.end()) {
        int idx = 0;
        std::vector<QtVariantProperty*>::iterator it2;
        for(it2 = it3->second->guiElements.begin();
            it2 != it3->second->guiElements.end() && *it2 != property; 
            ++it2, ++idx) /* do nothing */ ;
      
        item = &it3->second->dataPackage[idx];
        //item2 = &paramList[it->second.info.dataId].dataPackage[idx];

        switch(item->type) {
        case data_broker::DOUBLE_TYPE:
          dValue = value.toDouble();
          if(dValue != item->d) {
            item->d = dValue;
            //item2->d = dValue;
          }
          break;
        case data_broker::FLOAT_TYPE:
          fValue = value.toFloat();
          if(fValue != item->f) {
            item->f = fValue;
            //item2->f = fValue;
          }
          break;
        case data_broker::LONG_TYPE:
          lValue = value.toLongLong();
          if(lValue != item->l) {
            item->l = lValue;
            //item2->l = lValue;
          }
          break;
        case data_broker::INT_TYPE:
          iValue = value.toInt();
          if(iValue != item->i) {
            item->i = iValue;
            //item2->i = iValue;
          }
          break;
        case data_broker::BOOL_TYPE:
          bValue = value.toBool();
          if(bValue != item->b) {
            item->b = bValue;
            //item2->b = iValue;
          }
          break;
        case data_broker::STRING_TYPE:
          sValue = value.toString().toStdString();
          if(sValue != item->s) {
            item->s = sValue;
            //item2->s = sValue;
          }          
          break;
        case data_broker::UNDEFINED_TYPE:
          break;
        // don't supply a default case so that the compiler might warn
        // us if we forget to handle a new enum value.
        }
        dataBroker->pushData(it3->second->info.dataId,
                             it3->second->dataPackage);
      }
    }

    void DataWidget::accept() {}
    void DataWidget::reject() {}

  } // end of namespace data_broker_widget

} // end of namespace mars
