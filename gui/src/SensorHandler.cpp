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

#include "SensorHandler.h"

#include <mars/interfaces/sim/SensorManagerInterface.h>

namespace mars {
  namespace gui {

    static int local_ind = 0;

    SensorHandler::SensorHandler(QtVariantProperty* property, unsigned long index,
                                 main_gui::PropertyDialog *pd, 
                                 interfaces::ControlCenter *c, SensorTree::Mode m) 
    {
      mode = m;
      control = c;
      topLevelSensor = property;
      actualName = topLevelSensor->propertyName().toStdString();
      mySensorIndex = (int) index;
      sensorName = QString::number(index).toStdString() + ":" + actualName;
      topLevelSensor->setPropertyName(QString::fromStdString(sensorName));
      pDialog = pd;
      filled = false;
      editColor = pDialog->getPropertyColor(topLevelSensor);
      previewColor = QColor(255, 200, 200);
 
      control->sensors->getListSensors(&allSensors);
 
#warning Work needed
#if 0

      if (mode == SensorTree::EditMode) 
        mySensor = control->sensors->getFullSensor(mySensorIndex);
      else {
        vector<unsigned long> chosen;
        mySensor.name = "NewSensor" + QString::number(local_ind++).toStdString();
        mySensor.rate = 20;
        mySensor.sensor_type = SENSOR_TYPE_TORQUE_AVG;
        mySensor.indices = chosen;
        mySensor.id = 0;
        pDialog->setPropertyColor(topLevelSensor, previewColor);
      }
      fill();
      filled = true;
      on_change_type(mySensor.sensor_type);
#endif
    }

  
    SensorHandler::~SensorHandler() {
      pDialog->removeGenericProperty(topLevelSensor);
    }




    void SensorHandler::valueChanged(QtProperty* property, const QVariant& value) 
    {
#warning Work needed
#if 0
      if (filled == false) 
        return;

      if (property == name) {
        actualName = value.toString().toStdString();
        sensorName = QString::number(mySensor.id).toStdString()+":"+actualName;
        topLevelSensor->setPropertyName(QString::number(mySensor.id) + ":" + value.toString());
      }

      else if (property == type)
        on_change_type(value.toInt()+1); 
 
      updateSensor();
#endif
    }


    void SensorHandler::focusIn() {

    }


    void SensorHandler::focusOut() {

    }

    void SensorHandler::fill() {
#warning work needed
#if 0
      QStringList enumNames;
      enumNames << "Joint Torque AVG" << "Joint Load" << "Node Position" << "Node Rotation";
      enumNames << "Ground Contact" << "Ground Contact Force" << "Center of Gravity";
      enumNames << "Potentiometer" << "Camera" << "Node Velocity" << "Ray" << "Ray Grid";
      enumNames << "Node Angular Velocity" << "Motor Current" << "6DOF" << "Joint Torque";
      enumNames << "Motor Position"<< "Joint Velocity";
      std::map<QString, QVariant> attr;
      attr.insert(pair<QString, QVariant>(QString("minimum"), 0));
  
      name = pDialog->addGenericProperty("../"+sensorName+"/Name", QVariant::String, 
                                         QString::fromStdString(actualName));
  
      type = pDialog->addGenericProperty("../"+sensorName+"/Type", QtVariantPropertyManager::enumTypeId(),
                                         mySensor.sensor_type-1, NULL, &enumNames);
      objects = pDialog->addGenericProperty("../"+sensorName+"/Objects", QVariant::String, "");
      update_rate = pDialog->addGenericProperty("../"+sensorName+"/Update rate", QVariant::Int,
                                                mySensor.rate, &attr);

      control->nodes->getListNodes(&myNodes);
      control->joints->getListJoints(&myJoints);
      control->motors->getListMotors(&myMotors);
      QStringList items;
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

      items.clear();
      for(unsigned int i = 0;i<myJoints.size();i++)
        items << QString::fromStdString(myJoints[i].name);

      jointDialog = new SelectionDialog(pDialog);
      jointDialog->reset(items);
      jointDialog->setWindowFlags(Qt::Window);
      jointDialog->setWindowTitle(tr("Select Joints"));

      QObject::connect(nodeDialog, SIGNAL(modified(QString)), this, SLOT(choose_nodes(QString)));
      QObject::connect(jointDialog, SIGNAL(modified(QString)), this, SLOT(choose_joints(QString)));
      QObject::connect(motorDialog, SIGNAL(modified(QString)), this, SLOT(choose_motors(QString)));
#endif
    }




    void SensorHandler::on_change_type(int index) {
#warning Work needed
#if 0
      chosenOnes.clear();
      QStringList items;
      objects->setValue("");
  
      for (unsigned int i = 0; i < myMotors.size(); i++)
        items << QString::fromStdString(myMotors[i].name);
      motorDialog->reset(items);

      items.clear();
      for(unsigned int i = 0;i<myNodes.size();i++)
        items << QString::fromStdString(myNodes[i].name);
      nodeDialog->reset(items);

      items.clear();
      for(unsigned int i = 0;i<myJoints.size();i++)
        items << QString::fromStdString(myJoints[i].name);
      jointDialog->reset(items);

      switch (index) {
      case SENSOR_TYPE_TORQUE_AVG:
      case SENSOR_TYPE_LOAD:
      case SENSOR_TYPE_JOINT_POSITION:
      case SENSOR_TYPE_6DOF:
      case SENSOR_TYPE_TORQUE:
      case SENSOR_TYPE_JOINT_VELOCITY:
        objects->setPropertyName("Joints");
        nodeDialog->hide();
        motorDialog->hide();
        break;
      case SENSOR_TYPE_POSITION:
      case SENSOR_TYPE_ROTATION:
      case SENSOR_TYPE_GROUND_CONTACT:
      case SENSOR_TYPE_GROUND_CONTACT_FORCE:
      case SENSOR_TYPE_GROUND_CENTER_OF_GRAVITY:
      case SENSOR_TYPE_CAMERA:
      case SENSOR_TYPE_VELOCITY:
      case SENSOR_TYPE_RAY:
      case SENSOR_TYPE_RAY_GRID:
      case SENSOR_TYPE_ANGULAR_VELOCITY:
        objects->setPropertyName("Nodes");
        motorDialog->hide();
        jointDialog->hide();
        break;
      case SENSOR_TYPE_MOTOR_CURRENT:
      case SENSOR_TYPE_MOTOR_POSITION:
        objects->setPropertyName("Motors");
        nodeDialog->hide();
        jointDialog->hide();
        break;    
      }
#endif
    }


    void SensorHandler::updateSensor() {
#warning work needed
#if 0
      vector<unsigned long> result_objects;
      for(unsigned int i=0;i<chosenOnes.size();i++){
        result_objects.push_back(chosenOnes[i].index);
      }
      mySensor.name = name->value().toString().toStdString();
      mySensor.rate = update_rate->value().toInt();
      mySensor.sensor_type = static_cast<base::SensorType>(type->value().toInt()+1);
      mySensor.indices = result_objects;

      if (mode == SensorTree::EditMode) {
        // control->sensors->editSensor(mySensor) 
      }
#endif
    }


    void SensorHandler::choose_nodes(QString list) {
      objects->setValue(list);
      QStringList items = list.split(";");
      chosenOnes.clear();
      for (unsigned int i = 0; i < (unsigned int)items.size(); i++)
        for (unsigned int j = 0; j < myNodes.size(); j++)
          if (items[i] == QString::fromStdString(myNodes[j].name)) {
            chosenOnes.push_back(myNodes[j]);
            break;
          }
    }


    void SensorHandler::choose_joints(QString list) {
      objects->setValue(list);
      QStringList items = list.split(";");
      chosenOnes.clear();
      for (unsigned int i = 0; i < (unsigned int)items.size(); i++)
        for (unsigned int j = 0; j < myJoints.size(); j++)
          if (items[i] == QString::fromStdString(myJoints[j].name)) {
            chosenOnes.push_back(myJoints[j]);
            break;
          }
    }


    void SensorHandler::choose_motors(QString list) {
      objects->setValue(list);
      QStringList items = list.split(";");
      chosenOnes.clear();
      for (unsigned int i = 0; i < (unsigned int)items.size(); i++)
        for (unsigned int j = 0; j < myMotors.size(); j++)
          if (items[i] == QString::fromStdString(myMotors[j].name)) {
            chosenOnes.push_back(myMotors[j]);
            break;
          }
    }


    void SensorHandler::accept() {
#warning work needed
#if 0
      if (mode == SensorTree::PreviewMode) {
        if(mySensor.sensor_type == SENSOR_TYPE_CAMERA){
          cameraStruct* _data= (cameraStruct*) malloc(sizeof(cameraStruct));
          control->graphics->getCameraInfo(_data);
          //mySensor.data=_data; //TODO IMPORTANT
        }	
        control->sensors->addSensor(&mySensor);
        mySensorIndex = (int) mySensor.id;
        actualName = mySensor.name;
        sensorName = QString::number(mySensorIndex).toStdString() + ":"+actualName;
        topLevelSensor->setPropertyName(QString::fromStdString(sensorName));
        mode = SensorTree::EditMode;
        pDialog->setPropertyColor(topLevelSensor, editColor);  
      }
#endif
    }


    void SensorHandler::selection() {
      if(!filled)
        return;
      jointDialog->hide();
      motorDialog->hide();
      if (objects->propertyName() == "Nodes")
        nodeDialog->show();
      else if (objects->propertyName() == "Joints")
        jointDialog->show();
      else if (objects->propertyName() == "Motors")
        motorDialog->show();
    }

  } // end of namespace gui
} // end of namespace mars
