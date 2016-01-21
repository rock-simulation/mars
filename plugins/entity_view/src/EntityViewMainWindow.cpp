/*
 *  Copyright 2015, 2016, DFKI GmbH Robotics Innovation Center
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
 * \file EntityViewMainWindow.cpp
 * \author Malte (malte.langosz@me.com)
 * \brief A
 *
 * Version 0.1
 */

#include "EntityViewMainWindow.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>

namespace mars {
  namespace plugins {
    namespace EntityView {

      EntityViewMainWindow::EntityViewMainWindow (interfaces::ControlCenter *c)
        : main_gui::BaseWidget(0, c->cfg, "Entity View"), c(c) {
        setWindowTitle ("Entity View");

        setStyleSheet("padding:0px;");
        dw = new config_map_gui::DataWidget(c->cfg, 0, true);
        tree = new SelectionTree(c, dw);

        QSplitter *splitter = new QSplitter();
        splitter->addWidget(tree);
        splitter->addWidget(dw);

        QHBoxLayout *hLayout = new QHBoxLayout();
        hLayout->setContentsMargins(1, 1, 1, 1);

        hLayout->addWidget(splitter);
        this->setLayout(hLayout);
      }

      EntityViewMainWindow::~EntityViewMainWindow () {
        delete tree;
        delete dw;
      }
    } // end of namespace EntityView
  } // end of namespace plugins
} // end of namespace mars
