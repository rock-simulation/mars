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
 * \file DialogDistance.h
 */


#ifndef DIALOG_DISTANCE_H
#define DIALOG_DISTANCE_H

#ifdef _PRINT_HEADER_
#warning "DialogDistance.h"
#endif

#include <mars/main_gui/BaseWidget.h>
#include <mars/main_gui/PropertyDialog.h>
#include <mars/interfaces/sim/ControlCenter.h>
#include "NodeSelectionTree.h"

#include <QListWidget>

namespace mars {
  namespace gui {

    /**
     * \brief Provides selection functionality displaying information about
     * distance between two objects. Changing the distance is also possible along
     * the distance vector originating from the first object's position.
     */
    class DialogDistance : public main_gui::BaseWidget,
                           public main_gui::PropertyCallback {
  
      Q_OBJECT
    
      public:
      /**\brief creates the dialog */
      DialogDistance(interfaces::ControlCenter *c);
      ~DialogDistance();

      main_gui::PropertyDialog *pDialog;  
  
    private:
      std::vector<interfaces::core_objects_exchange> simNodes;
      std::vector<interfaces::core_objects_exchange> simJoints;

      std::vector<unsigned long> selectedNodes;
      std::vector<unsigned long> selectedJoints;
      utils::Vector direction, origin;
      interfaces::core_objects_exchange first, second;
      double original_distance;

      QListWidget *objectList;
      interfaces::ControlCenter* control;
      bool filled;

      QtVariantProperty* viewNodes, *viewJoints, *selection, *distance, *ap1, *ap2, *rp1, *rp2;

      void closeEvent(QCloseEvent *event);
      void updateProperties(void);
      void changeDistance(double new_dist);

    signals:
      void closeSignal(void*);
    
    private slots:
      virtual void valueChanged(QtProperty *property, const QVariant &value);
      void selectObjects();
    };

  } // end of namespace gui
} // end of namespace mars

#endif
 
