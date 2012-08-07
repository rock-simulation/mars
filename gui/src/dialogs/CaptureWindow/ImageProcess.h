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

#ifndef IMAGE_PROCESS_H
#define IMAGE_PROCESS_H

#include <QThread>
#include <QMutex>
#include <vector>

#ifdef WIN32
 #include <cv.h>
 #include <highgui.h>
#else
 #include <opencv/cv.h>
 #include <opencv/highgui.h>
#endif

namespace mars {
  namespace gui {

    struct myImage {
      void *data;
      int width;
      int height;
    };

    class ImageProcess : public QThread {
    public:
      ImageProcess(QString folder, int framerate);
      ~ImageProcess();
      void addImage(myImage image);
      int getState(void) {return state;}
      int getPercent(void) {return percent;}

    protected:
      void run(void);
  
    private:
      QMutex listMutex;
      std::vector<myImage> imageList;
      bool processing;
      QString folder, file;
      int imageCount;
      int state;
      int percent;
      int file_count;
      int width, height;
      int framerate;
      CvVideoWriter *writer;
    };

  } // end of namespace gui
} // end of namespace mars

#endif // IMAGE_PROCESS_H
