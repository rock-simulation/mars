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
 * \file DialogJoints.h
 * \brief A joint tree widget holding all joints as properties. Main joint handling 
 * functionality is provided - adding, ediitng and removing joints
 */

#ifndef DIALOGJOINTS_H
#define DIALOGJOINTS_H

#ifdef _PRINT_HEADER_
#warning "DialogJoints.h"
#endif

#include <mars/main_gui/BaseWidget.h>
#include <mars/main_gui/PropertyDialog.h>
#include <mars/interfaces/sim/ControlCenter.h>
#include "JointHandler.h"

namespace mars {

  namespace main_gui {
    class GuiInterface;
  }

  namespace gui {

    class DialogJoints : public main_gui::BaseWidget,
                         public main_gui::PropertyCallback {
      Q_OBJECT
    
      public:
      DialogJoints(interfaces::ControlCenter *c, main_gui::GuiInterface *gui,
                   std::string imagePath_);

      ~DialogJoints();

      main_gui::PropertyDialog *pDialog;  
  
    private:
      std::string imagePath;

      virtual void topLevelItemChanged(QtProperty* current);  // handles new focus
      virtual void valueChanged(QtProperty *property, const QVariant &value);

      QtProperty *oldFocus;
      bool filled;
      QPushButton *stateButton;
      QPushButton *addButton;
      QPushButton *removeButton;
      QPushButton* previewButton;

      std::vector<JointHandler*> allDialogs; // all handler classes in edit mode
      std::vector<JointHandler*> newDialogs; // handler classes in preview mode
      std::vector<QtProperty*> allJoints_p; // top level properties in edit mode
      std::vector<QtProperty*> newJoints_p; // top level properties in preview mode
      std::vector<interfaces::core_objects_exchange> allJoints; // all simulation joints
  
      interfaces::ControlCenter* control;
      main_gui::GuiInterface *mainGui;

      void closeEvent(QCloseEvent* event);

    signals:
      void closeSignal(void* widget);

    private slots:
      void on_new_joint(); // creates a new joint with its preview
      void on_remove_joint(); // removes the focused joint
      void on_add_joint(); // adds the focused joint to the simulation, changing its mode to edit mode
      void on_joint_state(); // displays the state dialog for the focused joint
      void on_preview(); // displays a picture of the joint
    };
  
  } // end of namespace gui
} // end of namespace mars

#endif // DIALOGJOINTS_H
