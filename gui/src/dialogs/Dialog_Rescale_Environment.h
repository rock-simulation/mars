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
 
#ifndef DIALOG_RESCALE_ENVIRONMENT_H
#define DIALOG_RESCALE_ENVIRONMENT_H

#ifdef _PRINT_HEADER_
  #warning "Dialog_Rescale_Environment.h"
#endif

#include <mars/main_gui/BaseWidget.h>
#include <mars/main_gui/PropertyDialog.h>
#include <mars/interfaces/sim/ControlCenter.h>

namespace mars {

  namespace main_gui {
    class GuiInterface;
  }

  namespace gui {

    /**
     *Dialog for rescaling the environment
     * every node in the environment is rescaled (including its position)
     */

    class Dialog_Rescale_Environment: public main_gui::BaseWidget,
                                      public main_gui::PropertyCallback {
      Q_OBJECT
      public:
      /**\brief Creates the Dialog */
      Dialog_Rescale_Environment(interfaces::ControlCenter *c, main_gui::GuiInterface *gui);
      ~Dialog_Rescale_Environment();

      main_gui::PropertyDialog* pDialog;
      void show(void) {pDialog->show();}
      void hide(void) {pDialog->hide();}
      bool isHidden(void) {return pDialog->isHidden();}
      void close(void) {pDialog->close();}

    private slots:
  
      virtual void accept();
      virtual void reject();
      virtual void valueChanged(QtProperty* property, const QVariant& value);
      void closeDialog();
  
    private:
      interfaces::ControlCenter *control;
      main_gui::GuiInterface *mainGui;
      QtVariantProperty *x_factor, *y_factor, *z_factor;

    };

  } // end of namespace gui
} // end of namespace mars

#endif
