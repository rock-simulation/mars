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

#include "MotorHandler.h"

#include <mars/interfaces/sim/JointManagerInterface.h>
#include <mars/interfaces/sim/MotorManagerInterface.h>

namespace mars {
  namespace gui {

    static int local_ind = 0;

    MotorHandler::MotorHandler(QtVariantProperty* property, unsigned long index,
                               main_gui::PropertyDialog *pd, 
                               interfaces::ControlCenter *c, MotorTree::Mode m) 
    {
      mode = m;
      control = c;
      topLevelMotor = property;
      actualName = topLevelMotor->propertyName().toStdString();
      myMotorIndex = (int) index;
      motorName = QString::number(index).toStdString() + ":" + actualName;
      topLevelMotor->setPropertyName(QString::fromStdString(motorName));
      pDialog = pd;
      filled = false;
      editColor = pDialog->getPropertyColor(topLevelMotor);
      previewColor = QColor(255, 200, 200);
 
      control->joints->getListJoints(&joints);
      control->motors->getListMotors(&allMotors);
  
      if (mode == MotorTree::EditMode) 
        myMotor = control->motors->getFullMotor(myMotorIndex);
      else {
        myMotor.name = "NewMotor" + QString::number(local_ind++).toStdString();
        myMotor.maximumVelocity = myMotor.motorMaxForce = 999.0;
        myMotor.p = 1.0;
        pDialog->setPropertyColor(topLevelMotor, previewColor);
      }
  

    }

  
    MotorHandler::~MotorHandler() {
      pDialog->removeGenericProperty(topLevelMotor);
    }




    void MotorHandler::valueChanged(QtProperty* property, const QVariant& value) 
    {
      if (filled == false) 
        return;

      if (property == name) {
        actualName = value.toString().toStdString();
        motorName = QString::number(myMotor.index).toStdString()+":"+actualName;
        topLevelMotor->setPropertyName(QString::number(myMotor.index) + ":" + value.toString());
      }

      if (property == joint)
        on_change_joint(value.toInt());
  
      else if (property == type)
        on_change_type(value.toInt());
  
      updateMotor();
  
    }


    void MotorHandler::focusIn() {
      if (filled == false) { 
        fill();
        on_change_type(type->value().toInt()); 
        on_change_joint(joint->value().toInt());
        filled = true;
      }
    }


    void MotorHandler::focusOut() {
      if (filled == true) {
        pDialog->destroyAllSubProperties(topLevelMotor);  
        filled = false;
      }
    }

    void MotorHandler::fill() {

      std::map<QString, QVariant> attr;
      attr.insert(std::pair<QString, QVariant>(QString("decimals"), 9));
      attr.insert(std::pair<QString, QVariant>(QString("singleStep"), 0.1));
      QStringList enumNames;
  
  
      name = pDialog->addGenericProperty("../"+motorName+"/Name", QVariant::String, 
                                         QString::fromStdString(actualName));
  
      enumNames << "Linear" << "PID";
      int m_type = 0;
      if (myMotor.type == interfaces::MOTOR_TYPE_PID)
        m_type = 1;
      type = pDialog->addGenericProperty("../"+motorName+"/Type", QtVariantPropertyManager::enumTypeId(),
                                         m_type, NULL, &enumNames);
      velocity = pDialog->addGenericProperty("../"+motorName+"/Maximum Velocity", 
                                             QVariant::Double, myMotor.maximumVelocity);
      force = pDialog->addGenericProperty("../"+motorName+"/Maximum Force", 
                                          QVariant::Double, myMotor.motorMaxForce);
      p_value = pDialog->addGenericProperty("../"+motorName+"/P", 
                                            QVariant::Double, myMotor.p, &attr);
      i_value = pDialog->addGenericProperty("../"+motorName+"/I", 
                                            QVariant::Double, myMotor.i, &attr);
      d_value = pDialog->addGenericProperty("../"+motorName+"/D", 
                                            QVariant::Double, myMotor.d, &attr);

      enumNames.clear(); 
      int j_ind = 0;
      for (unsigned int i =0; i< joints.size(); i++) { 
        enumNames << QString::fromStdString(joints[i].name);
        if (joints[i].index == myMotor.jointIndex) 
          j_ind = i;
      }
  
      j_options = pDialog->addGenericProperty("../"+motorName+"/Joint Options", 
                                              QtVariantPropertyManager::groupTypeId(), 0);
      joint = pDialog->addGenericProperty("../"+motorName+"/Joint Options/Joint", 
                                          QtVariantPropertyManager::enumTypeId(),
                                          j_ind, NULL, &enumNames);
      j_type = pDialog->addGenericProperty("../"+motorName+"/Joint Options/Type", QVariant::String, "");
      enumNames.clear();
      enumNames << "Axis 1" << "Axis 2";
      if (myMotor.axis == 0) myMotor.axis = 1;
      axis = pDialog->addGenericProperty("../"+motorName+"/Joint Options/Axis", 
                                         QtVariantPropertyManager::enumTypeId(),
                                         myMotor.axis-1, NULL, &enumNames);
  
      //  j_type->setEnabled(false);
    }



    void MotorHandler::on_change_joint(int index) {
      switch (control->joints->getFullJoint(joints[index].index).type) {
      case interfaces::JOINT_TYPE_UNIVERSAL:
        j_options->addSubProperty(axis);
        j_type->setValue("Universal");
        break;
      case interfaces::JOINT_TYPE_HINGE2:
        j_options->addSubProperty(axis);
        j_type->setValue("Hinge 2");
        break;
      case interfaces::JOINT_TYPE_HINGE:
        j_options->removeSubProperty(axis);
        j_type->setValue("Hinge");
        break;
      case interfaces::JOINT_TYPE_SLIDER:
        j_options->removeSubProperty(axis);
        j_type->setValue("Slider");
        break;
      case interfaces::JOINT_TYPE_BALL:
        j_options->removeSubProperty(axis);
        j_type->setValue("Ball and Socket");
        break;
      default:
        break;    
      }
    }


    void MotorHandler::on_change_type(int index) {
      switch (index) {
      case 0:
        myMotor.type = interfaces::MOTOR_TYPE_DC;
        topLevelMotor->removeSubProperty(p_value);
        topLevelMotor->removeSubProperty(i_value);
        topLevelMotor->removeSubProperty(d_value);
        break;
      case 1:
        myMotor.type = interfaces::MOTOR_TYPE_PID;
        pDialog->insertGenericProperty(topLevelMotor, d_value, force);
        pDialog->insertGenericProperty(topLevelMotor, i_value, force);
        pDialog->insertGenericProperty(topLevelMotor, p_value, force);
        break;
      default:
        myMotor.type = interfaces::MOTOR_TYPE_UNDEFINED;
        break;
      }
    }


    void MotorHandler::updateMotor() {
      myMotor.name = name->value().toString().toStdString();
      myMotor.axis = axis->value().toInt()+1;
      myMotor.maximumVelocity = velocity->value().toDouble();
      myMotor.motorMaxForce = force->value().toDouble();
      myMotor.p = p_value->value().toDouble();
      myMotor.i = i_value->value().toDouble();
      myMotor.d = d_value->value().toDouble();
      myMotor.jointIndex = joints[joint->value().toInt()].index;
  
      if (mode == MotorTree::EditMode) 
        control->motors->editMotor(myMotor);

    }



    void MotorHandler::accept() {
      if (mode == MotorTree::PreviewMode) {
        updateMotor();
        control->motors->addMotor(&myMotor);  
        myMotorIndex = (int) myMotor.index;
        actualName = myMotor.name;
        motorName = QString::number(myMotorIndex).toStdString() + ":"+actualName;
        topLevelMotor->setPropertyName(QString::fromStdString(motorName));
        mode = MotorTree::EditMode;
        pDialog->setPropertyColor(topLevelMotor, editColor);  
      }
    }

  } // end of namespace gui
} // end of namespace mars
