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
 * \file CameraConfiguratorGUI.h
 * \author Malte R�mmermann
 * \brief "CameraConfiguratorGUI" is a template for the widget interface of the MARS GUI
 **/

#ifndef CAMERA_CONFIGURATOR_GUI_H
#define CAMERA_CONFIGURATOR_GUI_H

#ifdef _PRINT_HEADER_
#warning "CameraConfiguratorGUI.h"
#endif

#include <QMutex>

#include "CameraConfig.h"
#include "mars/main_gui/BaseWidget.h"
#include "mars/main_gui/PropertyDialog.h"

namespace mars {
  namespace gui {

    class CameraConfiguratorGUI : public main_gui::BaseWidget,
                                  public main_gui::PropertyCallback {
      Q_OBJECT

      public:
      CameraConfiguratorGUI(interfaces::ControlCenter* c);
      ~CameraConfiguratorGUI();

      main_gui::PropertyDialog *pDialog;  

      void show(void) {if (pDialog) pDialog->show();}
      void hide(void) {if (pDialog) pDialog->hide();}
      bool isHidden(void) {if (pDialog) return pDialog->isHidden(); else return true;}
      void close(void) {if (pDialog) pDialog->close();}

      void addCamera(CameraConfig* camera);
      void removeCamera(CameraConfig* camera);

      void cameraSelected(int index);
      void checkStateChanged(bool checked);
      void checkRotationChanged(bool checked);
      void setOffsetPos(void);
      void setOffsetRot(void);
      void checkFrustumChanged(bool checked);
      void setFrustum(void);

    private:

      struct NodeElem {
        unsigned long id;
        std::string name;
        int index;
      };

      NodeElem *nodeElem;
      interfaces::ControlCenter *control;
      unsigned long frame;
      QMutex nodeMutex;
      QMutex boxmutex;
      unsigned long generic_id;
      unsigned int camera;
      bool take_events, first_camera, filled;
      bool set_frustum;

      QtVariantProperty *winIDCombo, *nodeIDCombo, *lockPos, *lockRot;
      QtVariantProperty *xPosBox, *yPosBox, *zPosBox, *xRotBox, *yRotBox, *zRotBox;
      QtVariantProperty *frustum, *frt_left, *frt_right, *frt_top, *frt_bottom, *frt_near, *frt_far;
      QtVariantProperty *save_config, *load_config, *state;

      std::vector<CameraConfig*> cameras;
      std::vector<NodeElem*> nodes;
      std::vector<double> frustumsettings;

      void updateFRTBoxes(std::vector<double> frustum);
      void updateGUI(void);

    protected slots:
      virtual void valueChanged(QtProperty *property, const QVariant &value);
      virtual void accept();
      virtual void reject();

      void saveConfig( );
      void loadConfig( );  
      void timerEvent(QTimerEvent* event);
  
    };

  } // end of namespace gui
} // end of namespace mars

#endif // BLENDER_EXPORT_H
