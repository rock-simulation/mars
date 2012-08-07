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

#include "Widget_Node_Options_ODE.h"

namespace mars {
  namespace gui {

    /**
     * Creates the widget
     */
    Widget_Node_Options::Widget_Node_Options(main_gui::PropertyDialog *par)
    {
      parent = par;
      actualNode = 0;
      my_cp.setZero();
    }

    QtVariantProperty* Widget_Node_Options::getTopLevelProperty() {
      return top;
    }


    void Widget_Node_Options::init(interfaces::contact_params cp, std::string name){

      std::map<QString, QVariant> attr;
      attr.insert(std::pair<QString, QVariant>(QString("singleStep"), 0.1));
      attr.insert(std::pair<QString, QVariant>(QString("decimals"), 16));

      top = 
        parent->addGenericProperty("../"+name+"ODE Options",
                                   QtVariantPropertyManager::groupTypeId(), 0);
      erp = 
        parent->addGenericProperty("../"+name+"ODE Options/ERP", 
                                   QVariant::Double, cp.erp, &attr);
      cfm = 
        parent->addGenericProperty("../"+name+"ODE Options/CFM", 
                                   QVariant::Double, cp.cfm, &attr);
      friction1 = 
        parent->addGenericProperty("../"+name+"ODE Options/Friction 1", 
                                   QVariant::Double, cp.friction1, &attr);
      friction2 = 
        parent->addGenericProperty("../"+name+"ODE Options/Friction 2", 
                                   QVariant::Double, cp.friction2, &attr);
      fds1 = 
        parent->addGenericProperty("../"+name+"ODE Options/Friction Dir Slip 1",
                                   QVariant::Double, cp.fds1, &attr);
      fds2 = 
        parent->addGenericProperty("../"+name+"ODE Options/Friction Dir Slip 2",
                                   QVariant::Double, cp.fds2, &attr);
      motion1 = 
        parent->addGenericProperty("../"+name+"ODE Options/Motion 1",
                                   QVariant::Double, cp.motion1, &attr);
      motion2 = 
        parent->addGenericProperty("../"+name+"ODE Options/Motion 2", 
                                   QVariant::Double, cp.motion2, &attr);
      bounce = 
        parent->addGenericProperty("../"+name+"ODE Options/Bounce",
                                   QVariant::Double, cp.bounce, &attr);
      bounce_vel = 
        parent->addGenericProperty("../"+name+"ODE Options/Bounce Velocity",
                                   QVariant::Double, cp.bounce_vel, &attr);
      approx_pyramid = 
        parent->addGenericProperty("../"+name+"ODE Options/Approx. Pyramid",
                                   QVariant::Double, cp.approx_pyramid, &attr);
      coll_bitmask = 
        parent->addGenericProperty("../"+name+"ODE Options/Collide Bitmask",
                                   QVariant::Int, cp.coll_bitmask);
      my_cp = cp;
    }

    interfaces::contact_params Widget_Node_Options::get(){
      return my_cp;
    }

    void Widget_Node_Options::accept(){
      my_cp.erp = erp->value().toDouble();
      my_cp.cfm = cfm->value().toDouble();
      my_cp.friction1 = friction1->value().toDouble();
      my_cp.friction2 = friction2->value().toDouble();
      my_cp.motion1 = motion1->value().toDouble();
      my_cp.motion2 = motion2->value().toDouble();
      my_cp.bounce = bounce->value().toDouble();
      my_cp.bounce_vel = bounce_vel->value().toDouble();
      my_cp.coll_bitmask = coll_bitmask->value().toInt();
      my_cp.approx_pyramid = approx_pyramid->value().toDouble();
      my_cp.fds1 = fds1->value().toDouble();
      my_cp.fds2 = fds2->value().toDouble();
  
      emit changed();
    }



    bool Widget_Node_Options::owns(QtProperty *property) {
      if (property == top || property == erp || property == cfm ||
          property == friction1 || property == friction2 || property == motion1 ||
          property == motion2 || property == bounce || property == bounce_vel ||
          property == coll_bitmask || property == approx_pyramid || 
          property == fds1 || property == fds2)
        return true;
      else
        return false;
    }


  } // end of namespace gui
} // end of namespace mars
