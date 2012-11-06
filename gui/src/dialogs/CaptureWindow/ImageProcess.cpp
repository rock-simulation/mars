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

#include "ImageProcess.h"
#include <cstdio>

namespace mars {
  namespace gui {

    ImageProcess::ImageProcess(QString folder, int framerate) {
      this->folder = folder;
      processing = true;
      start();
      state = 1;
      width = height = 0;
      writer = 0;
      this->framerate = framerate;
      fprintf(stderr, "created ImagePorcess\n");
    }

    ImageProcess::~ImageProcess() {
      bool wait = true;

      state = 2;
      while(wait) {
        listMutex.lock();
        if(imageList.size() == 0) wait = false;
        listMutex.unlock();
        msleep(100);
      }

      processing = false;
      while(isRunning()) msleep(100);

      fprintf(stderr, "destroyed ImagePorcess\n");
    }

    void ImageProcess::addImage(myImage image) {
      listMutex.lock();
      if(processing) imageList.push_back(image);
      listMutex.unlock();
    }

    void ImageProcess::run(void) {
      myImage newImage;
      uchar *data = 0;
      uchar *dest, *src;
      QString num;
      IplImage *cvImage = 0;

      imageCount = 0;
      file_count = 0;

      file = folder;
      file.append("/image_process_");
      num.setNum(++file_count);
      file.append(num);
      file.append(".avi");

      while(processing) {
        listMutex.lock();
        if(imageList.size() > 0) {

          // get first image
          newImage = imageList[0];
          imageList.erase(imageList.begin());
          listMutex.unlock();
      
          if(width == 0) {
            width = newImage.width;
            height = newImage.height;
            writer = cvCreateVideoWriter(qPrintable(file),
                                         //-1, framerate,
                                         CV_FOURCC('X', 'V', 'I', 'D'), framerate,
                                         //CV_FOURCC('M', 'J', 'P', 'G'), framerate,
                                         cvSize(width, height), 1); 
            cvImage = cvCreateImageHeader(cvSize(width, height), IPL_DEPTH_8U, 3);
            data = (uchar*)malloc(width*height*3);
            cvImage->imageData = (char*)data;
          }
          // convert to standard rgb image
          for (int i=0; i<newImage.height; ++i) {
            for(int k=0; k<newImage.width; ++k) {
              dest = data + ((newImage.height-1)*(newImage.width)*3-
                             i*newImage.width*3 + k*3);

              src  = (uchar*)newImage.data + (i*newImage.width*4) + k*4;

              memcpy(dest, src, 3);
            }
          }

          // save new Image
          if(writer) {
            cvWriteFrame(writer, cvImage);
            ++imageCount;
          }

          // free memory
          free(newImage.data);
        }
        else {
          listMutex.unlock();
          msleep(4);
        }
      }
  
      // clean up
      if(writer) cvReleaseVideoWriter(&writer);  
      if(cvImage) cvReleaseImageHeader(&cvImage);
      if(data) free(data);
    }

  } // end of namespace gui
} // end of namespace mars
