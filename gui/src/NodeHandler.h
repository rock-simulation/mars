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
 * \file NodeHandler.h
 * \brief GUI handling creating and editing nodes in the simulation
 * \detail A property in the node tree dialog is created for each node
 * and this class handles its subproperties.
 */

#ifndef NODE_HANDLER_H
#define NODE_HANDLER_H

#ifdef _PRINT_HEADER_
#warning "NodeHandler.h"
#endif

#ifndef Q_MOC_RUN
#include <mars/main_gui/PropertyDialog.h>
#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/interfaces/terrainStruct.h>
#include <mars/interfaces/core_objects_exchange.h>
#include <mars/interfaces/NodeData.h>
#endif

#include "Dialog_Create_Material.h"
#include "Dialog_Edit_Material.h"
#include "Widget_Node_Options_ODE.h"

namespace mars {
  namespace gui {

    namespace NodeTree {
      /** The two modes in which the class is used:
       * PreviewMode for new nodes when creating a node
       * EditMode for editing already created nodes
       */
      enum Mode {
        PreviewMode,
        EditMode
      };
      
    } // end namespace NodeTree

    class NodeHandler : public QObject {
      Q_OBJECT
      public:

      NodeHandler(QtVariantProperty* property, unsigned long ind,
                  main_gui::PropertyDialog *pd, interfaces::ControlCenter *c, 
                  NodeTree::Mode m);
      ~NodeHandler();

      /**
       * \brief Called on creation of node 
       */
      int accept();
  
      void valueChanged(QtProperty *property, const QVariant &value);
  
      /** 
       * \brief Creates the subproperties for the node when focused - having lots of
       * properties in the dialog results in poor performance for big scenes
       */
      void focusIn();
  
      /**
       * \brief Destroys the subproperties for the node when not in focus
       */
      void focusOut();

      /**
       * \brief Creates a node preview for the current node if in PreviewMode
       * Called when focused.
       */ 
      void previewOn();

      /** 
       * \brief Removes the preview of the node when focused out
       */
      void previewOff();
  
      /**
       * \brief Opens a dialog displaying information about the node
       */
      void showState();
 
      /**
       * \brief The current mode of the node - already created in the simulation or not
       */ 
      NodeTree::Mode mode;
  
      /**
       * \brief Indicates if the node is selected in the graphics window
       */ 
      bool isSelected(void);
  
      /**
       * \brief Selects the node in the graphics window
       */
      void setSelected(bool s);
  
      // those return the properties needed by other dialogs 
      QtVariantProperty *getGeometryProp(void);
      QtVariantProperty *getPositionProp(void);
      QtVariantProperty *getRotationProp(void);

    private slots:
      // closes the state dialog
      void closeState();

    private:  
      // indicates if the node has ever been focused in
      bool initialized;
      // indicates if the node is selected in the graphcis window
      bool selected;
      // indicates if the subproperties are created; usually when focused in 
      bool filled;
      // indicates if the state dialog is visible
      bool state_on;
      // holds the type of the node
      int primitive_type;

      interfaces::ControlCenter *control;
      std::vector<interfaces::core_objects_exchange> allNodes;

      // the structures representing the node
      interfaces::NodeData node;
      interfaces::NodeData myRelNode;
      interfaces::terrainStruct terrain;
      interfaces::MaterialData material;
  
      // separate classes handling a few properties
      Dialog_Create_Material *dcm;
      Dialog_Edit_Material *dem;
      Widget_Node_Options *wno;

  
      std::string nodeName;
      std::string actualName;
      int myNodeIndex;
  
      main_gui::PropertyDialog *pDialog;
  
      // the color of the properties is different for the two modes
      QColor previewColor;
      QColor editColor;

      // the subproperties of a node
      QtVariantProperty *topLevelNode, *node_type;
      QtVariantProperty *general, *node_name, *group_id;
      QtVariantProperty *physics, *physics_model, *density, *mass, *movable;
      QtVariantProperty *geometry, *position, *rotation, *size, *move_all;
      QtVariantProperty *relative_pos, *offset, *pos_x, *pos_y, *pos_z;
      QtVariantProperty *rot_alpha, *rot_beta, *rot_gamma;
      QtVariantProperty *size_x, *size_y, *size_z, *size_d, *size_r, *size_h;
      QtVariantProperty *image, *height, *width, *length;
      QtVariantProperty *create_material, *edit_material, *ode;
      QList<QtVariantProperty*> physics_props;
      QList<QtVariantProperty*> top_props;

      // creates the subproperties
      void fill();
      // update the structures and simulation in EditMode 
      void on_type_changed();
      void relative_changed();
      void updateContact();
      void updateMaterial();
      void updateNode();
      void updatePos();
      void updateRot();
      void updateSize();

      // updates the structures and the preview of the node according to the GUI in PreviewMode only
      void update(); 

      unsigned int getIndex(unsigned long index);
    };

  } // end of namespace gui
} // end of namespace mars

#endif // NODE_HANDLER_H
