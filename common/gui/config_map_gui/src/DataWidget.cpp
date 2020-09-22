/*
 *  Copyright 2014, 2017, DFKI GmbH Robotics Innovation Center
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
#include <QLabel>
#include <QPushButton>
#include <cassert>

namespace mars {

  using namespace configmaps;

  namespace config_map_gui {


    DataWidget::DataWidget(void* backwardCFG, QWidget *parent, bool onlyCompactView,
                           bool allowAdd) :
      QWidget(parent),
      pDialog(new main_gui::PropertyDialog(parent)),
      ignore_change(0) {

      (void)backwardCFG; // prevent paramter unsed warning
      startTimer(500);

      QVBoxLayout *vLayout = new QVBoxLayout();
      vLayout->addWidget(pDialog);
      vLayout->setContentsMargins(0,0,0,0);
      if(allowAdd) {
        QHBoxLayout *hLayout = new QHBoxLayout();
        QLabel *label = new QLabel("type:");
        typeBox = new QComboBox();
        typeBox->addItem("Map");
        typeBox->addItem("Vector");
        typeBox->addItem("Item");
        hLayout->addWidget(label);
        hLayout->addWidget(typeBox);
        vLayout->addLayout(hLayout);
        hLayout = new QHBoxLayout();
        keyEdit = new QLineEdit();
        valueEdit = new QLineEdit();
        hLayout->addWidget(keyEdit);
        hLayout->addWidget(valueEdit);
        QPushButton *button = new QPushButton("add");
        connect(button, SIGNAL(clicked()), this, SLOT(addKey2()));
        hLayout->addWidget(button);
        hLayout->setSpacing(4);
        vLayout->addLayout(hLayout);
      }
      vLayout->setSpacing(0);
      setLayout(vLayout);

      addProperty = 0;
      if(onlyCompactView) {
        pDialog->setViewButtonVisibility(false);
        pDialog->setViewMode(main_gui::TreeViewMode);
      }
      pDialog->setButtonBoxVisibility(true);
      pDialog->clearButtonBox();
      //pDialog->addGenericButton("add key", this, SLOT(addKey()));
      pDialog->setPropCallback(dynamic_cast<main_gui::PropertyCallback*>(this));
    }

    DataWidget::~DataWidget(void) {
    }

    void DataWidget::setEditPattern(const std::vector<std::string> &pattern) {
      editPattern = pattern;
    }

    void DataWidget::setColorPattern(const std::vector<std::string> &pattern) {
      colorPattern = pattern;
    }

    void DataWidget::setFilePattern(const std::vector<std::string> &pattern) {
      filePattern = pattern;
    }

    void DataWidget::setDropDownPattern(const std::vector<std::string> &pattern,
                                        const std::vector<std::vector<std::string> > &values) {
      dropDownPattern = pattern;
      dropDownValues = values;
    }

    void DataWidget::setCheckablePattern(const std::vector<std::string> &pattern) {
      checkablePattern = pattern;
    }

    void DataWidget::setFilterPattern(const std::vector<std::string> &pattern) {
      filterPattern = pattern;
    }

    void DataWidget::setBlackFilterPattern(const std::vector<std::string> &pattern) {
      blackFilterPattern = pattern;
    }

    void DataWidget::setConfigMap(const std::string &name,
                                  const ConfigMap &map) {
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
      QtVariantProperty *guiElem;
      if(checkInPattern(name, colorPattern)) {
        QColor c;
        c.setRgbF(map["r"],map["g"], map["b"], map["a"]);
        guiElem = pDialog->addGenericProperty(name, QVariant::Color, c);
        //addMap[tmp] = &map;
        colorMap[guiElem] = &map;
        nameMap[guiElem] = name;
        propMap[name] = guiElem;
        pDialog->expandTree(guiElem);
        return;
      }
      if(checkInPattern(name, checkablePattern)) {
        guiElem = pDialog->addGenericProperty(name,
                                              QVariant::Bool,
                                              0);
        checkMap[guiElem] = name;
      }
      else {
        guiElem = pDialog->addGenericProperty(name,
                                              QtVariantPropertyManager::groupTypeId(),
                                              0);
      }

      for(;it!=map.end(); ++it) {
        std::string n = it->first;
        if(!blackFilterPattern.empty() && checkInPattern(n, blackFilterPattern)) {
          continue;
        }
        if(filterPattern.empty() || checkInPattern(n, filterPattern)) {
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
      }
      nameMap[guiElem] = name;
      addMap[guiElem] = &map;
      pDialog->expandTree(guiElem);
    }

    void DataWidget::addConfigVector(const std::string &name,
                                     ConfigVector &v) {
      QtVariantProperty *guiElem;
      if(checkInPattern(name, checkablePattern)) {
        guiElem = pDialog->addGenericProperty(name,
                                              QVariant::Bool,
                                              0);
        checkMap[guiElem] = name;
      }
      else {
        guiElem = pDialog->addGenericProperty(name,
                                              QtVariantPropertyManager::groupTypeId(),
                                              0);
      }
      addVector[guiElem] = &v;
      for(unsigned long i=0; i<(unsigned long)v.size(); ++i) {
        char iText[64];
        iText[0] = '\0';
        sprintf(iText, "/%d", (int)i);
        if(v[i].isAtom()) addConfigAtom(name + iText, v[i]);
        else if(v[i].isVector()) addConfigVector(name + iText, v[i]);
        else if(v[i].isMap()) addConfigMap(name + iText, v[i]);
        else {
          guiElem = pDialog->addGenericProperty(name+iText,
                                                QtVariantPropertyManager::groupTypeId(),
                                                0);
        }
      }
    }

    int DataWidget::checkInPattern(const std::string &v,
                                    const std::vector<std::string> &pattern) {
      std::vector<std::string>::const_iterator it = pattern.begin();
      int i=0;
      for(; it!=pattern.end(); ++it, ++i) {
        //fprintf(stderr, "check: %s pattern: %s\n", v.c_str(), it->c_str());
        if(utils::matchPattern(*it, v)) {
          return i+1;
        }
      }
      return 0;
    }

    void DataWidget::addConfigAtom(const std::string &name,
                                   ConfigAtom &v) {
      QtVariantProperty *guiElem;
      std::map<QString, QVariant> attr;
      ConfigAtom::ItemType type = v.getType();
      if(propMap.find(name) == propMap.end()) {
        bool editable = true;
        if(!editPattern.empty()) {
          editable = checkInPattern(name, editPattern);
        }
        if(type == ConfigAtom::UNDEFINED_TYPE) {
          int type = QVariant::String;
          if(checkInPattern(name, filePattern)) {
            type = VariantManager::filePathTypeId();
            attr["directory"] = ".";
          }
          int index = 0;
          if((index = checkInPattern(name, dropDownPattern))) {
            index -= 1;
            QStringList enumNames;
            std::string value = v.getUnparsedString();
            int index2 = 0;
            int i=0;
            for(std::vector<std::string>::iterator it=dropDownValues[index].begin();
                it!=dropDownValues[index].end(); ++it, ++i) {
              enumNames << it->c_str();
              if(value == *it) {
                index2 = i;
              }
            }
            guiElem = pDialog->addGenericProperty(name,
                                                  QtVariantPropertyManager::enumTypeId(),
                                                  index2, NULL, &enumNames);
          }
          else {
            guiElem = pDialog->addGenericProperty(name, type,
                                                  QString::fromStdString(v.getUnparsedString()),
                                                  &attr);
          }
        }
        else if(type == ConfigAtom::STRING_TYPE) {
          int type = QVariant::String;
          if(checkInPattern(name, filePattern)) {
            type = VariantManager::filePathTypeId();
            attr["directory"] = ".";
          }
          int index;
          if((index = checkInPattern(name, dropDownPattern))) {
            index -= 1;
            QStringList enumNames;
            std::string value = v.getString();
            int index2 = 0;
            int i=0;
            for(std::vector<std::string>::iterator it=dropDownValues[index].begin();
                it!=dropDownValues[index].end(); ++it, ++i) {
              enumNames << it->c_str();
              if(value == *it) {
                index2 = i;
              }
            }
            guiElem = pDialog->addGenericProperty(name,
                                                  QtVariantPropertyManager::enumTypeId(),
                                                  index2, NULL, &enumNames);
          }
          else {
            guiElem = pDialog->addGenericProperty(name, type,
                                                  QString::fromStdString(v.getString()),
                                                  &attr);
          }
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
          attr["singleStep"] = 0.01;
          attr["decimals"] = 7;
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
      // todo: hande color itmes
      if(checkInPattern(name, colorPattern)) {
        std::map<std::string, QtVariantProperty*>::iterator it = propMap.find(name);
        if(it != propMap.end()) {
          ConfigItem *item = getItem(name);
          if(!item) return;
          ConfigMap &cMap = *item;
          cMap = map;
          QColor c;
          c.setRgbF(map["r"],map["g"], map["b"], map["a"]);
          it->second->setValue(c);
        }
        else { // add the element
          ConfigItem *item = getItem(name);
          if(!item) return;
          *item = map;
          addConfigMap(name, map);
        }
        return;
      }
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

      for(unsigned long i=0; i<(unsigned long)v.size(); ++i) {
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

      if(propMap.find(name) != propMap.end()) {
        QtVariantProperty *guiElem;
        ConfigAtom atom = v;
        ConfigAtom::ItemType type = atom.getType();
        guiElem = propMap[name];
        //*(dataMap[guiElem]) = v;

        ConfigAtom *atom_ = getAtom(name);
        if(!atom_) return;
        *atom_ = v;

        // todo: handle dropDownPattern
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
      else { // add the element
        ConfigAtom *atom = getAtom(name);
        if(!atom) return;
        *atom = v;
        addConfigAtom(name, *atom);
      }
    }

    void DataWidget::clearGUI() {
      map<QtVariantProperty*, ConfigAtom*>::iterator it;
      map<QtVariantProperty*, ConfigMap*>::iterator it2;
      for(it=dataMap.begin(); it!=dataMap.end(); ++it) {
        pDialog->removeGenericProperty(it->first);
      }
      for(it2=colorMap.begin(); it2!=colorMap.end(); ++it2) {
        pDialog->removeGenericProperty(it2->first);
      }
      for(it2=addMap.begin(); it2!=addMap.end(); ++it2) {
        pDialog->removeGenericProperty(it2->first);
      }
      dataMap.clear();
      colorMap.clear();
      nameMap.clear();
      propMap.clear();
      addMap.clear();
      checkMap.clear();
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
        // check for color property
        QtVariantProperty *vp = dynamic_cast<QtVariantProperty*>(property);
        if(vp->valueType() == QVariant::Color) {
          map<QtVariantProperty*, ConfigMap*>::iterator it;
          it = colorMap.find(vp);
          if(it != colorMap.end()) {
            std::string n = nameMap[vp];
            QColor c = value.value<QColor>();
            ConfigItem *item = getItem(n);
            (*item)["r"] = c.redF();
            (*item)["g"] = c.greenF();
            (*item)["b"] = c.blueF();
            (*item)["a"] = c.alphaF();
            // emit colorChanged(nameMap[vp], c.redF(), c.greenF(),
            //                   c.blueF(), c.alphaF());
            emit valueChanged(n+"/r", (*item)["r"].toString());
            emit valueChanged(n+"/g", (*item)["g"].toString());
            emit valueChanged(n+"/b", (*item)["b"].toString());
            emit valueChanged(n+"/a", (*item)["a"].toString());
          }
        }
        else if(vp->propertyType() == QtVariantPropertyManager::enumTypeId()) {
          std::string n = nameMap[(QtVariantProperty*)property];
          map<QtVariantProperty*, ConfigAtom*>::iterator it;
          it = dataMap.find((QtVariantProperty*)property);
          if(it != dataMap.end()) {
            ConfigAtom *atom = getAtom(n);
            if(!atom) return;
            ConfigAtom::ItemType type = atom->getType();
            if(type == ConfigAtom::UNDEFINED_TYPE) {
              atom->setUnparsedString(property->valueText().toStdString());
            }
            else if(type == ConfigAtom::STRING_TYPE) {
              atom->setString(property->valueText().toStdString());
            }
            emit valueChanged(n, atom->toString());
          }
        }
        else {
          std::string n = nameMap[(QtVariantProperty*)property];
          map<QtVariantProperty*, ConfigAtom*>::iterator it;
          it = dataMap.find((QtVariantProperty*)property);
          if(it != dataMap.end()) {
            ConfigAtom *atom = getAtom(n);
            if(!atom) return;
            ConfigAtom::ItemType type = atom->getType();
            if(type == ConfigAtom::UNDEFINED_TYPE) {
              atom->setUnparsedString(value.toString().toStdString());
            }
            else if(type == ConfigAtom::STRING_TYPE) {
              atom->setString(value.toString().toStdString());
            }
            else if(type == ConfigAtom::INT_TYPE) {
              atom->setInt(value.toInt());
            }
            else if(type == ConfigAtom::UINT_TYPE) {
              atom->setUInt(value.toUInt());
            }
            else if(type == ConfigAtom::DOUBLE_TYPE) {
              atom->setDouble(value.toDouble());
            }
            else if(type == ConfigAtom::ULONG_TYPE) {
              atom->setULong(value.toULongLong());
            }
            else if(type == ConfigAtom::BOOL_TYPE) {
              atom->setBool(value.toBool());
            }
            emit valueChanged(n, atom->toString());
          }
          else {
            map<QtVariantProperty*, std::string>::iterator it;
            it = checkMap.find((QtVariantProperty*)property);
            if(it != checkMap.end()) {
              emit checkChanged(it->second, value.toBool());
            }
          }
        }
      }
      /*
      {
        map<QtVariantProperty*, ConfigMap*>::iterator it;
        it = addMap.find((QtVariantProperty*)property);
        if(it != addMap.end()) {
          addKeyStr = value.toString().toStdString();
          addProperty = it->first;
        }
      }
      */
      emit mapChanged();
    }

    void DataWidget::addKey() {
      /*
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
      */
    }

    void DataWidget::addKey2() {
      std::string key = keyEdit->text().toStdString();
      std::string value = valueEdit->text().toStdString();
      QtVariantProperty *prop = dynamic_cast<QtVariantProperty*>(pDialog->activeItem());
      { // test for map
        ConfigMap *m = NULL;
        if(prop) {
          map<QtVariantProperty*, ConfigMap*>::iterator it;
          it = addMap.find(prop);
          if(it != addMap.end()) {
            ConfigItem *item = getItem(nameMap[prop]);
            m = *item;
          }
        }
        else {
          m = &config;
        }
        if(m) {
          if(m->hasKey(key)) {
            m->erase(key);
          }
          if(typeBox->currentIndex() == 0) { // map
            (*m)[key] = ConfigMap();
          }
          else if(typeBox->currentIndex() == 1) { // vector
            (*m)[key] = ConfigVector();
          }
          else {
            (*m)[key] = value;
            emit valueChanged(nameMap[prop]+"/"+key, value);
          }
          //config.toYamlFile("da.yml");
          setConfigMap(cname, config);
          emit mapChanged();
        }
      }
      if(prop) {
        { // test for vector
          map<QtVariantProperty*, ConfigVector*>::iterator it;
          it = addVector.find(prop);
          if(it != addVector.end()) {
            std::string n = nameMap[prop];
            ConfigItem *item = getItem(n);
            if(typeBox->currentIndex() == 0) { // map
              item->append(ConfigMap());
            }
            else if(typeBox->currentIndex() == 1) { // vector
              item->append(ConfigVector());
            }
            else {
              item->append(ConfigAtom(value));
              char da[10];
              sprintf(da, "/%d", (int)item->size()-1);
              emit valueChanged(nameMap[prop]+da, value);
            }
          }
        }
        //config.toYamlFile("da.yml");
        setConfigMap(cname, config);
        emit mapChanged();
      }
    }

    void DataWidget::accept() {}
    void DataWidget::reject() {}

    void DataWidget::setGroupChecked(const std::string &name, bool value) {
      ignore_change = 1;
      map<QtVariantProperty*, std::string>::iterator it = checkMap.begin();
      for(; it!=checkMap.end(); ++it) {
        if(it->second == name) {
          it->first->setValue(QVariant(value));
          break;
        }
      }
      ignore_change = 0;
    }

    configmaps::ConfigAtom* DataWidget::getAtom(std::string path) {
      ConfigItem *item = getItem(path);
      if(item) {
        return *item;
      }
      return NULL;
    }

    configmaps::ConfigItem* DataWidget::getItem(std::string path) {
      std::vector<std::string> arrPath = utils::explodeString('/', path);
      int index = 1;
      if(cname.size() > 0) {
        index = 2;
      }
      ConfigItem *item = config[arrPath[index]];
      for(size_t i=index+1; i<arrPath.size(); ++i) {
        if(item->isMap()) {
          item = ((*item)[arrPath[i]]);
        }
        else if (item->isVector()) {
          item = ((*item)[atoi(arrPath[i].c_str())]);
        }
        else if (item->isAtom()) {
          break;
        }
        else {
          fprintf(stderr, "ERROR: update configmap widget structure error!");
          return NULL;
        }
      }
      return item;
    }

  } // end of namespace config_map_widget

} // end of namespace mars
