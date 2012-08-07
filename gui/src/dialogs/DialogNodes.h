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

/**
 * \file DialogNodes.h
 * \brief A node tree widget holding all nodes as properties. Main node handling 
 * functionality is provided - adding, ediitng and removing nodes
 */


#ifndef DIALOGNODES_H
#define DIALOGNODES_H

#ifdef _PRINT_HEADER_
#warning "DialogNodes.h"
#endif

#include <mars/main_gui/BaseWidget.h>
#include <mars/main_gui/PropertyDialog.h>
#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/interfaces/graphics/GraphicsEventClient.h>
#include "NodeHandler.h"

namespace mars {

  namespace main_gui {
    class GuiInterface;
  }

  namespace gui {

    class DialogNodes : public main_gui::BaseWidget,
                        public main_gui::PropertyCallback,
                        public interfaces::GraphicsEventClient {

      Q_OBJECT
    
      public:
      DialogNodes(interfaces::ControlCenter *c, main_gui::GuiInterface *gui);
      ~DialogNodes();
  
      main_gui::PropertyDialog *pDialog;  
  
      /**
       * handles selection of a node in the graphics window
       */
      virtual void selectEvent(unsigned long int id, bool mode);
    
    private:

      // handles new focus
      virtual void topLevelItemChanged(QtProperty* current);
      virtual void valueChanged(QtProperty *property, const QVariant &value);

      QtProperty *oldFocus;
      bool filled;
      QPushButton *stateButton;
      QPushButton *addButton;
      QPushButton *removeButton;
      std::vector<NodeHandler*> allDialogs; // all handler classes in edit mode
      std::vector<NodeHandler*> newDialogs; // handler classes in previewmode
      std::vector<QtProperty*> allNodes_p; // all top level properties in edit mode
      std::vector<QtProperty*> newNodes_p; // top level properties for nodes in preview mode
      std::vector<interfaces::core_objects_exchange> allNodes; // all simulation nodes
      interfaces::ControlCenter* control;
      main_gui::GuiInterface *mainGui;

      void closeEvent(QCloseEvent* event);

    signals:
      void closeSignal(void* widget);

    private slots:
      void on_new_node(); // creates a new node in preview mode
      void on_remove_node(); // removes the focused node
      void on_add_node(); // adds a node in the simulation, changing its mode to edit mode
      void on_node_state(); // shows the state dialog for the focused node
    };
  
  } // end of namespace gui
} // end of namespace mars

#endif // DIALOGNODES_H
