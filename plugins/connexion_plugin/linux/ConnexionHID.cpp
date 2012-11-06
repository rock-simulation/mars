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

#include "../ConnexionHID.h"

#include <mars/interfaces/MARSDefs.h>

#include <linux/input.h>
#include <dirent.h>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cmath>

#define PATH_BUFFER_SIZE (1024)


namespace mars {
  namespace plugins {
    namespace connexion_plugin {

      /* file descriptor of the /dev/input node of the SpaceMouse */
      static int fd = -1;


      /* Scan all devices in /dev/input/ to find the SpaceMouse.
       * Returns the file descriptor of the SpaceMouse or -1 if it could not be found.
       */
      static int getFileDescriptor() {
        struct dirent *entry;
        DIR *dp;
        char path[PATH_BUFFER_SIZE];
        struct input_id device_info;
        const char *devDirectory = "/dev/input/";

        /* open the directory */
        dp = opendir(devDirectory);
        if(dp == NULL) {
          return -1;
        }
        /* walk the directory (non-recursively) */
        while((entry = readdir(dp))) {
          strncpy(path, devDirectory, sizeof(path));
          /* if strlen(devDirectory) > sizeof(path) the path won't be NULL terminated
           * and *bad things* will happen. Therfore, we force NULL termination.
           */
          path[PATH_BUFFER_SIZE-1] = '\0';
          strncat(path, entry->d_name, sizeof(path));

          fd = open(path, O_RDONLY | O_NONBLOCK);
          if(-1 == fd) {
            /* could not open file. probably we do not have read permission. */
            continue;
          }

          /* try to read the vendor and device ID */
          if(!ioctl(fd, EVIOCGID, &device_info)) {
            if((device_info.vendor == LOGITECH_VENDOR_ID) && 
               (device_info.product == LOGITECH_SPACE_NAVIGATOR_DEVICE_ID)) {
              /* BINGO!!! this is it! */
              break;
            }
          }
          close(fd);
          fd = -1;
        }

        if(-1 == fd) {
          fprintf(stderr, 
                  "ERROR: could not find SpaceMouse! \n"
                  "       Do you have read permission on the /dev/input/ device?\n");
        }
        closedir(dp);
        return fd;
      }

      int initConnexionHID(void *windowID) {
        getFileDescriptor();
        if(fd < 0) {
          return 0;
        }
        return 1;
      }

      void closeConnexionHID() {
        if(fd > 0) {
          close(fd);
        }
        fd = -1;
      }

      void getValue(mars::interfaces::sReal *coordinates,
                    struct connexionValues *rawValues) {
        /* If input events don't come in fast enough a certain DoF may not be 
         * updated during a frame. This results in choppy and ugly animation.
         * To solve this we record the number of frames a certain DoF was idle
         * and only set the DoF to 0 if we reach a certain idleThreshold.
         * When there is activity on a axis the idleFrameCount is reset to 0.
         */
        int i, eventCnt;
        /* how many bytes were read */
        size_t bytesRead;
        /* the events (up to 64 at once) */
        struct input_event events[64];
        /* buffer */
        static struct connexionValues valueBuffer;
        /* keep track of idle frames for each DoF for smoother animation. see above */
        static int idleFrameCount[6] = {0, 0, 0, 0, 0, 0};
        int idleThreshold = 3;

        /* read the raw event data from the device */
        bytesRead = read(fd, events, sizeof(struct input_event) * 64);
        eventCnt = (int) ((long)bytesRead / (long)sizeof(struct input_event));
        if (bytesRead < (int) sizeof(struct input_event)) {
          perror("evtest: short read");
          return;
        }

        /* Increase all idle counts. They are later reset if there is activity */
        for(i = 0; i < 6; ++i) {
          ++idleFrameCount[i];
        }

        /* handle input events sequentially */
        for(i = 0; i < eventCnt; ++i) {
          if(EV_KEY == events[i].type) {
            switch(events[i].code) {
            case BTN_0:
              rawValues->button1 = events[i].value;
              break;
            case BTN_1:
              rawValues->button2 = events[i].value;
              break;
            }
          } else if(EV_REL == events[i].type) {
            switch(events[i].code) {
            case REL_X:
              rawValues->tx = events[i].value;
              idleFrameCount[0] = 0;
              break;
            case REL_Y:
              rawValues->ty = events[i].value;
              idleFrameCount[1] = 0;
              break;
            case REL_Z:
              rawValues->tz = events[i].value;
              idleFrameCount[2] = 0;
              break;
            case REL_RX:
              rawValues->rx = events[i].value;
              idleFrameCount[3] = 0;
              break;
            case REL_RY:
              rawValues->ry = events[i].value;
              idleFrameCount[4] = 0;
              break;
            case REL_RZ:
              rawValues->rz = events[i].value;
              idleFrameCount[5] = 0;
              break;
            }
          }
        }

        /* Set rawValue to zero if DoF was idle for more than idleThreshold frames */
        for(i = 0; i < 6; ++i) {
          if(idleFrameCount[i] >= idleThreshold) {
            if(0==i) {
              rawValues->tx = 0;
            } else if (1==i) {
              rawValues->ty = 0;
            } else if (2==i) {
              rawValues->tz = 0;
            } else if (3==i) {
              rawValues->rx = 0;
            } else if (4==i) {
              rawValues->ry = 0;
            } else if (5==i) {
              rawValues->rz = 0;
            }
          }
        }

        coordinates[0] = rawValues->tx * fabs(rawValues->tx * 0.001);
        coordinates[1] = -rawValues->tz * fabs(rawValues->tz * 0.001);
        coordinates[2] = -rawValues->ty * fabs(rawValues->ty * 0.001);
        coordinates[3] = rawValues->rx * fabs(rawValues->rx * 0.0008);
        coordinates[4] = -rawValues->rz * fabs(rawValues->rz * 0.0008);
        coordinates[5] = -rawValues->ry * fabs(rawValues->ry * 0.0008);
      }

    } // end of namespace connexion_plugin
  } // end of namespace plugins
} // end of namespace mars
