/*
 *  Copyright 2015, 2016 DFKI GmbH Robotics Innovation Center
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
 * \file SelectionTree.h
 * \brief A widget for node selection with different selection options.
 */


#ifndef SELECTION_TREE_H
#define SELECTION_TREE_H

#ifdef _PRINT_HEADER_
#warning "SelectionTree.h"
#endif

//#include <QtGui>
#include <mars/main_gui/BaseWidget.h>
#include <mars/interfaces/NodeData.h>
#include <mars/interfaces/JointData.h>
#include <mars/interfaces/MotorData.h>
#include <mars/interfaces/sensor_bases.h>
#include <mars/interfaces/ControllerData.h>
#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/interfaces/core_objects_exchange.h>
#include <mars/interfaces/graphics/GraphicsEventClient.h>

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <mars/config_map_gui/DataWidget.h>

namespace mars {
  namespace plugins {

    class SelectionTree : public main_gui::BaseWidget,
                          public interfaces::GraphicsEventClient {
      Q_OBJECT

      public:
      SelectionTree(interfaces::ControlCenter *c,
                    config_map_gui::DataWidget *dw, QWidget *parent = NULL);
      ~SelectionTree();
      void selectEvent(unsigned long int id, bool mode);

    private:
      config_map_gui::DataWidget *dw;
      interfaces::ControlCenter *control;
      bool filled, selectAllowed;
      int editCategory;
      std::vector<interfaces::core_objects_exchange> simNodes, simJoints;
      std::vector<interfaces::core_objects_exchange> simMotors, simSensors;
      std::vector<interfaces::core_objects_exchange> simControllers;
      std::map<std::string, configmaps::ConfigMap> materialMap;
      std::vector<unsigned long> present;
      std::map<unsigned long, QTreeWidgetItem*> nodeItemMap;
      QTreeWidget *treeWidget;
      interfaces::NodeData nodeData;
      interfaces::JointData jointData;
      interfaces::MotorData motorData;
      interfaces::BaseSensor sensorData;
      interfaces::ControllerData controllerData;
      configmaps::ConfigMap currentMaterial;
      configmaps::ConfigMap defaultMaterial;

      void closeEvent(QCloseEvent* event);
      void fill(unsigned long id, QTreeWidgetItem *current = NULL);
      void reset(void);
      void createTree();
      void addCoreExchange(const std::vector<interfaces::core_objects_exchange> &objects, std::string category);

    signals:
      void closeSignal(void* widget);
      void itemSelectionChanged();

    private slots:
      void selectNodes(void);
      void valueChanged(std::string name, std::string value);
      void deleteEntities(void);
      void update(void);
    };

  } // end of namespace plugins
} // end of namespace mars

#endif
