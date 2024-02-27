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

#ifndef MARS_GRAPHICS_POSTDRAWCALLBACK_H
#define MARS_GRAPHICS_POSTDRAWCALLBACK_H

#include <osgViewer/Viewer>

#include <pthread.h>


namespace mars {
  namespace graphics {

    class PostDrawCallback : public osg::Camera::Camera::DrawCallback {
    public:
      PostDrawCallback(osg::Image* image);

      ~PostDrawCallback();

      virtual void operator () (osg::RenderInfo& renderInfo) const;

      void setSize(int width, int height);

      void setGrab(bool grab);
      void setSaveGrab(bool grab);

      void getImageData(void **data, int &width, int &height, unsigned long &image_time);

    private:
      osg::Image* _image;
      mutable unsigned long _image_time;
      int _width;
      int _height;
      bool _grab, _save_grab;
      unsigned long *image_id;
      pthread_mutex_t *imageMutex;
    };

  } // end of namespace graphics
} // end of namespace mars

#endif /* MARS_GRAPHICS_POSTDRAWCALLBACK_H */
