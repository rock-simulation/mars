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
 * \file CaptureWindow.h
 * \author Malte Römmermann
 * \brief "CaptureWindow" provides some options to setup the camera of a 3d window
 **/

#ifndef CAPTURE_WINDOW_H
#define CAPTURE_WINDOW_H

#ifdef _PRINT_HEADER_
#warning "CaptureWindow.h"
#endif

#include <QTimer>
#include <QMutex>

#include "CaptureGUI.h"
#include <mars/main_gui/MenuInterface.h>
#include <mars/interfaces/graphics/GraphicsUpdateInterface.h>

namespace mars {

  namespace main_gui {
    class GuiInterface;
  }

  namespace gui {

    class CaptureWindow : public QTimer, main_gui::MenuInterface,
      interfaces::GraphicsUpdateInterface {
      Q_OBJECT

      public:
      CaptureWindow(interfaces::ControlCenter* c, main_gui::GuiInterface *gui);
      ~CaptureWindow();
  
      virtual void menuAction(int action, bool checked = false);
      virtual void preGraphicsUpdate(void);

    private:
      interfaces::ControlCenter *control;
      main_gui::GuiInterface *mainGui;
      CaptureGUI* myWidget;
      std::vector<CaptureConfig*> cameras;
      QMutex myMutex;

      bool init;
  
    protected slots:
      void closeWidget(void);
      void timerEvent(QTimerEvent* event);

    };

  } // end of namespace gui
} // end of namespace mars

#endif // CAPTURE_WINDOW_H
