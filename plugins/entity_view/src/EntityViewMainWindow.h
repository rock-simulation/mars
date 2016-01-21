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
 * \file EntityViewMainWindow.h
 * \author Malte (malte.langosz@me.com)
 * \brief A
 *
 * Version 0.1
 */

#ifndef MARS_PLUGINS_ENTITY_VIEW_MAIN_WINDOW_H
#define MARS_PLUGINS_ENTITY_VIEW_MAIN_WINDOW_H

#include "SelectionTree.h"

#include <mars/main_gui/BaseWidget.h>
#include <mars/interfaces/sim/ControlCenter.h>
#include <configmaps/ConfigData.h>
#include <mars/config_map_gui/DataWidget.h>

namespace mars {
  namespace plugins {
    namespace EntityView {

      // TODO: use the base widget
      class EntityViewMainWindow : public main_gui::BaseWidget {

      public:
        EntityViewMainWindow (interfaces::ControlCenter *c);
        virtual ~EntityViewMainWindow ();

      private:
        SelectionTree *tree;
        interfaces::ControlCenter *c;
        config_map_gui::DataWidget *dw;

      };
    } // end of namespace EntityView
  } // end of namespace plugins
} // end of namespace mars

#endif
