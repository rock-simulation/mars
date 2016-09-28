/*
 *  Copyright 2011, 2012, 2016, DFKI GmbH Robotics Innovation Center
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
 * \file BaseWidget.h
 * \author Malte Römmermann
 */

#ifndef BASE_WIDGET_H
#define BASE_WIDGET_H

#ifdef _PRINT_HEADER_
  #warning "BaseWidget.h"
#endif

#include <mars/cfg_manager/CFGManagerInterface.h>
#include <QWidget>

namespace mars {
  namespace main_gui {

    class BaseWidget : public QWidget, public cfg_manager::CFGClient {

      Q_OBJECT

    public:

      BaseWidget(QWidget *parent, cfg_manager::CFGManagerInterface *_cfg,
                 std::string _widgetName);

      ~BaseWidget();
      virtual void cfgUpdateProperty(cfg_manager::cfgPropertyStruct _property);
      void setHiddenCloseState(bool v);
      bool getHiddenCloseState();
      void show();
      void hide();
      bool isHidden();
      void saveState();

    signals:
      void hideSignal(void);
      void closeSignal(void);

    protected:
      cfg_manager::CFGManagerInterface *cfg;
      bool setWindowProp;
      bool hiddenState;

      void changeEvent(QEvent *ev);
      void cfgWindow(void);
      void applyGeometry();
      void hideEvent(QHideEvent* event);
      void closeEvent(QCloseEvent* event);

    private:
      cfg_manager::cfgPropertyStruct wTop, wLeft;
      cfg_manager::cfgPropertyStruct wHeight, wWidth;
      cfg_manager::cfgPropertyStruct hidden;
      std::string widgetName;
    }; // end class BaseWidget

  } // end namespace main_gui
} // end namespace mars

#endif /* BASE_WIDGET_H */
