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


#include "MyQDockWidget.h"
#include "MyQMainWindow.h"

namespace mars {
  namespace main_gui {

    MyQDockWidget::MyQDockWidget(QWidget *parent, QWidget *child, int p,
                                 int a, Qt::WindowFlags flags)
      : QDockWidget(child->windowTitle(), parent, flags) {
      priority = p;
      area = (Qt::DockWidgetArea)a;
      this->setWidget(child);
      child->setParent(this);
      this->adjustSize();
      this->setFeatures(QDockWidget::AllDockWidgetFeatures);
      this->setAllowedAreas(Qt::AllDockWidgetAreas);
      ((MyQMainWindow*)parent)->addDockWidget(area, this);
      //  this->setObjectName(child->objectName());
    }


    MyQDockWidget::~MyQDockWidget() {
    }

    void MyQDockWidget::closeEvent(QCloseEvent *event) {
      //widget()->close();
      return;
    }

  } // namespace main_gui
} // namespace mars
