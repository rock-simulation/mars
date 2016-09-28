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
 * \file BaseWidget.cpp
 * \author Malte Römmermann
 */

#include "BaseWidget.h"
#include <QEvent>

namespace mars {
  namespace main_gui {
    
    BaseWidget::BaseWidget(QWidget *parent, cfg_manager::CFGManagerInterface *_cfg,
                           std::string _widgetName):
      QWidget(parent), cfg(_cfg), setWindowProp(false), widgetName(_widgetName) {
    
      if(cfg) cfgWindow();
      setWindowTitle(QString::fromStdString(_widgetName));
    }
  
    BaseWidget::~BaseWidget() {
      if(cfg) {
        //fprintf(stderr, "delete: %s %d\n", widgetName.c_str(), hiddenState);
        //fflush(stderr);
        cfg->unregisterFromParam(wTop.paramId, this);
        cfg->unregisterFromParam(wLeft.paramId, this);
        cfg->unregisterFromParam(wWidth.paramId, this);
        cfg->unregisterFromParam(wHeight.paramId, this);
        cfg->setProperty("Windows", widgetName+"/hidden", hiddenState);
      }
    }

    void BaseWidget::cfgUpdateProperty(cfg_manager::cfgPropertyStruct _property) {
      bool change_view = 0;

      if(setWindowProp) return;

      if(_property.paramId == wTop.paramId &&
         wTop.iValue != _property.iValue) {
        wTop.iValue = _property.iValue;
        change_view = 1;
      }
      else if(_property.paramId == wLeft.paramId &&
              wLeft.iValue != _property.iValue) {
        wLeft.iValue = _property.iValue;
        change_view = 1;
      }
      else if(_property.paramId == wWidth.paramId &&
              wWidth.iValue != _property.iValue) {
        wWidth.iValue = _property.iValue;
        change_view = 1;
      }
      else if(_property.paramId == wHeight.paramId &&
              wHeight.iValue != _property.iValue) {
        wHeight.iValue = _property.iValue;
        change_view = 1;
      }

      if(change_view) {
        setWindowProp = true;
        setGeometry(wLeft.iValue, wTop.iValue, wWidth.iValue, wHeight.iValue);
        setWindowProp = false;
      }
    }

    void BaseWidget::show() {
      if(parentWidget()) {
        parentWidget()->show();
      }
      else {
        QWidget::show();
      }
    }

    void BaseWidget::hide() {
      if(parentWidget()) {
        parentWidget()->hide();
      }
      else {
        QWidget::hide();
      }
    }

    bool BaseWidget::isHidden() {
      if(parentWidget()) {
        return parentWidget()->isHidden();
      }
      else {
        return QWidget::isHidden();
      }
    }

    void BaseWidget::changeEvent(QEvent *ev) {
      QWidget::changeEvent(ev);

      if(setWindowProp) return;

      int top = geometry().y();
      int left = geometry().x();
      int width = geometry().width();
      int height = geometry().height();
      bool updateCfg = false;

      if(top != wTop.iValue || left != wLeft.iValue ||
         width != wWidth.iValue || height != wHeight.iValue) {
        updateCfg = true;
      }

      wTop.iValue = top;
      wLeft.iValue = left;
      wWidth.iValue = width;
      wHeight.iValue = height;

      if(updateCfg && cfg) {
        setWindowProp = true;
        cfg->setProperty(wTop);
        cfg->setProperty(wLeft);
        cfg->setProperty(wWidth);
        cfg->setProperty(wHeight);
        setWindowProp = false;
      }
    }

    void BaseWidget::cfgWindow(void) {
      cfg_manager::CFGClient *cfgClient = dynamic_cast<cfg_manager::CFGClient*>(this);
      const char *group = "Windows";
      std::string pName;

      pName = widgetName;
      pName.append("/Window Top");
      wTop = cfg->getOrCreateProperty(group, pName.c_str(), (int)40, cfgClient);

      pName = widgetName;
      pName.append("/left");
      wLeft = cfg->getOrCreateProperty(group, pName.c_str(), (int)40, cfgClient);

      pName = widgetName;
      pName.append("/width");
      wWidth = cfg->getOrCreateProperty(group, pName.c_str(),
                                        (int)400, cfgClient);

      pName = widgetName;
      pName.append("/height");
      wHeight = cfg->getOrCreateProperty(group, pName.c_str(),
                                         (int)400, cfgClient);

      pName = widgetName;
      pName.append("/hidden");
      hidden = cfg->getOrCreateProperty(group, pName.c_str(),
                                        (bool)true);
      hiddenState = hidden.bValue;

      setGeometry(wLeft.iValue, wTop.iValue, wWidth.iValue, wHeight.iValue);
    }

    void BaseWidget::applyGeometry() {
      setGeometry(wLeft.iValue, wTop.iValue, wWidth.iValue, wHeight.iValue);    
    }

    void BaseWidget::setHiddenCloseState(bool v) {
      hiddenState = v;
    }

    bool BaseWidget::getHiddenCloseState() {
      return hiddenState;
    }

    void BaseWidget::closeEvent(QCloseEvent *event) {
      saveState();
      emit closeSignal();
    }

    void BaseWidget::saveState() {
      if(cfg) {
        cfg->unregisterFromParam(wTop.paramId, this);
        cfg->unregisterFromParam(wLeft.paramId, this);
        cfg->unregisterFromParam(wWidth.paramId, this);
        cfg->unregisterFromParam(wHeight.paramId, this);
        cfg->setProperty("Windows", widgetName+"/hidden", hiddenState);        
      }
    }

    void BaseWidget::hideEvent(QHideEvent* event) {
      (void)event;
      emit hideSignal();
    }

  } // end namespace main_gui
} // end namespace mars
