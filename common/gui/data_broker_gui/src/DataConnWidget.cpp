/**
 *  Copyright 2011, DFKI GmbH Robotics Innovation Center
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

#include "DataConnWidget.h"
#include <mars/data_broker/DataBrokerInterface.h>
//#include <QtUiTools>
#include <QHBoxLayout>
#include <QPushButton>
#include <cassert>

#define VERBOSE

namespace mars {
  namespace data_broker_gui {

    enum { CALLBACK_OTHER=0, CALLBACK_NEW_STREAM };

    DataConnWidget::DataConnWidget(mars::data_broker::DataBrokerInterface *_dataBroker, 
                                   mars::cfg_manager::CFGManagerInterface *_cfg,
                                   QWidget *parent) : 
      mars::main_gui::BaseWidget(parent, _cfg, "DataBroker_ConnectionWidget"),
      dataBroker(_dataBroker), cfg(_cfg) {
#ifdef VERBOSE
      fprintf(stderr, "\n");
#endif
      QHBoxLayout *horizontalListLayout = new QHBoxLayout();
      //fromListWidget = new QListWidget(this);
      //toListWidget = new QListWidget(this);
      //horizontalListLayout->addWidget(fromListWidget);
      //horizontalListLayout->addWidget(toListWidget);
      fromTreeWidget = new QTreeWidget(this);
      toTreeWidget = new QTreeWidget(this);
      horizontalListLayout->addWidget(fromTreeWidget);
      horizontalListLayout->addWidget(toTreeWidget);
      QVBoxLayout *verticalLayout = new QVBoxLayout();
      verticalLayout->addLayout(horizontalListLayout);
      QHBoxLayout *horizontalButtonLayout = new QHBoxLayout();
      horizontalButtonLayout->addStretch();

      connectButton = new QPushButton("connect");
      horizontalButtonLayout->addWidget(connectButton);
      horizontalButtonLayout->addStretch();
      verticalLayout->addLayout(horizontalButtonLayout);
      this->setLayout(verticalLayout);
      connect(connectButton, SIGNAL(clicked()), 
              this, SLOT(slotConnectDataItems()));

      unconnectButton = new QPushButton("unconnect");
      horizontalButtonLayout->addWidget(unconnectButton);
      //horizontalButtonLayout->addStretch();
      //verticalLayout->addLayout(horizontalButtonLayout);
      connect(unconnectButton, SIGNAL(clicked()), 
              this, SLOT(slotUnconnectDataItems()));

      QPushButton *saveButton = new QPushButton("save");
      horizontalButtonLayout->addWidget(saveButton);
      connect(saveButton, SIGNAL(clicked()), 
              this, SLOT(slotSaveConfiguration()));

   
      if(dataBroker) {
        dataBroker->registerSyncReceiver(this, "data_broker", "newStream",
                                         CALLBACK_NEW_STREAM);
        std::vector<mars::data_broker::DataInfo> infoList;
        std::vector<mars::data_broker::DataInfo>::iterator it;
        infoList = dataBroker->getDataList();
        mars::data_broker::DataPackage dbPackage;

        for(it=infoList.begin(); it!=infoList.end(); ++it) {
          dbPackage = dataBroker->getDataPackage(it->dataId);
          addDataPackage(*it, dbPackage);
        }
      }  
      connectColor = QColor(17, 110, 163);

      // load config
      if(cfg) {
        std::vector<std::string> fromDataNameList, fromGroupNameList, fromItemNameList;
        std::vector<std::string> toDataNameList, toGroupNameList, toItemNameList;
        std::string group("db_connection_widget");
        std::string configFile("db_connection_widget_default.yaml");
        PendingConnection pendingConnection;
        int num;
        bool err;
        std::string cfgString;

        cfg->loadConfig(configFile.c_str());

        if(cfg->getPropertyValue(group, "fromDataNameList", "value", &cfgString)) {
#ifdef VERBOSE
          fprintf(stderr, "DataConnWidget:fromDataNameList %s\n", cfgString.c_str());
#endif
          getCfgStringList(configFile, group, "fromDataNameList", &fromDataNameList, -1);
       
        
          if((num = fromDataNameList.size()) > 0) {
            err = !getCfgStringList(configFile, group, "fromGroupNameList", &fromGroupNameList, num);

            if(!err)
              err = !getCfgStringList(configFile, group, "fromItemNameList", &fromItemNameList, num);
            if(!err)
              err = !getCfgStringList(configFile, group, "toDataNameList", &toDataNameList, num);

            if(!err)
              err = !getCfgStringList(configFile, group, "toGroupNameList", &toGroupNameList, num);

            if(!err)
              err = !getCfgStringList(configFile, group, "toItemNameList", &toItemNameList, num);

            if(!err) {
              for(int i=0; i<num; ++i) {
                pendingConnection.fromWrapper.groupName = fromGroupNameList[i];
                pendingConnection.fromWrapper.dataName = fromDataNameList[i];
                pendingConnection.fromWrapper.itemName = fromItemNameList[i];
                pendingConnection.toWrapper.groupName = toGroupNameList[i];
                pendingConnection.toWrapper.dataName = toDataNameList[i];
                pendingConnection.toWrapper.itemName = toItemNameList[i];
                pendingConnection.fromItem = getEndTreeItem(pendingConnection.fromWrapper);
                pendingConnection.toItem = getEndTreeItem(pendingConnection.toWrapper);
                if(pendingConnection.fromItem && pendingConnection.toItem) {
                  makeConnection(pendingConnection.fromItem, pendingConnection.toItem);
                }
                else {
#ifdef VERBOSE
                  if(pendingConnection.fromItem) {
                    fprintf(stderr, "DataConnWidget::haveFromItem %s\n", makeName(pendingConnection.fromWrapper).c_str());
                  }
                  if(pendingConnection.toItem) {
                    fprintf(stderr, "DataConnWidget::haveToItem %s\n", makeName(pendingConnection.toWrapper).c_str());
                  }
                  fprintf(stderr, "DataConnWidget::addPendingConnection %s\n",
                          pendingConnection.toWrapper.itemName.c_str());
#endif
                  pendingConnections.push_back(pendingConnection);
                }
              }
            }
          }
        }
      }
    }

    DataConnWidget::~DataConnWidget(void) {
      fprintf(stderr, "\nDelete DataConnWidget\n");
      dataBroker->unregisterAsyncReceiver(this, "*", "*");
      dataBroker->unregisterTimedReceiver(this, "*", "*", "_REALTIME_");
      dataBroker->unregisterSyncReceiver(this, "data_broker", "newStream");
    }

    void DataConnWidget::slotSaveConfiguration(void) {
      // save connections in config
      if(cfg) {
        std::vector<TreeItem*>::iterator it;

        DataItemWrapper cfgWrapperFromList;
        DataItemWrapper cfgWrapperToList;
        bool first = true;
        for(it=endItems.begin(); it!=endItems.end(); ++it) {
          fprintf(stderr, ".");
          if((*it)->fromItem) {
            if(!first) {
              cfgWrapperFromList.groupName.append(", ");
              cfgWrapperFromList.dataName.append(", ");
              cfgWrapperFromList.itemName.append(", ");
              cfgWrapperToList.groupName.append(", ");
              cfgWrapperToList.dataName.append(", ");
              cfgWrapperToList.itemName.append(", ");
            }
            else first = false;
            cfgWrapperFromList.groupName.append((*it)->fromItem->wrapper->groupName);
            cfgWrapperFromList.dataName.append((*it)->fromItem->wrapper->dataName);
            cfgWrapperFromList.itemName.append((*it)->fromItem->wrapper->itemName);
            cfgWrapperToList.groupName.append((*it)->wrapper->groupName);
            cfgWrapperToList.dataName.append((*it)->wrapper->dataName);
            cfgWrapperToList.itemName.append((*it)->wrapper->itemName);
#ifdef VERBOSE
            fprintf(stderr, "current state: %s\n", cfgWrapperFromList.groupName.c_str());
#endif
          }
        }
        if(!first) {
          cfg->getOrCreateProperty("db_connection_widget", "fromGroupNameList",
                                   cfgWrapperFromList.groupName);
          cfg->setPropertyValue("db_connection_widget", "fromGroupNameList",
                                "value", cfgWrapperFromList.groupName);

          cfg->getOrCreateProperty("db_connection_widget", "fromDataNameList",
                                   cfgWrapperFromList.dataName);
          cfg->setPropertyValue("db_connection_widget", "fromDataNameList",
                                "value", cfgWrapperFromList.dataName);

          cfg->getOrCreateProperty("db_connection_widget", "fromItemNameList",
                                   cfgWrapperFromList.itemName);
          cfg->setPropertyValue("db_connection_widget", "fromItemNameList",
                                "value", cfgWrapperFromList.itemName);

          cfg->getOrCreateProperty("db_connection_widget", "toGroupNameList",
                                   cfgWrapperToList.groupName);
          cfg->setPropertyValue("db_connection_widget", "toGroupNameList",
                                "value", cfgWrapperToList.groupName);

          cfg->getOrCreateProperty("db_connection_widget", "toDataNameList",
                                   cfgWrapperToList.dataName);
          cfg->setPropertyValue("db_connection_widget", "toDataNameList",
                                "value", cfgWrapperToList.dataName);

          cfg->getOrCreateProperty("db_connection_widget", "toItemNameList",
                                   cfgWrapperToList.itemName);
          cfg->setPropertyValue("db_connection_widget", "toItemNameList",
                                "value", cfgWrapperToList.itemName);

          cfg->writeConfig("db_connection_widget_default.yaml",
                           "db_connection_widget");
        }
      }
    }

    void DataConnWidget::receiveData(const mars::data_broker::DataInfo &info,
                                     const mars::data_broker::DataPackage &dataPackage,
                                     int callbackParam) {
      //printf("receive Data\n");
      if(callbackParam == CALLBACK_NEW_STREAM) {
        std::string groupName, dataName;
        dataPackage.get("groupName", &groupName);
        dataPackage.get("dataName", &dataName);
        dataBroker->registerSyncReceiver(this, groupName, dataName);
      } else {
        dataBroker->unregisterSyncReceiver(this, info.groupName, info.dataName);

        addDataPackage(info, dataPackage);
      }
    }

    void DataConnWidget::addDataPackage(const mars::data_broker::DataInfo &info,
                                        const mars::data_broker::DataPackage &dbPackage) {

      std::map<std::string, TreeItem>::iterator treeIt;
      TreeItem *treeItem;
      std::vector<std::string> levels;
      std::vector<std::string>::iterator it;
      size_t pos, npos;

      for(unsigned int i = 0; i < dbPackage.size(); ++i) {
        std::string name = dbPackage[i].getName();

        treeIt = fromItems.find(info.groupName);
        if(treeIt == fromItems.end()) {
          fromItems[info.groupName] = TreeItem(info.groupName);
          fromTreeWidget->addTopLevelItem(&fromItems[info.groupName]);
        }
        treeItem = &fromItems[info.groupName];
      
        // split data name by /
        npos = 0;
        levels.clear();
        while((pos = info.dataName.find_first_of('/', npos)) != string::npos) {
          levels.push_back(info.dataName.substr(npos, pos-npos));
          npos = pos+1;
        }
        levels.push_back(info.dataName.substr(npos));

        for(it=levels.begin(); it!=levels.end(); ++it) {
          treeIt = treeItem->childs.find(*it);
          if(treeIt == treeItem->childs.end()) {
            treeItem->childs[*it] = TreeItem(*it);
            treeItem->addChild(&treeItem->childs[*it]);
          }
          treeItem = &treeItem->childs[*it];
        }
      
        if(dbPackage.size() > 1) {
          treeIt = treeItem->childs.find(name);
          if(treeIt == treeItem->childs.end()) {
            treeItem->childs[name] = TreeItem(name);
            treeItem->addChild(&treeItem->childs[name]);
          }
          treeItem = &treeItem->childs[name];
        }

        std::string tmp(info.groupName + "/" + info.dataName + "/" + name);
        DataItemWrapper *wrapper = new DataItemWrapper;
        wrapper->groupName = info.groupName;
        wrapper->dataName = info.dataName;
        wrapper->itemName = name;
        //fprintf(stderr, "DataConnWidget::addItem %s\n", tmp.c_str());
        treeItem->wrapper = wrapper;
        endItems.push_back(treeItem);
        checkFromPendingItem(treeItem);

        if(info.flags & mars::data_broker::DATA_PACKAGE_WRITE_FLAG) {
          treeIt = toItems.find(info.groupName);
          if(treeIt == toItems.end()) {
            toItems[info.groupName] = TreeItem(info.groupName);
            toTreeWidget->addTopLevelItem(&toItems[info.groupName]);
          }
          treeItem = &toItems[info.groupName];
        
          for(it=levels.begin(); it!=levels.end(); ++it) {
            treeIt = treeItem->childs.find(*it);
            if(treeIt == treeItem->childs.end()) {
              treeItem->childs[*it] = TreeItem(*it);
              treeItem->addChild(&treeItem->childs[*it]);
            }
            treeItem = &treeItem->childs[*it];
          }

          if(dbPackage.size() > 1) {
            treeIt = treeItem->childs.find(name);
            if(treeIt == treeItem->childs.end()) {
              treeItem->childs[name] = TreeItem(name);
              treeItem->addChild(&treeItem->childs[name]);
            }
            treeItem = &treeItem->childs[name];
          }
          treeItem->wrapper = wrapper;
          endItems.push_back(treeItem);
          checkToPendingItem(treeItem);
        }
      }
    }

    void DataConnWidget::slotConnectDataItems() {
      QList<QTreeWidgetItem*> selectedItemsFrom = fromTreeWidget->selectedItems();
      QList<QTreeWidgetItem*> selectedItemsTo = toTreeWidget->selectedItems();
      TreeItem *fromItem, *toItem;

      if(selectedItemsFrom.size() > 0 && selectedItemsTo.size() > 0) {
        fromItem = dynamic_cast<TreeItem*>(selectedItemsFrom[0]);
        toItem = dynamic_cast<TreeItem*>(selectedItemsTo[0]);
        if(fromItem && toItem) {
          makeConnection(fromItem, toItem);
        }
      }
    }

    void DataConnWidget::slotUnconnectDataItems() {
      QList<QTreeWidgetItem*> selectedItemsTo = toTreeWidget->selectedItems();
      TreeItem *toItem;

      if(selectedItemsTo.size() > 0) {
        toItem = dynamic_cast<TreeItem*>(selectedItemsTo[0]);
        if(toItem) {
          removeConnection(toItem);
        }
      }

    }

    bool DataConnWidget::getCfgStringList(std::string configFile, std::string group,
                                          std::string param,
                                          std::vector<std::string> *list,
                                          int desSize) {
      std::string cfgString;
      cfg->getPropertyValue(group, param, "value", &cfgString);
      int npos = 0;
      int pos;
      while((pos = cfgString.find_first_of(',', npos)) != string::npos) {
        list->push_back(cfgString.substr(npos, pos-npos));
        npos = pos+2;
      }
      list->push_back(cfgString.substr(npos));
      if(desSize >= 0 && (int)list->size() != desSize) {
        fprintf(stderr, "DataConnWidget: Error while parsing config file! Skip %s", configFile.c_str());
        return false;
      }
      return true;
    }


    void DataConnWidget::makeConnection(TreeItem *fromItem, TreeItem *toItem) {
      if(toItem->fromItem) {
        removeConnection(toItem);
      }
      printf("DataConnWidget::makeConnection: %s -> %s\n", makeName(*(fromItem->wrapper)).c_str(),
             makeName(*(toItem->wrapper)).c_str());        
      dataBroker->connectDataItems(fromItem->wrapper->groupName,
                                   fromItem->wrapper->dataName,
                                   fromItem->wrapper->itemName,
                                   toItem->wrapper->groupName,
                                   toItem->wrapper->dataName,
                                   toItem->wrapper->itemName);
      std::string newText = toItem->wrapper->itemName;
      newText.append(" [");
      newText.append(fromItem->wrapper->itemName);
      newText.append("]");
      toItem->setText(0, QString::fromStdString(newText));
      toItem->setForeground(0, QBrush(connectColor));
      fromItem->setForeground(0, QBrush(connectColor));
      toItem->fromItem = fromItem;
      fromItem->outCount++;
    }

    void DataConnWidget::removeConnection(TreeItem *toItem) {
      printf("DataConnWidget::removeConnection: %s\n", makeName(*(toItem->wrapper)).c_str());        

      dataBroker->disconnectDataItems(toItem->wrapper->groupName,
                                      toItem->wrapper->dataName,
                                      toItem->wrapper->itemName);
        
      toItem->fromItem->outCount--;
      if(toItem->fromItem->outCount == 0) {
        toItem->fromItem->setText(0, QString::fromStdString(toItem->fromItem->wrapper->itemName));
        toItem->fromItem->setForeground(0, QBrush(QColor(0, 0, 0)));
      }
      toItem->setText(0, QString::fromStdString(toItem->wrapper->itemName));
      toItem->setForeground(0, QBrush(QColor(0, 0, 0)));
    }
  
    TreeItem* DataConnWidget::getEndTreeItem(const DataItemWrapper &wrapper) {
      std::vector<TreeItem*>::iterator it;
    
      for(it=endItems.begin(); it!=endItems.end(); ++it) {
        if(wrapperEqual((*it)->wrapper, &wrapper)) {
          return *it;
        }
      }
      return NULL;
    }

    void DataConnWidget::checkFromPendingItem(TreeItem *treeItem) {
      std::vector<PendingConnection>::iterator it;
      std::vector<PendingConnection> copy;

      for(it=pendingConnections.begin(); it!=pendingConnections.end(); ++it) {
        if(wrapperEqual(&(it->fromWrapper), treeItem->wrapper)) {
          it->fromItem = treeItem;
#ifdef VERBOSE
          fprintf(stderr, "DataConnWidget::checkFromPendingItem found %s\n", makeName(it->fromWrapper).c_str());
#endif
          if(it->fromItem && it->toItem) {
            makeConnection(it->fromItem, it->toItem);
          }
          else copy.push_back(*it);
        }
        else copy.push_back(*it);      
      }
      copy.swap(pendingConnections);
    }

    void DataConnWidget::checkToPendingItem(TreeItem *treeItem) {
      std::vector<PendingConnection>::iterator it;
      std::vector<PendingConnection> copy;

      for(it=pendingConnections.begin(); it!=pendingConnections.end(); ++it) {
        if(wrapperEqual(&(it->toWrapper), treeItem->wrapper)) {
          it->toItem = treeItem;
#ifdef VERBOSE
          fprintf(stderr, "DataConnWidget::checkToPendingItem found %s\n", makeName(it->toWrapper).c_str());
#endif
          if(it->fromItem && it->toItem) {
            makeConnection(it->fromItem, it->toItem);
          }
          else copy.push_back(*it);
        }
        else copy.push_back(*it);      
      }
      copy.swap(pendingConnections);
    }

    void DataConnWidget::accept() {}
    void DataConnWidget::reject() {}

  } // end of namespace: data_broker_gui
} // end of namespace: mars
