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

#ifndef DIALOG_ADD_FORCE_H
#define DIALOG_ADD_FORCE_H

#ifdef _PRINT_HEADER_
#warning "Dialog_Add_Force.h"
#endif

#include <mars/main_gui/BaseWidget.h>
#include <mars/main_gui/PropertyDialog.h>
#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/interfaces/core_objects_exchange.h>

namespace mars {

  namespace main_gui {
    class GuiInterface;
  }
  
  namespace gui {


    /** \brief Adds a QDialog to set force to nodes */
    class Dialog_Add_Force : public main_gui::BaseWidget,
                             public main_gui::PropertyCallback {
      Q_OBJECT
  
      public:
      /**\brief Creates the Dialog */
      Dialog_Add_Force(interfaces::ControlCenter* c, main_gui::GuiInterface *gui);
      ~Dialog_Add_Force();

      /** \brief return the address of the NodeList vector  */
      std::vector<interfaces::core_objects_exchange>* getNodeListPtr(void);	

      main_gui::PropertyDialog* pDialog;
      void show(void) {pDialog->show();}
      void hide(void) {pDialog->hide();}
      bool isHidden(void) {return pDialog->isHidden();}
      void close(void) {pDialog->close();}

    private slots:
  
      /**\brief apply force to selected node */
      virtual void accept();
      virtual void reject();
      virtual void valueChanged(QtProperty*, const QVariant&) {}
      void closeDialog();

    private:
      std::vector<interfaces::core_objects_exchange> myNodes;
      interfaces::ControlCenter* control;
      main_gui::GuiInterface *mainGui;

      QtVariantProperty* vector_x;
      QtVariantProperty* vector_y;
      QtVariantProperty* vector_z;
      QtVariantProperty* magnitude;
      QtVariantProperty* node;

    };

  } // end of namespace gui
} // end of namespace mars

#endif
