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
 * \file CaptureConfig.h
 * \author Malte Römmermann
 * \brief "CaptureConfig"
 **/

#ifndef CAPTURE_CONFIG_H
#define CAPTURE_CONFIG_H

#ifdef _PRINT_HEADER_
#warning "CaptureConfig.h"
#endif

#include <mars/interfaces/sim/ControlCenter.h>

#include <QThread>

#include "ImageProcess.h"

namespace mars {

  namespace interfaces {
    class GraphicsCameraInterface;
    class GraphicsWindowInterface;
  }

  namespace gui {

    class CaptureConfig : public QThread {
      Q_OBJECT

      public:
      CaptureConfig(interfaces::ControlCenter *c);
      ~CaptureConfig();
  
      void setWindowID(unsigned long id);
      inline unsigned long getWindowID(void) {return win_id;}
      void setFrameRate(int frame_rate) {this->frame_rate = frame_rate;}
      inline int getFrameRate(void) {return frame_rate;}
      void startCapture(void);
      void stopCapture(void);
      bool isCapturing(void) {return capturing;}
      QString getState(void);

    protected:
      void run(void);

    private:
      interfaces::ControlCenter* control;
      interfaces::GraphicsWindowInterface *gw;
      interfaces::GraphicsCameraInterface* gc;
      ImageProcess *imageProcess;
      bool capture, capturing;
      int frame_rate;
      std::vector<myImage> imageList;

      unsigned long win_id;
    };

  } // end of namespace gui
} // end of namespace mars

#endif // CAPTURE_CONFIG_H
