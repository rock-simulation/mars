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

#ifndef DIALOG_IMPORT_MESH_H
#define DIALOG_IMPORT_MESH_H

#ifdef _PRINT_HEADER_
#warning "Dialog_Import_Mesh.h"
#endif

#ifndef Q_MOC_RUN
#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/interfaces/NodeData.h>
#endif

#include "Dialog_Create_Material.h"
#include "Widget_Node_Options_ODE.h"
#include <vector>

#include <mars/main_gui/BaseWidget.h>

namespace mars {

  namespace main_gui {
    class GuiInterface;
  }

  namespace gui {

    /**
     * \brief Creates the "Import Mesh" dialog
     */
    class Dialog_Import_Mesh : public main_gui::BaseWidget,
                               public main_gui::PropertyCallback {

      Q_OBJECT
  
      public:
      Dialog_Import_Mesh(interfaces::ControlCenter* c, main_gui::GuiInterface *gui);
      ~Dialog_Import_Mesh();

      main_gui::PropertyDialog *pDialog;  
	
      void show(void) {pDialog->show();}
      void hide(void) {pDialog->hide();}
      bool isHidden(void) {return pDialog->isHidden();}
      void close(void) {pDialog->close();}

    public slots:
      virtual void accept();
      virtual void reject();
      virtual void valueChanged(QtProperty *property, const QVariant &value);
      virtual void closeDialog();
  
      /** \brief removes selected item from the list */
      void on_remove();

      /** \brief scales all nodes to with factor */
      void on_scale();
	
    private:
      interfaces::ControlCenter* control;
      main_gui::GuiInterface *mainGui;

      Dialog_Create_Material* dcm;
      Widget_Node_Options* wno;
      interfaces::MaterialData material;
      std::vector<interfaces::NodeData> allNodes;
      bool user_input, filled, initialized;

      void fill();

      /**\brief if the physic model changed the size parameters have to be adjusted*/
      void model_changed();
  
      /**\brief the selected item changed so update the lineEdit boxes */
      void item_changed();
  
      /** \brief this function updates the NodeData structs if anything in
          the dialog has happend */
      void something_happened();

      QPushButton *remove_mesh, *scale_all;
      QtVariantProperty *meshes, *move_all, *scale_factor;
      QtVariantProperty* general, *physics, *geometry, *visual_geometry;
      QtVariantProperty* node_name, *group_id, *myFileName;
      QtVariantProperty* physics_model, *mass, *density, *unmovable;
      QtVariantProperty* position, *rotation, *size;
      QtVariantProperty* pos_x, *pos_y, *pos_z;
      QtVariantProperty* rot_alpha, *rot_beta, *rot_gamma;
      QtVariantProperty* size_x, *size_y, *size_z, *size_d, *size_r, *size_h;
      QtVariantProperty* visual_position, *visual_rotation, *visual_size;
      QtVariantProperty* visual_pos_x, *visual_pos_y, *visual_pos_z;
      QtVariantProperty* visual_rot_alpha, *visual_rot_beta, *visual_rot_gamma;
      QtVariantProperty* visual_size_x,*visual_size_y,*visual_size_z,*visual_size_d,*visual_size_r,*visual_size_h;

    };

  } // end of namespace gui
} // end of namespace mars

#endif
