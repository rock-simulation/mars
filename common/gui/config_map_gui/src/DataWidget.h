/*
 *  Copyright 2014, DFKI GmbH Robotics Innovation Center
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
 * \author Malte Langosz
 * \brief 
 **/

#ifndef DATA_WIDGET_H
#define DATA_WIDGET_H

#ifdef _PRINT_HEADER_
#warning "DataWidget.h"
#endif

#include <mars/main_gui/PropertyDialog.h>
#include <configmaps/ConfigData.h>

#include <vector>
#include <set>

#include <QWidget>
#include <QCloseEvent>
#include <QMutex>
#include <QComboBox>
#include <QLineEdit>

namespace mars {

  using namespace std;

  namespace config_map_gui {

    class DataWidget : public QWidget,
                       public main_gui::PropertyCallback {
    
      Q_OBJECT;
      
    public:
      DataWidget(void* backwardCFG = 0, QWidget *parent = 0, bool onlyCompactView = false, bool allowAdd = true);
      ~DataWidget();
    
      virtual void valueChanged(QtProperty *property, const QVariant &value);
    
      main_gui::PropertyDialog *pDialog;
      void setConfigMap(const std::string &name,
                        const configmaps::ConfigMap &map);
      void setEditPattern(const std::vector<std::string> &pattern);
      void setColorPattern(const std::vector<std::string> &pattern);
      void setFilePattern(const std::vector<std::string> &pattern);
      void setCheckablePattern(const std::vector<std::string> &pattern);
      void setDropDownPattern(const std::vector<std::string> &pattern,
                              const std::vector<std::vector<std::string> > &values);
      void setFilterPattern(const std::vector<std::string> &pattern);
      void setBlackFilterPattern(const std::vector<std::string> &pattern);
      void addConfigMap(const std::string &name, configmaps::ConfigMap &map);
      void addConfigAtom(const std::string &name, configmaps::ConfigAtom &v);
      void addConfigVector(const std::string &name, configmaps::ConfigVector &v);
      void updateConfigMap(const std::string &name,
                           const configmaps::ConfigMap &map);
      void updateConfigMapI(const std::string &name,
                            configmaps::ConfigMap &map);
      void updateConfigAtomI(const std::string &name,
                             configmaps::ConfigAtom &map);
      void updateConfigVectorI(const std::string &name,
                               configmaps::ConfigVector &map);
      const configmaps::ConfigMap& getConfigMap();
      void clearGUI();
      void setGroupChecked(const std::string &name, bool value);

    signals:
      void mapChanged();
      void valueChanged(std::string, std::string);
      void checkChanged(std::string, bool);
      void colorChanged(std::string, float r, float g, float b, float a);

    private:
      QMutex addMutex;
      configmaps::ConfigMap config;
      std::vector<std::string> editPattern, colorPattern, filePattern, dropDownPattern, checkablePattern, filterPattern, blackFilterPattern;
      std::vector<std::vector<std::string> > dropDownValues;
      map<QtVariantProperty*, configmaps::ConfigAtom*> dataMap;
      map<QtVariantProperty*, configmaps::ConfigMap*> addMap, colorMap;
      map<QtVariantProperty*, configmaps::ConfigVector*> addVector;
      map<QtVariantProperty*, std::string> checkMap;
      map<std::string, QtVariantProperty*> propMap;
      map<QtVariantProperty*, std::string> nameMap;
      std::string addKeyStr, cname;
      bool ignore_change;
      QtVariantProperty* addProperty;
      QComboBox *typeBox;
      QLineEdit *keyEdit, *valueEdit;

      int checkInPattern(const std::string &v,
                         const std::vector<std::string> &pattern);

    private slots:
      void addKey();
      void addKey2();

    protected slots:
      void timerEvent(QTimerEvent* event);
      virtual void accept();
      virtual void reject();

    };
  
  } // end of namespace config_map_gui

} // end of namespace mars 

#endif // DATA_WIDGET_H
  
