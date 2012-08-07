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

#ifndef WIDGET_NODE_STATE_H
#define WIDGET_NODE_STATE_H

#ifdef _PRINT_HEADER_
#warning "Widget_Node_State.h"
#endif

#include <mars/main_gui/PropertyDialog.h>
#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/utils/Vector.h>
#include <mars/utils/Quaternion.h>
#include <mars/interfaces/MARSDefs.h>
#include <mars/data_broker/ReceiverInterface.h>
#include <mars/data_broker/DataPackageMapping.h>

namespace mars {
  namespace gui {

    struct node_data{
      utils::Vector pos;
      utils::Quaternion rot;
      utils::Vector lvel;
      utils::Vector avel;
      utils::Vector f;
      utils::Vector t;
      bool ground_contact;
      interfaces::sReal ground_contact_force;
    };

    /**
     * \brief A widget to show the actual properties of a node to the user
     */
    class Widget_Node_State : public QObject, public data_broker::ReceiverInterface,
                              public main_gui::PropertyCallback {
      Q_OBJECT
  
      public:
      /**\brief The constructor creates the Dialog */
      Widget_Node_State(QWidget *par, 
                        interfaces::ControlCenter* c,
                        std::string name = "Node State");
  
      ~Widget_Node_State();
  
      /**\brief Connects with a LogReceiver */
      void connect(unsigned long index);

      main_gui::PropertyDialog *pDialog;
  
    private slots:
      virtual void accept();
	  
      /**\brief receives the node information from a DataBroker */
      virtual void receiveData(const data_broker::DataInfo &info,
                               const data_broker::DataPackage &package,
                               int callbackParam);
  
    private:
      unsigned long actualNode;
      interfaces::ControlCenter *control;
      node_data data;
      bool connected;
      QtVariantProperty *pos_x, *pos_y, *pos_z, *rot_x, *rot_y, *rot_z;
      QtVariantProperty *lv_x, *lv_y, *lv_z, *av_x, *av_y, *av_z;
      QtVariantProperty *lf_x, *lf_y, *lf_z, *af_x, *af_y, *af_z;
      // for dataBroker communication
      data_broker::DataPackageMapping dbMapper;

      void setupDataBrokerMapping();

    protected:
      void timerEvent(QTimerEvent *event);
    };


  } // end of namespace gui
} // end of namespace mars

#endif
