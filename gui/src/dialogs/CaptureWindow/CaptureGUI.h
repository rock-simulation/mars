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
 * \file CaptureGUI.h
 * \author Malte Römmermann
 * \brief "CaptureGUI" is a template for the widget interface of the MARS GUI
 **/

#ifndef CAPTURE_GUI_H
#define CAPTURE_GUI_H

#ifdef _PRINT_HEADER_
#warning "CaptureGUI.h"
#endif

#include "mars/main_gui/BaseWidget.h"
#include "mars/main_gui/PropertyDialog.h"
#include "CaptureConfig.h"

#include <QDoubleSpinBox>

namespace mars {
  namespace gui {

    class CaptureGUI : public main_gui::BaseWidget,
                       public main_gui::PropertyCallback {
      Q_OBJECT

      public:
      CaptureGUI(interfaces::ControlCenter* c);
      ~CaptureGUI();
  
      main_gui::PropertyDialog *pDialog;  
 
      void show(void) {if (pDialog) pDialog->show();}
      void hide(void) {if (pDialog) pDialog->hide();}
      bool isHidden(void) {if (pDialog) return pDialog->isHidden(); else return true;}
      void close(void) {if (pDialog!=NULL) pDialog->close();}

      void addCamera(CaptureConfig* camera);
      void removeCamera(CaptureConfig* camera);
      void cameraSelected(int index);
      void setFrameRate(double frameRate);
      void startCapture(void);
      void stopCapture(void);

    private:
      interfaces::ControlCenter *control;
 
      QtVariantProperty *status;
      QtVariantProperty *winIDCombo;
      QtVariantProperty *frameBox;
      QtVariantProperty *capture;
      unsigned int camera;
      bool take_events, first_camera, update_frame_box, filled;

      std::vector<CaptureConfig*> cameras;
      QPushButton* generatePushButton(QString objectName, QString text);
      QLayout* generateDoubleBox(QDoubleSpinBox *&theBox,
                                 QString objectName,
                                 QString text);
      void updateGUI(void);

    protected slots:
      void timerEvent(QTimerEvent* event);
      virtual void valueChanged(QtProperty *property, const QVariant &value);
      virtual void accept();
      virtual void reject();
  
    };

  } // end of namespace gui
} // end of namespace mars

#endif // CAPTURE_GUI_H
