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
#include <mars/interfaces/sim/MotorManagerInterface.h>
#include <mars/interfaces/sim/JointManagerInterface.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>
#include <mars/main_gui/GuiInterface.h>
#include <lib_manager/LibManager.hpp>
#include <QtGui>
#include <QPushButton>
#include <QHBoxLayout>

#include <QLineEdit>
#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QGridLayout>

#include <QMessageBox>
#include <QFileDialog>

#define GUI_ACTION_ADD_PLANE    1
#define GUI_ACTION_ADD_BOX      2
#define GUI_ACTION_ADD_SPHERE   3
#define GUI_ACTION_ADD_MATERIAL 4
#define GUI_ACTION_ADD_LIGHT    5
#define GUI_ACTION_ADD_MOTOR    6
#define GUI_ACTION_ADD_JOINT    7

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
                                    this, 0, tmp1, true);

      tmp1 = resPath + "/images";
      tmp1.append("/box.png");
      mainGui->addGenericMenuAction("../Edit/Add Box", GUI_ACTION_ADD_BOX,
                                    this, 0, tmp1, true);

      tmp1 = resPath + "/images";
      tmp1.append("/sphere.png");
      mainGui->addGenericMenuAction("../Edit/Add Sphere", GUI_ACTION_ADD_SPHERE,
                                    this, 0, tmp1, true);
      // add separator
      mainGui->addGenericMenuAction("../Edit/", 0, NULL, 0, "", 0, -1);
      mainGui->addGenericMenuAction("../Edit/Add Material",
                                    GUI_ACTION_ADD_MATERIAL, this);
      tmp1 = resPath + "/images";
      tmp1.append("/lamp.png");
      mainGui->addGenericMenuAction("../Edit/Add Light", GUI_ACTION_ADD_LIGHT,
                                    this, 0, tmp1, true);
      // add separator
      mainGui->addGenericMenuAction("../Edit/", 0, NULL, 0, "", 0, -1);

      mainGui->addGenericMenuAction("../Edit/Add Joint",
                                    GUI_ACTION_ADD_JOINT, this);

      mainGui->addGenericMenuAction("../Edit/Add Motor",
                                    GUI_ACTION_ADD_MOTOR, this);


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

      interfaces::LightData light;
      light.pos.x() = light.pos.x() = 0.00;
      light.pos.z() = 3.00;
      light.lookAt.x() = light.lookAt.x() = 0.00;
      light.lookAt.z() = -1.00;
      light.name = "light";
      light.constantAttenuation = 1.0000;
      light.quadraticAttenuation = 0.00002;
      light.type = 1;
      light.angle = 180;
      light.directional = false;
      light.ambient = utils::Color(50/255.,50/255.,50/255.,1);
      light.diffuse = utils::Color(1,1,1,1);
      light.specular = utils::Color(1,1,1,1);
      light.toConfigMap(&defaultLight);

      addType = 0;
      widgetAdd = new QWidget();
      QPushButton *button = new QPushButton("ok");
      addLabel = new QLabel("Name:");
      addLineEdit = new QLineEdit();
      vLayout = new QVBoxLayout();
      QHBoxLayout *layout = new QHBoxLayout();
      layout->addWidget(addLabel);
      layout->addWidget(addLineEdit);
      layout->addWidget(button);
      vLayout->addLayout(layout);
      widgetAdd->setLayout(vLayout);
      connect(button, SIGNAL(clicked()), this, SLOT(addObject()));

      comboLabel1 = new QLabel();
      comboLabel2 = new QLabel();
      combo1 = new QComboBox();
      combo2 = new QComboBox();
      gridLayout = new QGridLayout();
      gridLayout->addWidget(comboLabel1, 0, 0);
      gridLayout->addWidget(combo1, 0, 1);
      gridLayout->addWidget(comboLabel2, 1, 0);
      gridLayout->addWidget(combo2, 1, 1);
      comboLabel1->hide();
      comboLabel2->hide();
      combo1->hide();
      combo2->hide();
      vLayout->addLayout(gridLayout);
    }

    MenuAdd::~MenuAdd() {

    }

    void MenuAdd::menuAction(int action, bool checked)
    {
      (void)checked;
      std::string label, name;
      switch (action) {
      case GUI_ACTION_ADD_PLANE:    label="Plane Name:"; name="plane"; break;
      case GUI_ACTION_ADD_BOX:      label="Box Name:"; name="box"; break;
      case GUI_ACTION_ADD_SPHERE:   label="Sphere Name:"; name="sphere"; break;
      case GUI_ACTION_ADD_MATERIAL: label="Material Name:"; name="material"; break;
      case GUI_ACTION_ADD_LIGHT:    label="Light Name:"; name<<defaultLight["name"]; break;
      case GUI_ACTION_ADD_MOTOR: {
        std::vector<interfaces::core_objects_exchange>::iterator it;
        std::vector<interfaces::core_objects_exchange> simJoints;
        control->joints->getListJoints(&simJoints);
        combo1->clear();
        for(it=simJoints.begin(); it!=simJoints.end(); ++it) {
          combo1->addItem(it->name.c_str());
        }
        comboLabel1->setText("Add to joint:");
        comboLabel1->show();
        combo1->show();
        comboLabel2->hide();
        combo2->hide();
        label="Motor Name:";
        name="motor"; break;
      }
      case GUI_ACTION_ADD_JOINT: {
        std::vector<interfaces::core_objects_exchange>::iterator it;
        std::vector<interfaces::core_objects_exchange> simNodes;
        control->nodes->getListNodes(&simNodes);
        combo1->clear();
        combo2->clear();
        combo2->addItem("world");
        for(it=simNodes.begin(); it!=simNodes.end(); ++it) {
          combo1->addItem(it->name.c_str());
          combo2->addItem(it->name.c_str());
        }
        comboLabel1->setText("Add to first node:");
        comboLabel1->show();
        combo1->show();
        comboLabel2->setText("Add to second node:");
        comboLabel2->show();
        combo2->show();
        label="Joint Name:";
        name="joint"; break;
      }
      default: break;
      }
      addType = action;
      addLabel->setText(label.c_str());
      addLineEdit->setText(name.c_str());
      widgetAdd->show();
    }

    void MenuAdd::addObject() {
      std::string name = addLineEdit->text().toStdString();
      switch (addType) {
      case GUI_ACTION_ADD_PLANE: menu_addPlane(name); break;
      case GUI_ACTION_ADD_BOX: menu_addBox(name); break;
      case GUI_ACTION_ADD_SPHERE: menu_addSphere(name); break;
      case GUI_ACTION_ADD_MATERIAL: menu_addMaterial(name); break;
      case GUI_ACTION_ADD_LIGHT: menu_addLight(name); break;
      case GUI_ACTION_ADD_MOTOR: menu_addMotor(name); break;
      case GUI_ACTION_ADD_JOINT: menu_addJoint(name); break;
      default: break;
      }
      widgetAdd->hide();
      addType = 0;
    }

    void MenuAdd::menu_addMotor(const std::string &name) {
      configmaps::ConfigMap map;
      map["name"] = name;
      map["jointIndex"] = control->joints->getID(combo1->currentText().toStdString());
      map["jointIndex2"] = 0lu;
      interfaces::MotorData data;
      data.fromConfigMap(&map, "");
      control->motors->addMotor(&data);
      comboLabel1->hide();
      combo1->hide();
    }

    void MenuAdd::menu_addJoint(const std::string &name) {
      configmaps::ConfigMap map;
      map["name"] = name;
      map["type"] = "hinge";
      map["axis1"]["x"] = 0.0;
      map["axis1"]["y"] = 0.0;
      map["axis1"]["z"] = 1.0;
      map["nodeindex1"] = control->nodes->getID(combo1->currentText().toStdString());
      if(combo2->currentText() == "world") {
        map["nodeindex2"] = 0lu;
      }
      else {
        map["nodeindex2"] = control->nodes->getID(combo2->currentText().toStdString());
      }
      interfaces::JointData data;
      data.fromConfigMap(&map, "");
      control->joints->addJoint(&data);
      comboLabel1->hide();
      combo1->hide();
      comboLabel2->hide();
      combo2->hide();
    }

    void MenuAdd::menu_addLight(const std::string &name) {
      interfaces::LightData data;
      defaultLight["name"] = name;
      data.fromConfigMap(&defaultLight, "");
      if(control->graphics) {
        control->graphics->addLight(data);
      }
    }

    // todo: handle name clash
    void MenuAdd::menu_addMaterial(const std::string &name) {
      interfaces::MaterialData md;
      md.name = name;
      //fprintf(stderr, "add material: %s\n", md.name.c_str());
      control->graphics->addMaterial(md);
    }

    void MenuAdd::menu_addPlane(const std::string &name) {
      configmaps::ConfigMap map;
      map["name"] = name;
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

    void MenuAdd::menu_addBox(const std::string &name) {
      configmaps::ConfigMap map;
      map["name"] = name;
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

    void MenuAdd::menu_addSphere(const std::string &name) {
      configmaps::ConfigMap map;
      map["name"] = name;
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
