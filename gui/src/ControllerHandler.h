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

#ifndef MARS_GUI_CONTROLLER_HANDLER_H
#define MARS_GUI_CONTROLLER_HANDLER_H

#ifdef _PRINT_HEADER_
#warning "ControllerHandler.h"
#endif

#include <mars/main_gui/PropertyDialog.h>
#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/interfaces/core_objects_exchange.h>
#include <mars/interfaces/ControllerData.h>

#include "SelectionDialog.h"

namespace mars {
  namespace gui {

    namespace ControllerTree {

      enum Mode {
        PreviewMode,
        EditMode
      };

    } // end of namespace ControllerTree

    /**
     * \brief handles one controller in the simulation providing gui options.
     * Editing controlllers is not supported. 
     */
    class ControllerHandler : public QObject {
      Q_OBJECT;
    public:

      ControllerHandler(QtVariantProperty* property, unsigned long ind,
                        main_gui::PropertyDialog *pd,
                        interfaces::ControlCenter *c, 
                        ControllerTree::Mode m);
      ~ControllerHandler();

      void valueChanged(QtProperty *property, const QVariant &value) {}
      void accept();

      ControllerTree::Mode mode;

      void nodes_selection();
      void motors_selection();
      void sensors_selection();

    public slots:
      void choose_nodes(QString list);
      void choose_sensors(QString list);
      void choose_motors(QString list);
  
 
    private:  
      std::vector<interfaces::core_objects_exchange> mySensors;
      std::vector<interfaces::core_objects_exchange> myMotors;
      std::vector<interfaces::core_objects_exchange> myNodes;
      std::vector<interfaces::core_objects_exchange> chosenSensors;
      std::vector<interfaces::core_objects_exchange> chosenMotors;
      std::vector<interfaces::core_objects_exchange> chosenNodes;
      interfaces::ControlCenter* control;
  
      interfaces::ControllerData myController;
      bool filled;
      QtVariantProperty *filename, *sensors, *motors, *nodes, *rate;
      SelectionDialog* motorDialog, *sensorDialog, *nodeDialog;
 
      QtVariantProperty *topLevelController;

      std::vector<interfaces::core_objects_exchange> allControllers;
      std::string controllerName;
      std::string actualName;
      int myControllerIndex;
      main_gui::PropertyDialog *pDialog;
      QColor previewColor;
      QColor editColor;
  
      void fill();
  
    }; // end of class ControllerHandler

  } // end of namespace gui
} // end of namespace mars



#endif // MARS_GUI_CONTROLLER_HANDLER_H
