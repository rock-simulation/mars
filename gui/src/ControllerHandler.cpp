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

#include "ControllerHandler.h"
#include <mars/interfaces/sim/ControllerManagerInterface.h>
#include <mars/interfaces/sim/SensorManagerInterface.h>
#include <mars/interfaces/sim/MotorManagerInterface.h>
#include <mars/interfaces/sim/NodeManagerInterface.h>

namespace mars {
  namespace gui {

    static int local_ind = 0;

    ControllerHandler::ControllerHandler(QtVariantProperty* property,
                                         unsigned long index,
                                         main_gui::PropertyDialog *pd, 
                                         interfaces::ControlCenter *c,
                                         ControllerTree::Mode m) 
    {
      mode = m;
      control = c;
      topLevelController = property;
      myControllerIndex = (int) index;
      pDialog = pd;
      filled = false;
      editColor = pDialog->getPropertyColor(topLevelController);
      previewColor = QColor(255, 200, 200);
 
      control->controllers->getListController(&allControllers);
  
      if (mode == ControllerTree::EditMode) 
        myController = control->controllers->getFullController(myControllerIndex);
      else {
        myControllerIndex = local_ind++;
        std::vector<unsigned long> chosen;
        myController.rate = 20;
        myController.motors = myController.sNodes = myController.sensors = chosen;
        myController.id = 0;
        pDialog->setPropertyColor(topLevelController, previewColor);
      }
      actualName = "Controller"+QString::number(myControllerIndex).toStdString();
      controllerName = QString::number(index).toStdString() + ":" + actualName;
      topLevelController->setPropertyName(QString::fromStdString(controllerName));
      fill();
      filled = true;
    }

  
    ControllerHandler::~ControllerHandler() {
      pDialog->removeGenericProperty(topLevelController);
    }


    void ControllerHandler::fill() {
      std::map<QString, QVariant> attr;
      attr.insert(std::pair<QString, QVariant>(QString("minimum"), 0));

      filename = pDialog->addGenericProperty("../"+controllerName+"/Load", 
                                             VariantManager::filePathTypeId(), "");
      filename->setAttribute(QString("directory"), QString("."));
      rate = pDialog->addGenericProperty("../"+controllerName+"/Update rate", QVariant::Int, 20, &attr);
      sensors = pDialog->addGenericProperty("../"+controllerName+"/Sensors", QVariant::String, "");
      motors = pDialog->addGenericProperty("../"+controllerName+"/Motors", QVariant::String, "");
      nodes = pDialog->addGenericProperty("../"+controllerName+"/Nodes", QVariant::String, "");
      //  sensors->setEnabled(false); motors->setEnabled(false); nodes->setEnabled(false);

      QStringList items;
      control->sensors->getListSensors(&mySensors);
      control->motors->getListMotors(&myMotors);
      control->nodes->getListNodes(&myNodes);

      for(unsigned int i = 0;i<mySensors.size();i++)
        items << QString::fromStdString(mySensors[i].name);
  
      sensorDialog = new SelectionDialog(pDialog);
      sensorDialog->reset(items);
      sensorDialog->setWindowFlags(Qt::Window);
      sensorDialog->setWindowTitle(tr("Select Sensors"));

      items.clear();
      for(unsigned int i = 0;i<myMotors.size();i++)
        items << QString::fromStdString(myMotors[i].name);
  
      motorDialog = new SelectionDialog(pDialog);
      motorDialog->reset(items);
      motorDialog->setWindowFlags(Qt::Window);
      motorDialog->setWindowTitle(tr("Select Motors"));

      items.clear();
      for(unsigned int i = 0;i<myNodes.size();i++) 
        items << QString::fromStdString(myNodes[i].name);
  
      nodeDialog = new SelectionDialog(pDialog);
      nodeDialog->reset(items);
      nodeDialog->setWindowFlags(Qt::Window);
      nodeDialog->setWindowTitle(tr("Select Nodes"));

      QObject::connect(nodeDialog, SIGNAL(modified(QString)), this, SLOT(choose_nodes(QString)));
      QObject::connect(motorDialog, SIGNAL(modified(QString)), this, SLOT(choose_motors(QString)));
      QObject::connect(sensorDialog, SIGNAL(modified(QString)), this, SLOT(choose_sensors(QString)));

      if (mode == ControllerTree::EditMode) {
        interfaces::ControllerData cs = control->controllers->getFullController(myControllerIndex);
  
        filename->setValue(QString::fromStdString(cs.dylib_path));
        rate->setValue(cs.rate);
  
        QStringList free, chosen;
        bool found;

        for (unsigned int i = 0; i < cs.sensors.size(); i++) 
          for (unsigned int j = 0; j < mySensors.size(); j++)
            if (mySensors[j].index == cs.sensors[i]) {
              chosen << QString::fromStdString(mySensors[j].name);
              break;
            }
        for (unsigned int i = 0; i < mySensors.size(); i++) {
          found = false;
          for (unsigned int j = 0; j < chosen.size() && !found; j++)
            if (mySensors[i].name == chosen[j].toStdString()) 
              found = true;
          if (found == false)
            free << QString::fromStdString(mySensors[i].name);
        }
	
        sensorDialog->reset(free, chosen);
        free.clear();
        chosen.clear();
  
        for (unsigned int i = 0; i < cs.motors.size(); i++) 
          for (unsigned int j = 0; j < myMotors.size(); j++)
            if (myMotors[j].index == cs.motors[i]) {
              chosen << QString::fromStdString(myMotors[j].name);
              break;
            }
        for (unsigned int i = 0; i != myMotors.size(); i++) {
          found = false;
          for (unsigned int j = 0; j < chosen.size() && !found; j++)
            if (myMotors[i].name == chosen[j].toStdString()) 
              found = true;
          if (found == false)
            free << QString::fromStdString(myMotors[i].name);
        }
	
        motorDialog->reset(free, chosen);
        free.clear();
        chosen.clear();

        for (unsigned int i = 0; i < cs.sNodes.size(); i++) 
          for (unsigned int j = 0; j < myNodes.size(); j++)
            if (myNodes[j].index == cs.sNodes[i]) {
              chosen << QString::fromStdString(myNodes[j].name);
              break;
            }
        for (unsigned int i = 0; i < myNodes.size(); i++) {
          found = false;
          for (unsigned int j = 0; j < chosen.size() && !found; j++)
            if (myNodes[i].name == chosen[j].toStdString()) 
              found = true;
          if (found == false)
            free << QString::fromStdString(myNodes[i].name);
        }

        nodeDialog->reset(free, chosen);
      }

    }



    void ControllerHandler::choose_nodes(QString list) {
      nodes->setValue(list);
      QStringList items = list.split(";");
      chosenNodes.clear();
      for (unsigned int i = 0; i < items.size(); i++)
        for (unsigned int j = 0; j < myNodes.size(); j++)
          if (items[i] == QString::fromStdString(myNodes[j].name)) {
            chosenNodes.push_back(myNodes[j]);
            break;
          }
    }


    void ControllerHandler::choose_sensors(QString list) {
      sensors->setValue(list);
      QStringList items = list.split(";");
      chosenSensors.clear();
      for (unsigned int i = 0; i < items.size(); i++)
        for (unsigned int j = 0; j < mySensors.size(); j++)
          if (items[i] == QString::fromStdString(mySensors[j].name)) {
            chosenSensors.push_back(mySensors[j]);
            break;
          }
    }


    void ControllerHandler::choose_motors(QString list) {
      motors->setValue(list);
      QStringList items = list.split(";");
      chosenMotors.clear();
      for (unsigned int i = 0; i < items.size(); i++)
        for (unsigned int j = 0; j < myMotors.size(); j++)
          if (items[i] == QString::fromStdString(myMotors[j].name)) {
            chosenMotors.push_back(myMotors[j]);
            break;
          }
    }


    void ControllerHandler::accept() {
      if (mode == ControllerTree::PreviewMode) {
        std::vector<unsigned long> result_motors;
        for(unsigned int i=0;i<chosenMotors.size();i++){
          result_motors.push_back(chosenMotors[i].index);
        }
      
        std::vector<unsigned long> result_sensors;
        for(unsigned int i=0;i<chosenSensors.size();i++){
          result_sensors.push_back(chosenSensors[i].index);
        }  
      
        std::vector<unsigned long> result_nodes;
        for(unsigned int i=0;i<chosenNodes.size();i++){
          result_sensors.push_back(chosenNodes[i].index);
        }  

        interfaces::ControllerData newController;
        newController.motors = result_motors;
        newController.sensors = result_sensors;
        newController.rate = rate->value().toInt();

        control->controllers->addController(newController);
        controllerName = QString::number(myControllerIndex).toStdString() + ":"+actualName;
        topLevelController->setPropertyName(QString::fromStdString(controllerName));
        mode = ControllerTree::EditMode;
        pDialog->setPropertyColor(topLevelController, editColor);  
      }
    }


    void ControllerHandler::nodes_selection() {
      nodeDialog->show();
    }


    void ControllerHandler::motors_selection() {
      motorDialog->show();
    }


    void ControllerHandler::sensors_selection() {
      sensorDialog->show();
    }

  } // end of namespace gui
} // end of namespace mars
