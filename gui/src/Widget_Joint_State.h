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

#ifndef WIDGET_JOINT_STATE_H
#define WIDGET_JOINT_STATE_H

#ifdef _PRINT_HEADER_
#warning "Widget_Joint_State.h"
#endif

#include "mars/main_gui/PropertyDialog.h"
#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/utils/Vector.h>
#include <mars/interfaces/MARSDefs.h>
#include <mars/data_broker/ReceiverInterface.h>
#include <mars/data_broker/DataPackageMapping.h>

namespace mars {
  namespace gui {

    struct joint_data{
      interfaces::sReal angle_axis1;
      interfaces::sReal angle_axis2;
      interfaces::sReal velocity_axis1;
      interfaces::sReal velocity_axis2;
      utils::Vector anchor;
      utils::Vector axis;
      utils::Vector axis2;
      utils::Vector f1, f2;
      utils::Vector t1, t2;
      utils::Vector axis1_torque, axis2_torque;
      utils::Vector joint_load;
      interfaces::sReal motor_torque;
    };


    /**
     * \brief A widget to show the actual properties of a joint
     */
    class Widget_Joint_State : public QObject, public data_broker::ReceiverInterface,
                               public main_gui::PropertyCallback {
      Q_OBJECT
  
      public:
      /**\brief The constructor creates the Dialog */
      Widget_Joint_State(main_gui::PropertyDialog *par, 
                         interfaces::ControlCenter* c,
                         std::string name = "Joint State");
  
      ~Widget_Joint_State();
  
      void connect(unsigned long index);
  
      main_gui::PropertyDialog *pDialog;

  
    private slots:
      virtual void accept();
      virtual void receiveData(const data_broker::DataInfo &info,
                               const data_broker::DataPackage &package,
                               int callbackParam);
  
    private:
      unsigned long actualJoint;
      interfaces::ControlCenter* control;
      joint_data data;
      QtVariantProperty *anchor_x, *anchor_y, *anchor_z;
      QtVariantProperty *axis1_x, *axis1_y, *axis1_z;  
      QtVariantProperty *axis2_x, *axis2_y, *axis2_z;  
      QtVariantProperty *angle1, *angle2;
      QtVariantProperty *lf1_x, *lf1_y, *lf1_z, *af1_x, *af1_y, *af1_z;
      QtVariantProperty *lf2_x, *lf2_y, *lf2_z, *af2_x, *af2_y, *af2_z;

      void timerEvent(QTimerEvent *event);

      // for dataBroker communication
      data_broker::DataPackageMapping dbMapper;
      void setupDataBrokerMapping();

    };

  } // end of namespace gui
} // end of namespace mars

#endif
