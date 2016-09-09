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

#include <mars/utils/misc.h>

#include <QVBoxLayout>
#include <cassert>

namespace mars {

  using namespace configmaps;

  namespace config_map_gui {


    DataWidget::DataWidget(cfg_manager::CFGManagerInterface *cfg,
                           QWidget *parent, bool onlyCompactView) :
      main_gui::BaseWidget(parent, cfg, "ConfigMapWidget"),
      pDialog(new main_gui::PropertyDialog(parent)),
      ignore_change(0) {

      startTimer(500);

      QVBoxLayout *vLayout = new QVBoxLayout();
      vLayout->addWidget(pDialog);
      setLayout(vLayout);

      addProperty = 0;
      if(onlyCompactView) {
        pDialog->setViewButtonVisibility(false);
        pDialog->setViewMode(main_gui::TreeViewMode);
      }
      pDialog->setButtonBoxVisibility(true);
      pDialog->clearButtonBox();
      pDialog->addGenericButton("add key", this, SLOT(addKey()));
      pDialog->setPropCallback(dynamic_cast<main_gui::PropertyCallback*>(this));
    }

    DataWidget::~DataWidget(void) {
    }

    void DataWidget::setConfigMap(const std::string &name,
                                  const ConfigMap &map) {
      std::vector<std::string> pattern;
      setConfigMap(name, map, pattern);
    }

    void DataWidget::setConfigMap(const std::string &name,
                                  const ConfigMap &map,
                                  const std::vector<std::string> &pattern) {
      editPattern = pattern;
      ignore_change = 1;
      clearGUI();
      if(&config != &map) {
        config = map;
        cname = name;
      }
      std::string n = name;
      for(size_t i=0; i<n.size(); ++i) {
        if(n[i] == '/') {
          n.insert(n.begin()+i, '/');
          ++i;
        }
      }
      // config.toYamlFile("foo.yml");
      std::string path = "..";
      if(!n.empty()) {
        path += "/" + n;
      }
      addConfigMap(path, config);
      ignore_change = 0;
    }

    void DataWidget::addConfigMap(const std::string &name,
                               ConfigMap &map) {
      ConfigMap::iterator it = map.begin();
      QtVariantProperty *tmp =
        pDialog->addGenericProperty(name,
                                    QtVariantPropertyManager::groupTypeId(),
                                    0);
      for(;it!=map.end(); ++it) {
        std::string n = it->first;
        for(size_t i=0; i<n.size(); ++i) {
          if(n[i] == '/') {
            n.insert(n.begin()+i, '/');
            ++i;
          }
        }

        if(it->second.isVector()) {
          addConfigVector(name+"/"+n, it->second);
        }
        else if(it->second.isMap()) {
          addConfigMap(name+"/"+n, it->second);
        }
        else if(it->second.isAtom()) {
          addConfigAtom(name+"/"+n, it->second);
        }
      }
      QtVariantProperty *guiElem;
      guiElem = pDialog->addGenericProperty(name+"/add key",
                                            QVariant::String,
                                            "");
      addMap[guiElem] = &map;
      pDialog->expandTree(tmp);
    }

    void DataWidget::addConfigVector(const std::string &name,
                                     ConfigVector &v) {
      for(size_t i=0; i<v.size(); ++i) {
        char iText[64];
        iText[0] = '\0';
        sprintf(iText, "/%d", (int)i);
        if(v[i].isAtom()) addConfigAtom(name + iText, v[i]);
        else if(v[i].isVector()) addConfigVector(name + iText, v[i]);
        else if(v[i].isMap()) addConfigMap(name + iText, v[i]);
      }
    }

    void DataWidget::addConfigAtom(const std::string &name,
                                   ConfigAtom &v) {
      QtVariantProperty *guiElem;
      std::map<QString, QVariant> attr;
      attr["singleStep"] = 0.01;
      attr["decimals"] = 7;
      ConfigAtom::ItemType type = v.getType();
      if(propMap.find(name) == propMap.end()) {
        bool editable = true;
        if(editPattern.size() > 0) {
          editable = false;
          for(size_t i=0; i<editPattern.size(); ++i) {
            if(utils::matchPattern(editPattern[i], name)) {
              editable = true;
              break;
            }
          }
        }
        if(type == ConfigAtom::UNDEFINED_TYPE) {
          guiElem = pDialog->addGenericProperty(name,
                                                QVariant::String,
                                                QString::fromStdString(v.getUnparsedString()),
                                                &attr);
        }
        else if(type == ConfigAtom::STRING_TYPE) {
          guiElem = pDialog->addGenericProperty(name,
                                                QVariant::String,
                                                QString::fromStdString(v.getString()),
                                                &attr);
              }
        else if(type == ConfigAtom::INT_TYPE) {
          guiElem = pDialog->addGenericProperty(name,
                                                QVariant::Int,
                                                v.getInt(),
                                                &attr);
        }
        else if(type == ConfigAtom::UINT_TYPE) {
          guiElem = pDialog->addGenericProperty(name,
                                                QVariant::Int,
                                                (int)v.getUInt(),
                                                &attr);
        }
        else if(type == ConfigAtom::DOUBLE_TYPE) {
          guiElem = pDialog->addGenericProperty(name,
                                                QVariant::Double,
                                                v.getDouble(),
                                                &attr);
        }
        else if(type == ConfigAtom::ULONG_TYPE) {
          guiElem = pDialog->addGenericProperty(name,
                                                QVariant::Int,
                                                (int)v.getULong(),
                                                &attr);
        }
        else if(type == ConfigAtom::BOOL_TYPE) {
          guiElem = pDialog->addGenericProperty(name,
                                                QVariant::Bool,
                                                v.getBool(),
                                                &attr);
        }
        guiElem->setEnabled(editable);
        dataMap[guiElem] = &v;
        propMap[name] = guiElem;
        nameMap[guiElem] = name;
      }
    }

    void DataWidget::updateConfigMap(const std::string &name,
                                     const ConfigMap &map) {
      ignore_change = 1;
      ConfigMap tmpMap = map;
      std::string path = "..";
      std::string n = name;
      for(size_t i=0; i<n.size(); ++i) {
        if(n[i] == '/') {
          n.insert(n.begin()+i, '/');
          ++i;
        }
      }
      if(!n.empty()) {
        path += "/" + n;
      }
      updateConfigMapI(path, tmpMap);
      ignore_change = 0;
    }

    void DataWidget::updateConfigMapI(const std::string &name,
                                      ConfigMap &map) {
      ConfigMap::const_iterator it = map.begin();
      std::string n;
      for(;it!=map.end(); ++it) {
        n = it->first;
        for(size_t i=0; i<n.size(); ++i) {
          if(n[i] == '/') {
            n.insert(n.begin()+i, '/');
            ++i;
          }
        }

        if(it->second.isAtom()) {
          updateConfigAtomI(name+"/"+n, it->second);
        }
        else if(it->second.isVector()) {
          updateConfigVectorI(name+"/"+n, it->second);
        }
        else if(it->second.isMap()) {
          updateConfigMapI(name+"/"+n, it->second);
        }
      }
    }

    void DataWidget::updateConfigVectorI(const std::string &name,
                                         ConfigVector &v) {

      for(size_t i=0; i<v.size(); ++i) {
        char iText[64];
        iText[0] = '\0';
        sprintf(iText, "/%d", (int)i);
        if(v[i].isAtom()) updateConfigAtomI(name + iText, v[i]);
        else if(v[i].isVector()) updateConfigVectorI(name + iText, v[i]);
        else if(v[i].isMap()) updateConfigMapI(name + iText, v[i]);
      }
    }

    void DataWidget::updateConfigAtomI(const std::string &name,
                                       ConfigAtom &v) {

      QtVariantProperty *guiElem;
      std::map<QString, QVariant> attr;
      attr["singleStep"] = 0.01;
      attr["decimals"] = 7;
      ConfigAtom atom = v;
      ConfigAtom::ItemType type = atom.getType();
      if(propMap.find(name) != propMap.end()) {
        guiElem = propMap[name];
        *(dataMap[guiElem]) = v;
        if(type == ConfigAtom::UNDEFINED_TYPE) {
          guiElem->setValue(QVariant(QString::fromStdString(atom.getUnparsedString())));
        }
        else if(type == ConfigAtom::STRING_TYPE) {
          guiElem->setValue(QVariant(QString::fromStdString(atom.getString())));
        }
        else if(type == ConfigAtom::INT_TYPE) {
          guiElem->setValue(QVariant(atom.getInt()));
        }
        else if(type == ConfigAtom::UINT_TYPE) {
          guiElem->setValue(QVariant(atom.getUInt()));
        }
        else if(type == ConfigAtom::DOUBLE_TYPE) {
          guiElem->setValue(QVariant(atom.getDouble()));
        }
        else if(type == ConfigAtom::ULONG_TYPE) {
          guiElem->setValue(QVariant((int)atom.getULong()));
        }
        else if(type == ConfigAtom::BOOL_TYPE) {
          guiElem->setValue(QVariant(atom.getBool()));
        }
      }
    }

    void DataWidget::clearGUI() {
      map<QtVariantProperty*, ConfigAtom*>::iterator it;
      map<QtVariantProperty*, ConfigMap*>::iterator it2;
      for(it=dataMap.begin(); it!=dataMap.end(); ++it) {
        pDialog->removeGenericProperty(it->first);
      }
      for(it2=addMap.begin(); it2!=addMap.end(); ++it2) {
        pDialog->removeGenericProperty(it2->first);
      }
      dataMap.clear();
      nameMap.clear();
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
        map<QtVariantProperty*, ConfigAtom*>::iterator it;
        it = dataMap.find((QtVariantProperty*)property);
        if(it != dataMap.end()) {
          ConfigAtom::ItemType type = it->second->getType();
          if(type == ConfigAtom::UNDEFINED_TYPE) {
            it->second->setUnparsedString(value.toString().toStdString());
          }
          else if(type == ConfigAtom::STRING_TYPE) {
            it->second->setString(value.toString().toStdString());
          }
          else if(type == ConfigAtom::INT_TYPE) {
            it->second->setInt(value.toInt());
          }
          else if(type == ConfigAtom::UINT_TYPE) {
            it->second->setUInt(value.toUInt());
          }
          else if(type == ConfigAtom::DOUBLE_TYPE) {
            it->second->setDouble(value.toDouble());
          }
          else if(type == ConfigAtom::ULONG_TYPE) {
            it->second->setULong(value.toULongLong());
          }
          else if(type == ConfigAtom::BOOL_TYPE) {
            it->second->setBool(value.toBool());
          }
        }
        emit valueChanged(nameMap[(QtVariantProperty*)property], it->second->toString());
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
