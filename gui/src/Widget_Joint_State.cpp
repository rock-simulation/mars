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

#include "Widget_Joint_State.h"
#include <mars/interfaces/utils.h>

#include <mars/interfaces/sim/JointManagerInterface.h>

namespace mars {
  namespace gui {

    Widget_Joint_State::Widget_Joint_State(main_gui::PropertyDialog *par, 
                                           interfaces::ControlCenter* c, std::string name)
      : pDialog(new main_gui::PropertyDialog(par)), control(c) {
  
      pDialog->setAttribute(Qt::WA_DeleteOnClose);
      pDialog->setWindowTitle(QString::fromStdString(name));
      pDialog->setPropCallback(dynamic_cast<main_gui::PropertyCallback*>(this));
      pDialog->clearButtonBox();
      pDialog->addGenericButton("OK", this, SLOT(accept()));
      pDialog->show();
  
      actualJoint = 0;

      anchor_x = pDialog->addGenericProperty("../Anchor /x", QVariant::Double, 0);
      anchor_y = pDialog->addGenericProperty("../Anchor /y", QVariant::Double, 0);
      anchor_z = pDialog->addGenericProperty("../Anchor /z", QVariant::Double, 0);	
      axis1_x = pDialog->addGenericProperty("../Axis 1 /x", QVariant::Double, 0);
      axis1_y = pDialog->addGenericProperty("../Axis 1 /y", QVariant::Double, 0);
      axis1_z = pDialog->addGenericProperty("../Axis 1 /z", QVariant::Double, 0);
      axis2_x = pDialog->addGenericProperty("../Axis 2 /x", QVariant::Double, 0);
      axis2_y = pDialog->addGenericProperty("../Axis 2 /y", QVariant::Double, 0);
      axis2_z = pDialog->addGenericProperty("../Axis 2 /z", QVariant::Double, 0);
      angle1 = pDialog->addGenericProperty("../Angle 1", QVariant::Double, 0);
      angle2 = pDialog->addGenericProperty("../Angle 2", QVariant::Double, 0);
      lf1_x = pDialog->addGenericProperty("../Linear Force 1/x", QVariant::Double, 0.0);
      lf1_y = pDialog->addGenericProperty("../Linear Force 1/y", QVariant::Double, 0.0);
      lf1_z = pDialog->addGenericProperty("../Linear Force 1/z", QVariant::Double, 0.0);
      lf2_x = pDialog->addGenericProperty("../Linear Force 2/x", QVariant::Double, 0.0);
      lf2_y = pDialog->addGenericProperty("../Linear Force 2/y", QVariant::Double, 0.0);
      lf2_z = pDialog->addGenericProperty("../Linear Force 2/z", QVariant::Double, 0.0);
      af1_x = pDialog->addGenericProperty("../Angular Force 1/x", QVariant::Double, 0.0);
      af1_y = pDialog->addGenericProperty("../Angular Force 1/y", QVariant::Double, 0.0);
      af1_z = pDialog->addGenericProperty("../Angular Force 1/z", QVariant::Double, 0.0);
      af2_x = pDialog->addGenericProperty("../Angular Force 2/x", QVariant::Double, 0.0);
      af2_y = pDialog->addGenericProperty("../Angular Force 2/y", QVariant::Double, 0.0);
      af2_z = pDialog->addGenericProperty("../Angular Force 2/z", QVariant::Double, 0.0);
      /*
        anchor_x->setEnabled(false); anchor_y->setEnabled(false); anchor_z->setEnabled(false);
        axis1_x->setEnabled(false); axis1_y->setEnabled(false); axis1_z->setEnabled(false);
        axis2_x->setEnabled(false); axis2_y->setEnabled(false); axis2_z->setEnabled(false);
        angle1->setEnabled(false); angle2->setEnabled(false);
        lf1_x->setEnabled(false); lf1_y->setEnabled(false); lf1_z->setEnabled(false);
        lf2_x->setEnabled(false); lf2_y->setEnabled(false); lf2_z->setEnabled(false);
        af1_x->setEnabled(false); af1_y->setEnabled(false); af1_z->setEnabled(false);
        af2_x->setEnabled(false); af2_y->setEnabled(false); af2_z->setEnabled(false);
      */
      setupDataBrokerMapping();
    }


    Widget_Joint_State::~Widget_Joint_State(){
      control->dataBroker->unregisterAsyncReceiver(this, "*", "*");
    }


    void Widget_Joint_State::connect(unsigned long index){
      std::string groupName, dataName;
      if (actualJoint != 0){
        control->joints->getDataBrokerNames(actualJoint, &groupName, &dataName);
        control->dataBroker->unregisterAsyncReceiver(this, groupName, dataName);
        //    control->log->unregisterReceiver((LogClient*)this, LOG_TYPE_JOINT,
        //                                     actualJoint);
      }
      control->joints->getDataBrokerNames(index, &groupName, &dataName);
      control->dataBroker->registerAsyncReceiver(this, groupName, dataName);
      //  control->log->registerReceiver((LogClient*)this, LOG_TYPE_JOINT, index, 100);
      actualJoint = index;
      startTimer(150);
    }

  
    void Widget_Joint_State::receiveData(const data_broker::DataInfo &info,
                                         const data_broker::DataPackage &package,
                                         int callbackParam) {
      CPP_UNUSED(info);
      CPP_UNUSED(callbackParam);

      dbMapper.readPackage(package);
    }

    void Widget_Joint_State::setupDataBrokerMapping() {
      dbMapper.add("axis1/x", &data.axis.x());
      dbMapper.add("axis1/y", &data.axis.y());
      dbMapper.add("axis1/z", &data.axis.z());
      dbMapper.add("axis1/angle", &data.angle_axis1);
      dbMapper.add("axis1/speed", &data.velocity_axis1);
      dbMapper.add("axis1/torque/x", &data.axis1_torque.x());
      dbMapper.add("axis1/torque/y", &data.axis1_torque.y());
      dbMapper.add("axis1/torque/z", &data.axis1_torque.z());

      dbMapper.add("axis2/x", &data.axis2.x());
      dbMapper.add("axis2/y", &data.axis2.y());
      dbMapper.add("axis2/z", &data.axis2.z());
      dbMapper.add("axis2/angle", &data.angle_axis2);
      dbMapper.add("axis2/speed", &data.velocity_axis2);
      dbMapper.add("axis2/torque/x", &data.axis2_torque.x());
      dbMapper.add("axis2/torque/y", &data.axis2_torque.y());
      dbMapper.add("axis2/torque/z", &data.axis2_torque.z());

      dbMapper.add("force1/x", &data.f1.x());
      dbMapper.add("force1/y", &data.f1.y());
      dbMapper.add("force1/z", &data.f1.z());
      dbMapper.add("torque1/x", &data.t1.x());
      dbMapper.add("torque1/y", &data.t1.y());
      dbMapper.add("torque1/z", &data.t1.z());

      dbMapper.add("force2/x", &data.f2.x());
      dbMapper.add("force2/y", &data.f2.y());
      dbMapper.add("force2/z", &data.f2.z());
      dbMapper.add("torque2/x", &data.t2.x());
      dbMapper.add("torque2/y", &data.t2.y());
      dbMapper.add("torque2/z", &data.t2.z());

      dbMapper.add("anchor/x", &data.anchor.x());
      dbMapper.add("anchor/y", &data.anchor.y());
      dbMapper.add("anchor/z", &data.anchor.z());
      dbMapper.add("jointLoad/x", &data.joint_load.x());
      dbMapper.add("jointLoad/y", &data.joint_load.y());
      dbMapper.add("jointLoad/z", &data.joint_load.z());

      dbMapper.add("motorTorque", &data.motor_torque);
    }

    void Widget_Joint_State::timerEvent(QTimerEvent* event) {
      (void)event;
      anchor_x->setValue(data.anchor.x());
      anchor_y->setValue(data.anchor.y());
      anchor_z->setValue(data.anchor.z());

      axis1_x->setValue(data.axis.x());
      axis1_y->setValue(data.axis.y());
      axis1_z->setValue(data.axis.z());

      axis2_x->setValue(data.axis2.x());
      axis2_y->setValue(data.axis2.y());
      axis2_z->setValue(data.axis2.z());
  
      angle1->setValue(data.angle_axis1);
      angle2->setValue(data.angle_axis2);

      lf1_x->setValue(data.f1.x());
      lf1_y->setValue(data.f1.y());
      lf1_z->setValue(data.f1.z());
  
      lf2_x->setValue(data.f2.x());
      lf2_y->setValue(data.f2.y());
      lf2_z->setValue(data.f2.z());
  
      af1_x->setValue(data.t1.x());
      af1_y->setValue(data.t1.y());
      af1_z->setValue(data.t1.z());
  
      af2_x->setValue(data.t2.x());
      af2_y->setValue(data.t2.y());
      af2_z->setValue(data.t2.z());
    }



    void Widget_Joint_State::accept() {
      pDialog->close();
    }

  } // end of namespace gui
} // end of namespace mars
