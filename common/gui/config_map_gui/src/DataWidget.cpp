/*
 *  Copyright 20114, DFKI GmbH Robotics Innovation Center
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

#include <QVBoxLayout>

#include <cassert>

namespace mars {

  using namespace utils;
  using namespace configmaps;

  namespace config_map_gui {


    DataWidget::DataWidget(cfg_manager::CFGManagerInterface *cfg,
                           QWidget *parent) :
      main_gui::BaseWidget(parent, cfg, "ConfigMapWidget"),
      pDialog(new main_gui::PropertyDialog(parent)),
      ignore_change(0) {

      startTimer(500);

      QVBoxLayout *vLayout = new QVBoxLayout();
      vLayout->addWidget(pDialog);
      setLayout(vLayout);
      
      addProperty = 0;
      pDialog->setButtonBoxVisibility(true);
      pDialog->clearButtonBox();
      pDialog->addGenericButton("add key", this, SLOT(addKey()));
      pDialog->setPropCallback(dynamic_cast<main_gui::PropertyCallback*>(this));
    }

    DataWidget::~DataWidget(void) {
    }

    void DataWidget::setConfigMap(const std::string &name,
                                  const ConfigMap &map) {
      ignore_change = 1;
      clearGUI();
      if(&config != &map) {
        config = map;
        cname = name;
      }
      config.toYamlFile("foo.yml");
      addConfigMap(name, config);
      ignore_change = 0;
    }

    void DataWidget::addConfigMap(const std::string &name,
                               ConfigMap &map) {
      QtVariantProperty *guiElem;
      ConfigMap::iterator it = map.begin();
      for(;it!=map.end(); ++it) {
        for(size_t i=0; i<it->second.size(); ++i) {
          char iText[64];
          iText[0] = '\0';
          if(it->second.size() > 1) {
            sprintf(iText, "/%d", (int)i);
          }
          if(it->second[i].children.size() > 0) {
            addConfigMap(name+"/"+it->first+iText, it->second[i].children);
          }
          else {
            std::map<QString, QVariant> attr;
            attr["singleStep"] = 0.01;
            attr["decimals"] = 7;
            ConfigItem *item = &(it->second[i]);
            ConfigItem::ItemType type = item->getType();
            std::string propName = name+"/"+it->first+iText;

            if(propMap.find(propName) == propMap.end()) {
              if(type == ConfigItem::UNDEFINED_TYPE) {
                guiElem = pDialog->addGenericProperty(propName,
                                                      QVariant::String,
                                                      QString::fromStdString(item->getUnparsedString()),
                                                      &attr);
              }
              else if(type == ConfigItem::STRING_TYPE) {
                guiElem = pDialog->addGenericProperty(propName,
                                                      QVariant::String,
                                                      QString::fromStdString(item->getString()),
                                                      &attr);
              }
              else if(type == ConfigItem::INT_TYPE) {
                guiElem = pDialog->addGenericProperty(propName,
                                                      QVariant::Int,
                                                      item->getInt(),
                                                      &attr);
              }
              else if(type == ConfigItem::UINT_TYPE) {
                guiElem = pDialog->addGenericProperty(propName,
                                                      QVariant::Int,
                                                      (int)item->getUInt(),
                                                      &attr);
              }
              else if(type == ConfigItem::DOUBLE_TYPE) {
              guiElem = pDialog->addGenericProperty(propName,
                                                    QVariant::Double,
                                                    item->getDouble(),
                                                    &attr);
              }
              else if(type == ConfigItem::ULONG_TYPE) {
                guiElem = pDialog->addGenericProperty(propName,
                                                      QVariant::Int,
                                                      (int)item->getULong(),
                                                      &attr);
              }
              else if(type == ConfigItem::BOOL_TYPE) {
                guiElem = pDialog->addGenericProperty(propName,
                                                      QVariant::Bool,
                                                      item->getBool(),
                                                      &attr);
              }
              dataMap[guiElem] = item;
              propMap[propName] = guiElem;
            }
          }
          
        }
      }
      guiElem = pDialog->addGenericProperty(name+"/add key",
                                            QVariant::String,
                                            "");
      addMap[guiElem] = &map;
    }

    void DataWidget::updateConfigMap(const std::string &name,
                                     const ConfigMap &map) {
      ignore_change = 1;
      updateConfigMapI(name, map);
      ignore_change = 0;
    }

    void DataWidget::updateConfigMapI(const std::string &name,
                                      const ConfigMap &map) {
      ConfigMap::const_iterator it = map.begin();
      for(;it!=map.end(); ++it) {
        for(size_t i=0; i<it->second.size(); ++i) {
          char iText[64];
          iText[0] = '\0';
          if(it->second.size() > 1) {
            sprintf(iText, "/%d", (int)i);
          }
          if(it->second[i].children.size() > 0) {
            updateConfigMapI(name+"/"+it->first+iText, it->second[i].children);
          }
          else {
            std::map<QString, QVariant> attr;
            attr["singleStep"] = 0.01;
            attr["decimals"] = 7;
            QtVariantProperty *guiElem;
            ConfigItem *item = &(it->second[i]);
            ConfigItem::ItemType type = item->getType();
            std::string propName = name+"/"+it->first+iText;

            if(propMap.find(propName) != propMap.end()) {
              guiElem = propMap[propName];
              *(dataMap[guiElem]) = *item;
              if(type == ConfigItem::UNDEFINED_TYPE) {
                guiElem->setValue(QVariant(QString::fromStdString(item->getUnparsedString())));
              }
              else if(type == ConfigItem::STRING_TYPE) {
                guiElem->setValue(QVariant(QString::fromStdString(item->getString())));
              }
              else if(type == ConfigItem::INT_TYPE) {
                guiElem->setValue(QVariant(item->getInt()));
              }
              else if(type == ConfigItem::UINT_TYPE) {
                guiElem->setValue(QVariant(item->getUInt()));
              }
              else if(type == ConfigItem::DOUBLE_TYPE) {
                guiElem->setValue(QVariant(item->getDouble()));
              }
              else if(type == ConfigItem::ULONG_TYPE) {
                guiElem->setValue(QVariant((int)item->getULong()));
              }
              else if(type == ConfigItem::BOOL_TYPE) {
                guiElem->setValue(QVariant(item->getBool()));
              }              
            }
          }
        }
      }
    }

    void DataWidget::clearGUI() {
      map<QtVariantProperty*, ConfigItem*>::iterator it;
      map<QtVariantProperty*, ConfigMap*>::iterator it2;
      for(it=dataMap.begin(); it!=dataMap.end(); ++it) {
        pDialog->removeGenericProperty(it->first);
      }
      for(it2=addMap.begin(); it2!=addMap.end(); ++it2) {
        pDialog->removeGenericProperty(it2->first);        
      }
      dataMap.clear();
      propMap.clear();
      addMap.clear();
      addProperty = 0;
    }

    const ConfigMap& DataWidget::getConfigMap() {
      return config;
    }

    void DataWidget::timerEvent(QTimerEvent* event) {
      (void)event;
    }

    void DataWidget::valueChanged(QtProperty *property, const QVariant &value) {
      if(ignore_change) return;

      {
        map<QtVariantProperty*, ConfigItem*>::iterator it;
        it = dataMap.find((QtVariantProperty*)property);
        if(it != dataMap.end()) {
          ConfigItem::ItemType type = it->second->getType();
          if(type == ConfigItem::UNDEFINED_TYPE) {
            it->second->setUnparsedString(value.toString().toStdString());
          }
          else if(type == ConfigItem::STRING_TYPE) {
            it->second->setString(value.toString().toStdString());
          }
          else if(type == ConfigItem::INT_TYPE) {
            it->second->setInt(value.toInt());
          }
          else if(type == ConfigItem::UINT_TYPE) {
            it->second->setUInt(value.toUInt());
          }
          else if(type == ConfigItem::DOUBLE_TYPE) {
            it->second->setDouble(value.toDouble());
          }
          else if(type == ConfigItem::ULONG_TYPE) {
            it->second->setULong(value.toULongLong());
          }
          else if(type == ConfigItem::BOOL_TYPE) {
            it->second->setBool(value.toBool());
          }
        }
      }
      {
        map<QtVariantProperty*, ConfigMap*>::iterator it;
        it = addMap.find((QtVariantProperty*)property);
        if(it != addMap.end()) {
          addKeyStr = value.toString().toStdString();
          addProperty = it->first;
        }
      }
      emit mapChanged();
    }

    void DataWidget::addKey() {
      if(addProperty) {
        map<QtVariantProperty*, ConfigMap*>::iterator it;
        it = addMap.find(addProperty);
        if(it != addMap.end()) {
          (*it->second)[addKeyStr] = "";
          addProperty = 0;
        }
        setConfigMap(cname, config);
        emit mapChanged();
      }
    }

    void DataWidget::accept() {}
    void DataWidget::reject() {}

  } // end of namespace config_map_widget

} // end of namespace mars
