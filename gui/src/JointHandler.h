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


#ifndef EDITJOINT_H
#define EDITJOINT_H

#ifdef _PRINT_HEADER_
#warning "JointHandler.h"
#endif

#include <mars/main_gui/PropertyDialog.h>
#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/interfaces/core_objects_exchange.h>
#include <mars/interfaces/JointData.h>

#include <QLabel>

namespace mars {
  namespace gui {

    namespace JointTree {

      enum Mode {
        PreviewMode,
        EditMode
      };

    } // end of namespace JointTree

    class JointHandler : public QObject {
      Q_OBJECT
      public:

      JointHandler(QtVariantProperty* property, unsigned long ind,
                   main_gui::PropertyDialog *pd, interfaces::ControlCenter *c, 
                   JointTree::Mode m, std::string imagePath_);

      ~JointHandler();

      int accept();
      void valueChanged(QtProperty *property, const QVariant &value);
      void focusIn();
      void focusOut();
      void previewOn();
      void previewOff();
      void showState();
      JointTree::Mode mode;
 

      void on_reset_anchor(void);
      QLabel *previewLabel;

    private slots:
      void closeState();

    private:  
      bool state_on;
      interfaces::ControlCenter *control;
      std::vector<interfaces::core_objects_exchange> allJoints;
      std::vector<interfaces::core_objects_exchange> allNodes;
      std::string imagePath;
      interfaces::JointData myJoint;
      bool userEdited, filled;
      bool double_axis;
      int posType;
      QtVariantProperty *general, *type, *first, *second;
      QtVariantProperty *anchor, *a_pos, *center_x, *center_y, *center_z;
      QtVariantProperty *axis1, *axis2, *alpha1, *alpha2, *beta1, *beta2, *gamma1, *gamma2;
      QtVariantProperty *constraints, *enable_con, *low_stop, *high_stop, *con_damp, *con_spring;
      QtVariantProperty *low1, *low2, *high1, *high2, *damp1, *damp2, *spring1, *spring2;

      std::string jointName;
      std::string actualName;
      int myJointIndex;
      main_gui::PropertyDialog *pDialog;
      QColor previewColor;
      QColor editColor;

      QtVariantProperty *topLevelJoint;
      QList<QtVariantProperty*> top_props;

      void fill();
      void on_change_type(int index);
      void on_check_constraints(bool checked);
      void drawAxis();
      void on_set_anchor(void);
      void update(void);
    };


  } // end of namespace gui
} // end of namespace mars

#endif // EDITJOINT_H
