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

#include "CameraWidget.hpp"

#include <mars/interfaces/graphics/GraphicsManagerInterface.h>
#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/sim/SensorManagerInterface.h>
#include <mars/sim/CameraSensor.h>
#include <mars/utils/misc.h>
#include <mars/graphics/2d_objects/HUDTexture.h>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <mars/utils/misc.h>

#include <osgDB/WriteFile>


#include <cassert>

namespace mars {

  using namespace utils;
  using namespace interfaces;

  namespace plugins {
    namespace CameraGUI {


      CameraWidget::CameraWidget(interfaces::ControlCenter *control,
                                 main_gui::GuiInterface *gui,
                                 QWidget *parent, const std::string &name) :
        main_gui::BaseWidget(parent, control->cfg, name),
        control(control), gui(gui) {


        QVBoxLayout *vLayout = new QVBoxLayout();
        QHBoxLayout *hLayout = new QHBoxLayout();
        QPushButton *button;

        windowIDs = new QComboBox();
        connect(windowIDs, SIGNAL(currentIndexChanged(int)), this, SLOT(set(int)));

        hLayout->addWidget(windowIDs);

        button = new QPushButton("update list", this);
        connect(button, SIGNAL(clicked()), this, SLOT(update()));
        hLayout->addWidget(button);

        button = new QPushButton("save image", this);
        connect(button, SIGNAL(clicked()), this, SLOT(saveImage()));
        hLayout->addWidget(button);

        vLayout->addLayout(hLayout);

        winID = control->graphics->new3DWindow(0, 0, 1280, 768,
                                               "graph_gui");
        gw = (graphics::GraphicsWidget*)control->graphics->get3DWindow(winID);
        gw->setScene(new osg::Group());

        hudElementStruct he;
        he.type = HUD_ELEMENT_TEXTURE;
        he.posx = 0;
        he.posy = 0;
        he.width = 1920;
        he.height = 1080;
        he.texture_width = 1920;
        he.texture_height = 1080;
        he.background_color[0] = 0.9;
        he.background_color[1] = 0.9;
        he.background_color[2] = 0.9;
        he.background_color[3] = 0.95;
        he.border_color[0] = 1.0;
        he.border_color[1] = 1.0;
        he.border_color[2] = 1.0;
        he.border_color[3] = 0.5;
        he.border_width = 4.0;
        he.font_size = 42;
        he.padding[0] = 5;
        he.padding[1] = 5;
        he.padding[2] = 5;
        he.padding[3] = 5;
        he.direction = 2;
        elem = new graphics::OSGHudElementStruct(he, "", 1);
        if (elem) {
          gw->addHUDElement(elem->getHUDElement());
        }
        vLayout->addWidget((QWidget*)gw->getWidget());

        setLayout(vLayout);
      }

      CameraWidget::~CameraWidget(void) {
        //fprintf(stderr, "Delete CameraWidget\n");
        //gui->removeDockWidget(this, 0);
        control->graphics->remove3DWindow(winID);
        //delete windowIDs;
      }


      void CameraWidget::update() {
        std::vector<interfaces::core_objects_exchange> sensorList;
        std::vector<interfaces::core_objects_exchange>::iterator it;

        windowIDs->clear();
        control->sensors->getListSensors(&sensorList);
        for(it=sensorList.begin(); it!=sensorList.end(); ++it) {
          const interfaces::BaseSensor* bs;
          bs = control->sensors->getFullSensor(it->index);
          const sim::CameraSensor *c = dynamic_cast<const sim::CameraSensor*>(bs);
          if(c) {
            std::stringstream ss;
            ss << c->getWindowID() << ":" << c->getName() << ":color";
            windowIDs->addItem(ss.str().c_str());
            ss.str("");
            ss << c->getWindowID() << ":" << c->getName() << ":depth";
            windowIDs->addItem(ss.str().c_str());
          }
        }
      }

      void CameraWidget::set(int i) {
        std::string camera = windowIDs->currentText().toStdString();
        std::vector<std::string> arrString = explodeString(':', camera);
        sscanf(arrString[0].c_str(), "%lu", &currentID);
        graphics::GraphicsWidget *g;
        g = (graphics::GraphicsWidget*)control->graphics->get3DWindow(currentID);
        graphics::HUDTexture *t;
        t = dynamic_cast<graphics::HUDTexture*>(elem->getHUDElement());
        if(arrString.back() == "color") {
          t->setTexture(g->getRTTTexture());
          depthImage = false;
        }
        else if(arrString.back() == "depth") {
          t->setTexture(g->getRTTDepthTexture());
          depthImage = true;
        }
      }

      void CameraWidget::saveImage() {
        QString fileName = QFileDialog::getSaveFileName(NULL, QObject::tr("select save file name"),
                                                        ".", QObject::tr("png file (*.png)"),0,
                                                        QFileDialog::DontUseNativeDialog);
        if(!fileName.isNull()) {
          graphics::GraphicsWidget *g;
          g = (graphics::GraphicsWidget*)control->graphics->get3DWindow(currentID);
          if(depthImage) {
            osgDB::writeImageFile(*(g->getRTTDepthImage()), fileName.toStdString());
          }
          else {
            osgDB::writeImageFile(*(g->getRTTImage()), fileName.toStdString());
          }
        }
      }
    } // end of namespace CameraGUI
  } // end of namespace plugins
} // end of namespace mars
