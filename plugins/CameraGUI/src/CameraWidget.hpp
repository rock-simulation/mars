/*
 *  Copyright 2018, DFKI GmbH Robotics Innovation Center
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
 * \file CameraWidget.hpp
 * \author Malte Langosz
 * \brief
 **/

#ifndef CAMERA_WIDGET_HPP
#define CAMERA_WIDGET_HPP

#ifdef _PRINT_HEADER_
#warning "CameraWidget.hpp"
#endif

#include <mars/main_gui/BaseWidget.h>
#include <configmaps/ConfigData.h>
#include <mars/config_map_gui/DataWidget.h>
#include <mars/graphics/wrapper/OSGHudElementStruct.h>
#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/graphics/GraphicsWidget.h>
#include <mars/main_gui/GuiInterface.h>

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>

namespace mars{
  namespace plugins {
    namespace CameraGUI {

      class CameraWidget : public main_gui::BaseWidget {

        Q_OBJECT;

      public:
        CameraWidget(mars::interfaces::ControlCenter *control,
                     mars::main_gui::GuiInterface *gui,
                     QWidget *parent, const std::string &name);
        ~CameraWidget();


      public slots:
        void update();
        void set(int);
        void saveImage();

      private:
        osg::ref_ptr<graphics::OSGHudElementStruct> elem;
        graphics::GraphicsWidget *gw;
        QComboBox *windowIDs;
        mars::interfaces::ControlCenter *control;
        mars::main_gui::GuiInterface *gui;
        unsigned long winID, currentID;
        bool depthImage;
      };

    } // end of namespace CameraGUI
  } // end of namespace plugins
} // end of namespace mars

#endif // CAMERA_WIDGET_H
