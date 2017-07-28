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
 * \file GraphicsWindowInterface.h
 * \author Malte Roemmermann
 * \brief "GraphicsWindowInterface"
 *
 */

#ifndef MARS_INTERFACES_GRAPHICS_WINDOW_INTERFACE_H
#define MARS_INTERFACES_GRAPHICS_WINDOW_INTERFACE_H

#ifdef _PRINT_HEADER_
  #warning "GraphicsWindowInterface.h"
#endif

#include "GraphicsCameraInterface.h"
#include "GraphicsEventInterface.h"
#include <mars/utils/Color.h>

namespace osg{
    class Group;
}

namespace mars {
  namespace interfaces {

    class GraphicsWindowInterface {

    public:

      virtual ~GraphicsWindowInterface() {}
      virtual const std::string getName() const = 0;
      virtual GraphicsCameraInterface* getCameraInterface() const = 0;
      virtual void grabFocus() = 0;
      virtual void setClearColor(mars::utils::Color color) = 0;
      virtual const mars::utils::Color& getClearColor() const = 0;
      virtual void switchHudElemtVis(int num_element) = 0;
      virtual void setFullscreen(bool val, int display = 1) = 0;
      virtual void setGrabFrames(bool grab) = 0;
      virtual void setSaveFrames(bool grab) = 0;
      /**
       * This function copies the image data in the given buffer.
       * It assumes that the buffer ist correctly initalized
       * with a char array of the size width * height * 4
       * 
       * @param buffer buffer in which the image gets copied
       * @param width returns the width of the image
       * @param height returns the height of the image
       * */
      virtual void getImageData(char *buffer, int &width, int &height) = 0;
      virtual void getImageData(void **data, int &width, int &height) = 0;
      
      /**
       * This function copies the depth image in the given buffer.
       * It assumes that the buffer ist correctly initalized
       * with a double array of the size width * height
       * 
       * @param buffer buffer in which the image gets copied
       * @param width returns the width of the image
       * @param height returns the height of the image
       * */
      virtual void getRTTDepthData(float *buffer, int &width, int &height) = 0;
      virtual void getRTTDepthData(float **data, int &width, int &height) = 0;      
      virtual osg::Group* getScene() = 0;
      virtual void setScene(osg::Group *scene) = 0;
      virtual void addGraphicsEventHandler(GraphicsEventInterface *graphicsEventHandler) = 0;

      virtual void setHUDViewOffsets(double x1, double y1,
                                     double x2, double y2) = 0;

    }; // end of class GraphicsWindowInterface

  } // end of namespace interfaces
} // end of namespace mars

#endif  /* MARS_INTERFACES_GRAPHICS_WINDOW_INTERFACE_H */
