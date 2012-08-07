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
 * \file Dialog_Graphics_Options.h
 */

#ifndef DIALOG_GRAPHICS_OPTIONS_H
#define DIALOG_GRAPHICS_OPTIONS_H

#ifdef _PRINT_HEADER_
#warning "Dialog_Graphics_Options.h"
#endif


#include <mars/main_gui/BaseWidget.h>
#include <mars/main_gui/PropertyDialog.h>
#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/interfaces/GraphicData.h>

namespace mars {

  namespace main_gui {
    class GuiInterface;
  }

  namespace interfaces {
    class GraphicsCameraInterface;
  }  

  namespace gui {

    /**
     * This class creates the Register Frame Dialog
     */
    class Dialog_Graphics_Options : public main_gui::BaseWidget,
                                    public main_gui::PropertyCallback {
      Q_OBJECT
  
      public:
      Dialog_Graphics_Options(interfaces::ControlCenter* c, main_gui::GuiInterface *gui);
      ~Dialog_Graphics_Options();
  
      main_gui::PropertyDialog *pDialog;
      void show(void) {pDialog->show();}
      void hide(void) {pDialog->hide();}
      bool isHidden(void) {return pDialog->isHidden();}
      void close(void) {pDialog->close();}

    private slots:
      virtual void valueChanged(QtProperty *property, const QVariant &value);
      virtual void accept();
      virtual void reject();
      void closeDialog();


    private:
      interfaces::GraphicData gs_backup;
      bool coords_backup;
      bool clouds_backup;
      bool grid_backup;
  
      struct cam_backup_struct {
        interfaces::GraphicsCameraInterface *g_cam;
        int camera_backup;
        double rx_backup, ry_backup, tx_backup, ty_backup, tz_backup, rz_backup;   
      };

      std::vector<cam_backup_struct> cam_backup;

      interfaces::ControlCenter* control;
      main_gui::GuiInterface *mainGui;

      bool filled;

      std::vector<QtVariantProperty*> winIds;
      QtVariantProperty *clearColor, *fogEnabled, *fogDensity, *fogStart, *fogEnd, *fogColor;
      QtVariantProperty *coords, *clouds, *grid, *camera;
      QtVariantProperty *cam_type, *viewport, *pos, *rot;
      QtVariantProperty *rot_x, *rot_y, *rot_z, *pos_x, *pos_y, *pos_z;
  
      void update();
      void on_change_window(int index);
      void on_change_camera_type(int index);
      void on_change_viewport(int index);
      void on_change_fog(bool enabled);
      utils::Color to_my_color(QColor color);
      QColor to_QColor(utils::Color color);
   
    };

  } // end of namespace gui
} // end of namespace mars

#endif
