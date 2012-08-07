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

#include "Widget_Node_State.h"
#include <mars/utils/mathUtils.h>
#include <mars/data_broker/DataBrokerInterface.h>

#include <mars/interfaces/sim/NodeManagerInterface.h>

using namespace std;

namespace mars {
  namespace gui {

    /**
     * Creates the widget
     */
    Widget_Node_State::Widget_Node_State(QWidget *parent, 
                                         interfaces::ControlCenter* c,
                                         std::string name)
      : pDialog(new main_gui::PropertyDialog(parent)), control(c) {

      pDialog->setAttribute(Qt::WA_DeleteOnClose);
      pDialog->setWindowTitle(QString::fromStdString(name));
      pDialog->setPropCallback(dynamic_cast<main_gui::PropertyCallback*>(this));
      pDialog->clearButtonBox();
      pDialog->addGenericButton("OK", this, SLOT(accept()));
      pDialog->show();

      actualNode = 0;

      map<QString, QVariant> attr;
      attr.insert(pair<QString, QVariant>(QString("decimals"), 9));

      pDialog->setWindowTitle(QString::fromStdString(name));

      pos_x = 
        pDialog->addGenericProperty("../Position/x", 
                                    QVariant::Double, 0.0, &attr);
      pos_y = 
        pDialog->addGenericProperty("../Position/y", 
                                    QVariant::Double, 0.0, &attr);
      pos_z = 
        pDialog->addGenericProperty("../Position/z", 
                                    QVariant::Double, 0.0, &attr);
      rot_x = 
        pDialog->addGenericProperty("../Rotation/alpha",
                                    QVariant::Double, 0.0, &attr);
      rot_y  = 
        pDialog->addGenericProperty("../Rotation/beta",
                                    QVariant::Double, 0.0, &attr);
      rot_z = 
        pDialog->addGenericProperty("../Rotation/gamma", 
                                    QVariant::Double, 0.0, &attr);
      lv_x = 
        pDialog->addGenericProperty("../Linear Velocity/x",
                                    QVariant::Double, 0.0, &attr);
      lv_y = 
        pDialog->addGenericProperty("../Linear Velocity/y",
                                    QVariant::Double, 0.0, &attr);
      lv_z = 
        pDialog->addGenericProperty("../Linear Velocity/z",
                                    QVariant::Double, 0.0, &attr);
      av_x = 
        pDialog->addGenericProperty("../Angular Velocity/x",
                                    QVariant::Double, 0.0, &attr);
      av_y = 
        pDialog->addGenericProperty("../Angular Velocity/y",
                                    QVariant::Double, 0.0, &attr);
      av_z = 
        pDialog->addGenericProperty("../Angular Velocity/z",
                                    QVariant::Double, 0.0, &attr);
      lf_x = 
        pDialog->addGenericProperty("../Linear Force/x",
                                    QVariant::Double, 0.0, &attr);
      lf_y = 
        pDialog->addGenericProperty("../Linear Force/y", 
                                    QVariant::Double, 0.0, &attr);
      lf_z = 
        pDialog->addGenericProperty("../Linear Force/z", 
                                    QVariant::Double, 0.0, &attr);
      af_x = 
        pDialog->addGenericProperty("../Angular Force/x", 
                                    QVariant::Double, 0.0, &attr);
      af_y = 
        pDialog->addGenericProperty("../Angular Force/y", 
                                    QVariant::Double, 0.0, &attr);
      af_z = 
        pDialog->addGenericProperty("../Angular Force/z", 
                                    QVariant::Double, 0.0, &attr);
      /*
        pos_x->setEnabled(false); 
        pos_y->setEnabled(false); 
        pos_z->setEnabled(false);
        rot_x->setEnabled(false); 
        rot_y->setEnabled(false); 
        rot_z->setEnabled(false);
        lv_x->setEnabled(false); 
        lv_y->setEnabled(false); 
        lv_z->setEnabled(false);
        av_x->setEnabled(false); 
        av_y->setEnabled(false); 
        av_z->setEnabled(false);
        lf_x->setEnabled(false); 
        lf_y->setEnabled(false); 
        lf_z->setEnabled(false);
        af_x->setEnabled(false); 
        af_y->setEnabled(false); 
        af_z->setEnabled(false);
      */
  
      setupDataBrokerMapping();
      connected = false;
      startTimer(150);
  
    }

    /**
     * Deletes the widget and calls unregisterLogReceiver
     */
    Widget_Node_State::~Widget_Node_State(){
      control->dataBroker->unregisterAsyncReceiver(this, "*", "*");
    }


    /**
     * Connects with a LogReceiver. Unregisters previous one
     */
    void Widget_Node_State::connect(unsigned long index){
      std::string groupName, dataName;
      if (actualNode != 0){
        control->nodes->getDataBrokerNames(actualNode, &groupName, &dataName);
        control->dataBroker->unregisterAsyncReceiver(this, groupName, dataName);
      }
      control->nodes->getDataBrokerNames(index, &groupName, &dataName);
      control->dataBroker->registerAsyncReceiver(this, groupName, dataName);
      actualNode = index;
      connected = true;
    }


    void Widget_Node_State::receiveData(const data_broker::DataInfo &info,
                                        const data_broker::DataPackage &package,
                                        int callbackParam) {
      CPP_UNUSED(info);
      CPP_UNUSED(callbackParam);
      dbMapper.readPackage(package);
    }

    void Widget_Node_State::setupDataBrokerMapping() {
      dbMapper.add("position/x", &data.pos.x());
      dbMapper.add("position/y", &data.pos.y());
      dbMapper.add("position/z", &data.pos.z());
      dbMapper.add("rotation/x", &data.rot.x());
      dbMapper.add("rotation/y", &data.rot.y());
      dbMapper.add("rotation/z", &data.rot.z());
      dbMapper.add("rotation/w", &data.rot.w());
      dbMapper.add("linearVelocity/x", &data.lvel.x());
      dbMapper.add("linearVelocity/y", &data.lvel.y());
      dbMapper.add("linearVelocity/z", &data.lvel.z());
      dbMapper.add("angularVelocity/x", &data.avel.x());
      dbMapper.add("angularVelocity/y", &data.avel.y());
      dbMapper.add("angularVelocity/z", &data.avel.z());
      dbMapper.add("force/x", &data.f.x());
      dbMapper.add("force/y", &data.f.y());
      dbMapper.add("force/z", &data.f.z());
      dbMapper.add("torque/x", &data.t.x());
      dbMapper.add("torque/y", &data.t.y());
      dbMapper.add("torque/z", &data.t.z());
      dbMapper.add("groundContact", &data.ground_contact);
      dbMapper.add("groundContactForce", &data.ground_contact_force);
    }

    void Widget_Node_State::timerEvent(QTimerEvent *event) {
      (void)event;
      if (connected == false) return;

      /*
        if (actualNode != 0)
        control->log->unregisterReceiver((LogClient*)this, LOG_TYPE_NODE,
        actualNode);
  
        control->log->registerReceiver((LogClient*)this, LOG_TYPE_NODE,
        actualNode, 150);
      */
      pos_x->setValue(data.pos.x());
      pos_y->setValue(data.pos.y());
      pos_z->setValue(data.pos.z());

      //sRotation rot = data.rot.toEuler();
      utils::sRotation rot = utils::quaternionTosRotation(data.rot);
      rot_x->setValue(rot.alpha);
      rot_y->setValue(rot.beta);
      rot_z->setValue(rot.gamma);

      lv_x->setValue(data.lvel.x());
      lv_y->setValue(data.lvel.y());
      lv_z->setValue(data.lvel.z());
  
      av_x->setValue(data.avel.x());
      av_y->setValue(data.avel.y());
      av_z->setValue(data.avel.z());
   
      lf_x->setValue(data.f.x());
      lf_y->setValue(data.f.y());
      lf_z->setValue(data.f.z());

      af_x->setValue(data.t.x());
      af_y->setValue(data.t.y());
      af_z->setValue(data.t.z());
    }


    void Widget_Node_State::accept() {
      pDialog->close();
    }

  } // end of namespace gui
} // end of namespace mars
