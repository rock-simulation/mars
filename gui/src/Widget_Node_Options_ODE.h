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


#ifndef WIDGET_NODE_OPTIONS_H
#define WIDGET_NODE_OPTIONS_H

#ifdef _PRINT_HEADER_
  #warning "Widget_Node_Options_ODE.h"
#endif



#include <mars/main_gui/PropertyDialog.h>
#include <mars/interfaces/contact_params.h>
#include <mars/interfaces/sim_common.h>
#include <mars/utils/Vector.h>

namespace mars {
  namespace gui {

    /**
     * \brief A widget to show the actual properties of a node to the user
     */
    class Widget_Node_Options : public QObject {
      Q_OBJECT
  
      public:
      /**\brief The constructor creates the Dialog */
      Widget_Node_Options(main_gui::PropertyDialog *p);
  
      /**\brief inits widget with the given contact_params */
      void init(interfaces::contact_params cp, std::string name = "");
  
      /**\brief returns the contact_params struct */
      interfaces::contact_params get();

      bool owns(QtProperty *property);

      QtVariantProperty* getTopLevelProperty();

    signals:
      void changed();

    private slots:
      void accept();
  
    private:
      main_gui::PropertyDialog* parent;
      QtVariantProperty *top;
      QtVariantProperty *erp, *cfm, *friction1, *friction2, *motion1, *motion2;
      QtVariantProperty *bounce, *bounce_vel, *coll_bitmask, *approx_pyramid;
      QtVariantProperty *fds1, *fds2;
  
      utils::Vector fd;
      interfaces::contact_params my_cp;
      unsigned long actualNode;
    };


  } // end of namespace gui
} // end of namespace mars

#endif
