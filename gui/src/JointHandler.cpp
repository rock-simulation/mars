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


#include "JointHandler.h"
#include "Widget_Joint_State.h"
#include <iostream>
#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/interfaces/sim/JointManagerInterface.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>

#include <QMessageBox>

namespace mars {
  namespace gui {

    using namespace std;

    JointHandler::JointHandler(QtVariantProperty* property, unsigned long index,
                               main_gui::PropertyDialog *pd, 
                               interfaces::ControlCenter *c, JointTree::Mode m,
                               std::string imagePath_) : imagePath(imagePath_) {
      state_on = false;
      mode = m;
      control = c;
      topLevelJoint = property;
      actualName = topLevelJoint->propertyName().toStdString();
      myJointIndex = (int) index;
      jointName = QString::number(index).toStdString() + ":" + actualName;
      topLevelJoint->setPropertyName(QString::fromStdString(jointName));
      pDialog = pd;
      filled = false;
      editColor = pDialog->getPropertyColor(topLevelJoint);
      previewColor = QColor(255, 200, 200);

      previewLabel = new QLabel(pDialog);
      previewLabel->setWindowFlags(Qt::Window);
      previewLabel->setWindowTitle(tr("Joint Preview"));
      previewLabel->setFixedSize(200, 200);
    }



    void JointHandler::fill() {

      control->nodes->getListNodes(&allNodes);
      int first_node, second_node=0;
 
      if (mode == JointTree::PreviewMode) {
        pDialog->setPropertyColor(topLevelJoint, previewColor);
        //NEW_JOINT_STRUCT(myJoint);
        interfaces::JointData myJoint;
        first_node = second_node = 0;
      } else {
        myJoint = control->joints->getFullJoint(myJointIndex);
        for (unsigned int i=0; i<allNodes.size();i++){
          if (myJoint.nodeIndex1 == allNodes[i].index)
            first_node = i;
          if (myJoint.nodeIndex2 == allNodes[i].index)
            second_node = i+1;
        }
      }



      posType = interfaces::ANCHOR_CUSTOM;
      std::map<QString, QVariant> attr;
      attr.insert(pair<QString, QVariant>(QString("decimals"), 9));
      attr.insert(pair<QString, QVariant>(QString("singleStep"), 0.1));
      QStringList enumNames;
      enumNames << "Hinge" << "Hinge 2" << "Slider" << "Ball and Socket" << "Universal" ;
  
      type = pDialog->addGenericProperty("../"+jointName+"/General/Type", QtVariantPropertyManager::enumTypeId(),
                                         myJoint.type-1, NULL, &enumNames);
      enumNames.clear();
      for (unsigned int i = 0; i <allNodes.size(); i++)
        enumNames << QString::fromStdString(allNodes[i].name);
  
      general = type;
      first = pDialog->addGenericProperty("../"+jointName+"/General/First Node", QtVariantPropertyManager::enumTypeId(),
                                          first_node, NULL, &enumNames);
      enumNames.clear();
      enumNames << "Air";
      for (unsigned int i = 0; i <allNodes.size(); i++)
        enumNames << QString::fromStdString(allNodes[i].name);
      second = pDialog->addGenericProperty("../"+jointName+"/General/Second Node", QtVariantPropertyManager::enumTypeId(),
                                           second_node, NULL, &enumNames);
      enumNames.clear();
      enumNames << "Custom" << "Center" << "First" << "Second";
      anchor = pDialog->addGenericProperty("../"+jointName+"/Anchor", QtVariantPropertyManager::groupTypeId(), 0);
      a_pos = pDialog->addGenericProperty("../"+jointName+"/Anchor/Anchor Position", QtVariantPropertyManager::enumTypeId(),
                                          QVariant(0), NULL, &enumNames);
      center_x = pDialog->addGenericProperty("../"+jointName+"/Anchor/Joint Center/x", QVariant::Double, 0.0, &attr);
      center_y = pDialog->addGenericProperty("../"+jointName+"/Anchor/Joint Center/y", QVariant::Double, 0.0, &attr);
      center_z = pDialog->addGenericProperty("../"+jointName+"/Anchor/Joint Center/z", QVariant::Double, 0.0, &attr);
      axis1 = pDialog->addGenericProperty("../"+jointName+"/Anchor/Axis 1", QtVariantPropertyManager::groupTypeId(), 0);
      alpha1 = pDialog->addGenericProperty("../"+jointName+"/Anchor/Axis 1/x", QVariant::Double, 1.0, &attr);
      beta1 = pDialog->addGenericProperty("../"+jointName+"/Anchor/Axis 1/y", QVariant::Double, 0.0, &attr);
      gamma1 = pDialog->addGenericProperty("../"+jointName+"/Anchor/Axis 1/z", QVariant::Double, 0.0, &attr);
      axis2 = pDialog->addGenericProperty("../"+jointName+"/Anchor/Axis 2", QtVariantPropertyManager::groupTypeId(), 0);
      alpha2 = pDialog->addGenericProperty("../"+jointName+"/Anchor/Axis 2/x", QVariant::Double, 0.0, &attr);
      beta2 = pDialog->addGenericProperty("../"+jointName+"/Anchor/Axis 2/y", QVariant::Double, 1.0, &attr);
      gamma2 = pDialog->addGenericProperty("../"+jointName+"/Anchor/Axis 2/z", QVariant::Double, 0.0, &attr);
      enable_con = pDialog->addGenericProperty("../"+jointName+"/Constraints", QVariant::Bool, false);
      low_stop = pDialog->addGenericProperty("../"+jointName+"/Constraints/Low Stop",
                                             QtVariantPropertyManager::groupTypeId(), 0);
      low1 = pDialog->addGenericProperty("../"+jointName+"/Constraints/Low Stop/Low Stop Axis 1", QVariant::Double, 0, &attr);
      low2 = pDialog->addGenericProperty("../"+jointName+"/Constraints/Low Stop/Low Stop Axis 2", QVariant::Double, 0, &attr);
      high_stop = pDialog->addGenericProperty("../"+jointName+"/Constraints/High Stop",
                                              QtVariantPropertyManager::groupTypeId(), 0);
      high1 = pDialog->addGenericProperty("../"+jointName+"/Constraints/High Stop/High Stop Axis 1", QVariant::Double, 0, &attr);
      high2 = pDialog->addGenericProperty("../"+jointName+"/Constraints/High Stop/High StopAxis 2", QVariant::Double, 0, &attr);
      con_damp = pDialog->addGenericProperty("../"+jointName+"/Constraints/Damping Constant",
                                             QtVariantPropertyManager::groupTypeId(), 0);
      damp1 = pDialog->addGenericProperty("../"+jointName+"/Constraints/Damping Constant/Damping Axis 1", 
                                          QVariant::Double, 0, &attr);
      damp2 = pDialog->addGenericProperty("../"+jointName+"/Constraints/Damping Constant/Damping Axis 2", 
                                          QVariant::Double, 0, &attr);
      con_spring = pDialog->addGenericProperty("../"+jointName+"/Constraints/Spring Constant",
                                               QtVariantPropertyManager::groupTypeId(), 0);
      spring1 = pDialog->addGenericProperty("../"+jointName+"/Constraints/Spring Constant/Spring Axis 1", 
                                            QVariant::Double, 0, &attr);
      spring2 = pDialog->addGenericProperty("../"+jointName+"/Constraints/Spring Constant/Spring Axis 2",
                                            QVariant::Double, 0, &attr);
      constraints = enable_con;

      center_x->setValue(myJoint.anchor.x());
      center_y->setValue(myJoint.anchor.y());
      center_z->setValue(myJoint.anchor.z());
  
      alpha1->setValue(myJoint.axis1.x());
      beta1->setValue(myJoint.axis1.y());
      gamma1->setValue(myJoint.axis1.z());
	
      alpha2->setValue(myJoint.axis2.x());
      beta2->setValue(myJoint.axis2.y());
      gamma2->setValue(myJoint.axis2.z());

      //name->setValue(QString::fromStdString(myJoint.name));

      low1->setValue(myJoint.lowStopAxis1);
      high1->setValue(myJoint.highStopAxis1);
      damp1->setValue(myJoint.damping_const_constraint_axis1);
      spring1->setValue(myJoint.spring_const_constraint_axis1);
      low2->setValue(myJoint.lowStopAxis2);
      high2->setValue(myJoint.highStopAxis2);
      damp2->setValue(myJoint.damping_const_constraint_axis2);
      spring2->setValue(myJoint.spring_const_constraint_axis2);

      switch(type->value().toInt()+1) {
      case interfaces::JOINT_TYPE_BALL: // ball and socket
        type->setValue("Ball-and-socket");
        anchor->removeSubProperty(axis2);
        low_stop->removeSubProperty(low2);
        high_stop->removeSubProperty(high2);
        con_damp->removeSubProperty(damp2);
        con_spring->removeSubProperty(spring2);
        double_axis = false;
        break;
      
      case interfaces::JOINT_TYPE_HINGE: // hinge
        type->setValue("Hinge");
        anchor->removeSubProperty(axis2);
        low_stop->removeSubProperty(low2);
        high_stop->removeSubProperty(high2);
        con_damp->removeSubProperty(damp2);
        con_spring->removeSubProperty(spring2);
        double_axis = false;
        break;
      
      case interfaces::JOINT_TYPE_SLIDER: // slider
        type->setValue("Slider");
        anchor->removeSubProperty(axis2);
        low_stop->removeSubProperty(low2);
        high_stop->removeSubProperty(high2);
        con_damp->removeSubProperty(damp2);
        con_spring->removeSubProperty(spring2);
        double_axis = false;
        break;
      
      case interfaces::JOINT_TYPE_UNIVERSAL: //universal
        type->setValue("Universal");
        anchor->insertSubProperty(axis2, axis1);
        low_stop->addSubProperty(low2);
        high_stop->addSubProperty(high2);
        con_damp->addSubProperty(damp2);
        con_spring->addSubProperty(spring2);
        double_axis = true;
        break;
      
      case interfaces::JOINT_TYPE_HINGE2: // hinge2
        type->setValue("Hinge 2");
        anchor->insertSubProperty(axis2, axis1);
        low_stop->addSubProperty(low2);
        high_stop->addSubProperty(high2);
        con_damp->addSubProperty(damp2);
        con_spring->addSubProperty(spring2);
        double_axis = true;
        break;
      }

      if (myJoint.lowStopAxis1 == 0.0 && myJoint.highStopAxis1 == 0.0 && 
          myJoint.lowStopAxis2 == 0.0 && myJoint.highStopAxis2 == 0.0) 
        enable_con->setValue(false);    
      else 
        enable_con->setValue(true);
  
      on_change_type(type->value().toInt());
      on_check_constraints(enable_con->value().toBool());

      filled = true;
    }


  
    JointHandler::~JointHandler() {
      previewOff();
      pDialog->removeGenericProperty(topLevelJoint);
    }





    int JointHandler::accept() {
      if (mode == JointTree::EditMode)
        return 1;

      utils::Vector anchor;
      utils::Vector axis1;
      utils::Vector axis2;

      myJoint.name = allNodes[first->value().toInt()].name;
      myJoint.name.append(" - ");
      if (second->value().toInt() > 0){
        myJoint.name.append(allNodes[second->value().toInt()-1].name);
      }
      else myJoint.name.append("AIR");
  
      // set attached nodes
      myJoint.nodeIndex1 = allNodes[first->value().toInt()].index;
  
      if (second->value().toInt() > 0) {
        myJoint.nodeIndex2 = allNodes[second->value().toInt()-1].index;
      }
      else myJoint.nodeIndex2 = 0;
  
      // set anchor position
      myJoint.type = static_cast<interfaces::JointType>(type->value().toInt() + 1);
  
      anchor.x() = center_x->value().toDouble();
      anchor.y() = center_y->value().toDouble();
      anchor.z() = center_z->value().toDouble();
	
      // set  axis1 direction
      axis1.x() = alpha1->value().toDouble();
      axis1.y() = beta1->value().toDouble();
      axis1.z() = gamma1->value().toDouble();
	
      if (axis1.x() == 0 && axis1.y() == 0 && axis1.z() == 0) {
        QMessageBox::information(0, "Simulation",
                                 "Please select anchor axis 1", "OK", 0);
        return 1;
      }

      // set axis2 direction
      axis2.x() = alpha2->value().toDouble();
      axis2.y() = beta2->value().toDouble();
      axis2.z() = gamma2->value().toDouble();
	
      if ((axis2.x() == 0 && axis2.y() == 0 && axis2.z() == 0) &&
          (myJoint.type == interfaces::JOINT_TYPE_UNIVERSAL ||
           myJoint.type == interfaces::JOINT_TYPE_HINGE2)) {
        QMessageBox::information(0, "Simulation",
                                 "Please select anchor axis 2", "OK", 0);
        return 1;
      }

      myJoint.anchor = anchor;
      myJoint.axis1 = axis1;
      myJoint.axis2 = axis2;
  
      // set the spring and damping constants  
      myJoint.spring_constant = 0;
      myJoint.damping_constant = 0;
  
      // set the constraints
      if (enable_con->value().toBool() == true){
        myJoint.lowStopAxis1 = low1->value().toDouble();
        myJoint.highStopAxis1 = high1->value().toDouble();
        myJoint.damping_const_constraint_axis1 = damp1->value().toDouble(); 
        myJoint.spring_const_constraint_axis1 = spring1->value().toDouble();
        myJoint.lowStopAxis2 = low2->value().toDouble();
        myJoint.highStopAxis2 = high2->value().toDouble();
        myJoint.damping_const_constraint_axis2 = damp2->value().toDouble(); 
        myJoint.spring_const_constraint_axis2 = spring2->value().toDouble();
      }
      else{
        myJoint.lowStopAxis1 = 0.0;
        myJoint.highStopAxis1 = 0.0;
        myJoint.damping_const_constraint_axis1 = 0;
        myJoint.spring_const_constraint_axis1 = 0;
        myJoint.lowStopAxis2 = 0.0;
        myJoint.highStopAxis2 = 0.0;
        myJoint.damping_const_constraint_axis2 = 0;
        myJoint.spring_const_constraint_axis2 = 0;
      }
      myJoint.anchorPos = posType;
      // we have to be carful because after this function the
      // joint will be freed
      control->joints->addJoint(&myJoint);

      myJointIndex = (int) myJoint.index;
      actualName = myJoint.name;
      jointName = QString::number(myJointIndex).toStdString() + ":" + actualName;
      topLevelJoint->setPropertyName(QString::fromStdString(jointName));
  
      pDialog->setPropertyColor(topLevelJoint, editColor);  
  
      mode = JointTree::EditMode;
      //  on_type_changed();
      return 0;
    }






    void JointHandler::previewOn() {
      drawAxis();
      if (mode == JointTree::EditMode || filled == 0) 
        return; // if not a preview
    }

    void JointHandler::previewOff() {
      control->graphics->closeAxis();
      previewLabel->hide();
      if (mode == JointTree::EditMode || filled == false) 
        return; // if not a preview

    }




    void JointHandler::valueChanged(QtProperty *property, const QVariant &value)
    {
      if (filled == false) return;

      if (property == type)
        on_change_type(value.toInt());

      else if (property == a_pos || property == first || property == second) 
        on_set_anchor();
   
      else if (property == enable_con)
        on_check_constraints(value.toBool());
  
      else if (property == center_x || property == center_y || property == center_z)
        userEdited = true;
  
      drawAxis();

      if (mode == JointTree::EditMode)		
        update();
    }



    void JointHandler::update() {

      if (mode == JointTree::PreviewMode)
        return;

      utils::Vector anchor;
      utils::Vector axis1;
      utils::Vector axis2;

      anchor.x() = center_x->value().toDouble();
      anchor.y() = center_y->value().toDouble();
      anchor.z() = center_z->value().toDouble();
	
      // set  axis1 direction
      axis1.x() = alpha1->value().toDouble();
      axis1.y() = beta1->value().toDouble();
      axis1.z() = gamma1->value().toDouble();

      if (axis1.x() == 0 && axis1.y() == 0 && axis1.z() == 0) {
        QMessageBox::information(0, "Simulation",
                                 "Please select anchor axis 1", "OK", 0);
        return;
      }
  
      // set axis2 direction
      axis2.x() = alpha2->value().toDouble();
      axis2.y() = beta2->value().toDouble();
      axis2.z() = gamma2->value().toDouble();
	  
      if ((axis2.x() == 0 && axis2.y() == 0 && axis2.z() == 0) &&
          (type->value().toString() == "Universal" ||
           type->value().toString() == "Hinge 2")) {
        QMessageBox::information(0, "Simulation",
                                 "Please select anchor axis 2", "OK", 0);
        return;
      }

      myJoint.anchor = anchor;
      myJoint.axis1 = axis1;
      myJoint.axis2 = axis2;

      // set the constraints
      if (enable_con->value().toBool() == true){
        myJoint.lowStopAxis1 = low1->value().toDouble();
        myJoint.highStopAxis1 = high1->value().toDouble();
        myJoint.damping_const_constraint_axis1 = damp1->value().toDouble(); 
        myJoint.spring_const_constraint_axis1 = spring1->value().toDouble();
        myJoint.lowStopAxis2 = low2->value().toDouble();
        myJoint.highStopAxis2 = high2->value().toDouble();
        myJoint.damping_const_constraint_axis2 = damp2->value().toDouble(); 
        myJoint.spring_const_constraint_axis2 = spring2->value().toDouble();
      }
      else{
        myJoint.lowStopAxis1 = 0.0;
        myJoint.highStopAxis1 = 0.0;
        myJoint.damping_const_constraint_axis1 = 0;
        myJoint.spring_const_constraint_axis1 = 0;
        myJoint.lowStopAxis2 = 0.0;
        myJoint.highStopAxis2 = 0.0;
        myJoint.damping_const_constraint_axis2 = 0;
        myJoint.spring_const_constraint_axis2 = 0;
      }
      myJoint.anchorPos = posType;  

      control->joints->editJoint(&myJoint);

    }





    void JointHandler::focusIn() {
      if (filled == false) { 
        fill();
        previewOn();
        //    on_type_changed();
      }
    }


    void JointHandler::focusOut() {
      if (filled == true) {
        previewOff();
        pDialog->destroyAllSubProperties(topLevelJoint);  
        top_props.clear();
        filled = false;
      }
    }


    /** 
     * draw preview axis via calling MainWindow->drawAxis with the calculated
     * line positions
     */
    void JointHandler::drawAxis(){
      utils::Vector first;
      utils::Vector second;
      utils::Vector third;
      utils::Vector axis1;
      utils::Vector axis2;
      bool bFirst = false;
      bool bSecond = false;
  
      // first -> second -> third
      // body1 -> anchor -> body2

      if (mode == JointTree::EditMode)
        //get node positions
        for (unsigned int i=0; i<allNodes.size();i++){
          if (myJoint.nodeIndex1 == allNodes[i].index){
            first = allNodes[i].pos;
            bFirst = true;
          }
          if (myJoint.nodeIndex2 == allNodes[i].index){
            third = allNodes[i].pos;
            bSecond =true;
          }
        }
      else { // PreviewMode
        first = allNodes[this->first->value().toInt()].pos;
        if (this->second->value().toInt() != 0) {
          //-1 because of the "None" entry
          third = allNodes[this->second->value().toInt()-1].pos;
        }
        else {
          third.x() = 0.0;
          third.y() = 0.0;
          third.z() = 0.0;
        }
      }

  
      //second position is the anchor
      second.x() = center_x->value().toDouble();
      second.y() = center_y->value().toDouble();
      second.z() = center_z->value().toDouble();
  
      axis1.x() = alpha1->value().toDouble();
      axis1.y() = beta1->value().toDouble();
      axis1.z() = gamma1->value().toDouble();
		
      // set axis2 direction
      axis2.x() = alpha2->value().toDouble();
      axis2.y() = beta2->value().toDouble();
      axis2.z() = gamma2->value().toDouble();
  
      control->graphics->drawAxis(first,second,third, axis1, axis2);
      if (userEdited)
        posType = interfaces::ANCHOR_CUSTOM;
    }






    /** switches constraint fields on and off */
    void JointHandler::on_check_constraints(bool checked){
      if (checked) {
        constraints->addSubProperty(low_stop);
        constraints->addSubProperty(high_stop);
        constraints->addSubProperty(con_damp);
        constraints->addSubProperty(con_spring);
        if (double_axis) {
          low_stop->addSubProperty(low2);
          high_stop->addSubProperty(high2);
          con_damp->addSubProperty(damp2);
          con_spring->addSubProperty(spring2);
        } else {
          low_stop->removeSubProperty(low2);
          high_stop->removeSubProperty(high2);
          con_damp->removeSubProperty(damp2);
          con_spring->removeSubProperty(spring2);
        }
      } else{
        constraints->removeSubProperty(low_stop);
        constraints->removeSubProperty(high_stop);
        constraints->removeSubProperty(con_damp);
        constraints->removeSubProperty(con_spring);
      }
    }

    /** displays joint images and edit fields for the selected joint type */
    void JointHandler::on_change_type(int index) {

      QString imagepath = QString::fromStdString(imagePath);

      // enable and disable axis and anchor edit fields according
      // to the selected joint type
      switch(index+1) {
      case 4: // ball and socket
        imagepath.append("/Ball-and-socket.jpg");
        anchor->removeSubProperty(axis2);
        low_stop->removeSubProperty(low2);
        high_stop->removeSubProperty(high2);
        con_damp->removeSubProperty(damp2);
        con_spring->removeSubProperty(spring2);
        double_axis = false;
        break;
      
      case 1: // hinge
        imagepath.append("/Hinge.jpg");
        anchor->removeSubProperty(axis2);
        low_stop->removeSubProperty(low2);
        high_stop->removeSubProperty(high2);
        con_damp->removeSubProperty(damp2);
        con_spring->removeSubProperty(spring2);
        double_axis = false;
        break;
      
      case 3: // slider
        imagepath.append("/Slider.jpeg");
        anchor->removeSubProperty(axis2);
        low_stop->removeSubProperty(low2);
        high_stop->removeSubProperty(high2);
        con_damp->removeSubProperty(damp2);
        con_spring->removeSubProperty(spring2);
        double_axis = false;
        break;
      
      case 5: //universal
        imagepath.append("/Universal.jpg");
        anchor->insertSubProperty(axis2, axis1);
        low_stop->addSubProperty(low2);
        high_stop->addSubProperty(high2);
        con_damp->addSubProperty(damp2);
        con_spring->addSubProperty(spring2);
        double_axis = true;
        break;
      
      case 2: // hinge2
        imagepath.append("/Hinge2.jpg");
        anchor->insertSubProperty(axis2, axis1);
        low_stop->addSubProperty(low2);
        high_stop->addSubProperty(high2);
        con_damp->addSubProperty(damp2);
        con_spring->addSubProperty(spring2);
        double_axis = true;
        break;
      }

      previewLabel->setPixmap(QPixmap::fromImage(QImage(imagepath)));
    }



    void JointHandler::on_reset_anchor(void) {
      center_x->setValue(0.0);
      center_y->setValue(0.0);
      center_z->setValue(0.0);
      alpha1->setValue(0.0);
      beta1->setValue(0.0);
      gamma1->setValue(0.0);
      alpha2->setValue(0.0);
      beta2->setValue(0.0);
      gamma2->setValue(0.0);
      posType = interfaces::ANCHOR_CUSTOM;
    }




    /** sets predefined anchor positions */
    void JointHandler::on_set_anchor() {
      //sets the anchor of the joint to the intended position.
      //0: custom
      //1: coordinate of the joints center
      //2: coordinate of first node
      //3: coordinate of the second node
      utils::Vector pos1, pos2;
      bool found = false;

      if (mode == JointTree::EditMode) {  
        for (unsigned int i=0; i<allNodes.size();i++){
          if (myJoint.nodeIndex1 == allNodes[i].index){
            pos1 = allNodes[i].pos;
          }
          if (myJoint.nodeIndex2 == allNodes[i].index){
            pos2 = allNodes[i].pos;
            found = true;
          }
        }
    
        if (found == false) { // means second node is AIR
          pos2.x() = 0.0;
          pos2.y() = 0.0;
          pos2.z() = 0.0;
        }
      }

      switch(a_pos->value().toInt()) {
      case 0:
        posType = interfaces::ANCHOR_CUSTOM;
        break;
      case 1:
        if (mode == JointTree::PreviewMode) {
          pos1 = allNodes[first->value().toInt()].pos;
          if (second->value().toInt() != 0) {
            //-1 because of the "None" entry
            pos2 = allNodes[second->value().toInt()-1].pos;
          }
          else {
            pos2.x() = 0.0;
            pos2.y() = 0.0;
            pos2.z() = 0.0;
          }
        }
        userEdited=false;
        center_x->setValue((pos1.x()+pos2.x())/2);
        center_y->setValue((pos1.y()+pos2.y())/2);
        center_z->setValue((pos1.z()+pos2.z())/2);
        posType = interfaces::ANCHOR_CENTER;
        break;
      case 2:		
        if (mode == JointTree::PreviewMode) 
          pos1 = allNodes[first->value().toInt()].pos;
        userEdited=false;
        center_x->setValue(pos1.x());
        center_y->setValue(pos1.y());
        center_z->setValue(pos1.z());
        posType = interfaces::ANCHOR_NODE1;
        break;
      case 3:
        if (mode == JointTree::PreviewMode) {
          if (second->value().toInt() != 0) {	
            //-1 because of the "AIR" entr
            pos2 = allNodes[second->value().toInt()-1].pos;
            userEdited=false;
            center_x->setValue(pos2.x());
            center_y->setValue(pos2.y());
            center_z->setValue(pos2.z());
          }
        } else { // EditMode
          if (found == true) {	
            userEdited=false;
            center_x->setValue(pos2.x());
            center_y->setValue(pos2.y());
            center_z->setValue(pos2.z());
          }      
        }
        posType = interfaces::ANCHOR_NODE2;
        break;
      }
    }



    void JointHandler::showState() {
      if (mode != JointTree::EditMode || state_on == true) 
        return;
      Widget_Joint_State *wjs = new Widget_Joint_State(pDialog, control, jointName +" State");
      wjs->connect(myJointIndex);
      connect(wjs->pDialog, SIGNAL(closeSignal()), this, SLOT(closeState()));
      state_on = true;
    }


    void JointHandler::closeState() {
      state_on = false;
    }

  } // end of namespace gui
} // end of namespace mars
