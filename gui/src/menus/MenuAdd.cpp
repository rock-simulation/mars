/*
 *  Copyright 2016, DFKI GmbH Robotics Innovation Center
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
 * \file MenuAdd.h
 * \author Malte Langosz
 * \brief MenuAdd creates the menus and menu items in the Add menu of the simulation.
 */

#include "MenuAdd.h"

#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/interfaces/sim/SimulatorInterface.h>
#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>
#include <mars/main_gui/GuiInterface.h>
#include <lib_manager/LibManager.hpp>
#include <QtGui>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>

#include <QMessageBox>
#include <QFileDialog>

#define GUI_ACTION_ADD_PLANE    1
#define GUI_ACTION_ADD_BOX      2
#define GUI_ACTION_ADD_SPHERE   3
#define GUI_ACTION_ADD_MATERIAL 4

namespace mars {
  namespace gui {

    /**
     * \brief MenuAdd creates the menus and menu items in the File menu of the simulation.
     */
    MenuAdd::MenuAdd(interfaces::ControlCenter *c,
                       main_gui::GuiInterface *gui, std::string resPath,
                       lib_manager::LibManager *theManager)
      : libManager(theManager), mainGui(gui), control(c) {

      std::string tmp1;

      tmp1 = resPath + "/images";
      tmp1.append("/plane.png");
      mainGui->addGenericMenuAction("../Edit/Add Plane", GUI_ACTION_ADD_PLANE,
                                    (main_gui::MenuInterface*)this, 0, tmp1, true);

      tmp1 = resPath + "/images";
      tmp1.append("/box.png");
      mainGui->addGenericMenuAction("../Edit/Add Box", GUI_ACTION_ADD_BOX,
                                    (main_gui::MenuInterface*)this, 0, tmp1, true);

      tmp1 = resPath + "/images";
      tmp1.append("/sphere.png");
      mainGui->addGenericMenuAction("../Edit/Add Sphere", GUI_ACTION_ADD_SPHERE,
                                    (main_gui::MenuInterface*)this, 0, tmp1, true);
      mainGui->addGenericMenuAction("../Edit/Add Material",
                                    GUI_ACTION_ADD_MATERIAL,
                                    (main_gui::MenuInterface*)this);

      material["name"] = "defaultGrey";
      material["diffuseColor"]["a"] = 1.0;
      material["diffuseColor"]["r"] = 0.33;
      material["diffuseColor"]["g"] = 0.39;
      material["diffuseColor"]["b"] = 0.5;
      material["specularColor"]["a"] = 1.0;
      material["specularColor"]["r"] = 0.5;
      material["specularColor"]["g"] = 0.5;
      material["specularColor"]["b"] = 0.5;
      material["ambientColor"]["a"] = 1.0;
      material["ambientColor"]["r"] = 0.53;
      material["ambientColor"]["g"] = 0.59;
      material["ambientColor"]["b"] = 0.7;
      material["shininess"] = 100.;

      widgetAddMaterial = new QWidget();
      QPushButton *button = new QPushButton("ok");
      QLabel *label = new QLabel("Material Name:");
      materialLineEdit = new QLineEdit();
      QHBoxLayout *layout = new QHBoxLayout();
      layout->addWidget(label);
      layout->addWidget(materialLineEdit);
      layout->addWidget(button);
      widgetAddMaterial->setLayout(layout);
      connect(button, SIGNAL(clicked()), this, SLOT(addMaterial()));
    }

    MenuAdd::~MenuAdd() {

    }

    void MenuAdd::menuAction(int action, bool checked)
    {
      (void)checked;
      switch (action) {
      case GUI_ACTION_ADD_PLANE: menu_addPlane(); break;
      case GUI_ACTION_ADD_BOX: menu_addBox(); break;
      case GUI_ACTION_ADD_SPHERE: menu_addSphere(); break;
      case GUI_ACTION_ADD_MATERIAL: menu_addMaterial(); break;
      }
    }

    void MenuAdd::addMaterial() {
      interfaces::MaterialData md;
      md.name = materialLineEdit->text().toStdString();
      fprintf(stderr, "add material: %s\n", md.name.c_str());
      control->graphics->addMaterial(md);
      widgetAddMaterial->hide();
    }

    void MenuAdd::menu_addMaterial(void) {
      widgetAddMaterial->show();
    }

    void MenuAdd::menu_addPlane(void) {
      configmaps::ConfigMap map;
      map["name"] = "plane";
      map["physicmode"] = "plane";
      map["origname"] = "plane";
      map["filename"] = "PRIMITIVE";
      map["extend"]["x"] = 10.;
      map["extend"]["y"] = 10.;
      map["extend"]["z"] = 10.;
      map["movable"] = false;
      interfaces::NodeData node;
      node.fromConfigMap(&map, "");
      node.material.fromConfigMap(&material, "");
      control->nodes->addNode(&node);
    }

    void MenuAdd::menu_addBox(void) {
      configmaps::ConfigMap map;
      map["name"] = "box";
      map["physicmode"] = "box";
      map["origname"] = "box";
      map["filename"] = "PRIMITIVE";
      map["extend"]["x"] = 1.;
      map["extend"]["y"] = 1.;
      map["extend"]["z"] = 1.;
      map["position"]["z"] = 0.5;
      map["movable"] = true;
      interfaces::NodeData node;
      node.fromConfigMap(&map, "");
      node.material.fromConfigMap(&material, "");
      control->nodes->addNode(&node);
    }

    void MenuAdd::menu_addSphere(void) {
      configmaps::ConfigMap map;
      map["name"] = "sphere";
      map["physicmode"] = "sphere";
      map["visualType"] = "sphere";
      fprintf(stderr, "add with visual type\n");
      map["extend"]["x"] = 0.5;
      map["extend"]["y"] = 1.;
      map["extend"]["z"] = 1.;
      map["position"]["z"] = 0.5;
      map["movable"] = true;
      interfaces::NodeData node;
      node.fromConfigMap(&map, "");
      node.material.fromConfigMap(&material, "");
      control->nodes->addNode(&node);
    }

  } // end of namespace gui
} // end of namespace mars
