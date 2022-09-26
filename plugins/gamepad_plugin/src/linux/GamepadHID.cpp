#include "../GamepadHID.hpp"

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
    namespace gamepad_plugin {

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
               (device_info.product == LOGITECH_F510_ID)) {
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

      int initGamepadHID(void *windowID) {
        getFileDescriptor();
        if(fd < 0) {
          return 0;
        }
        return 1;
      }

      void closeGamepadHID() {
        if(fd > 0) {
          close(fd);
        }
        fd = -1;
      }

      void getValue(struct gamepadValues *rawValues) {
        int i, eventCnt;
        /* how many bytes were read */
        size_t bytesRead;
        /* the events (up to 64 at once) */
        struct input_event events[64];
        /* keep track of idle frames for each DoF for smoother animation. see above */
        static int idleFrameCount[6] = {0, 0, 0, 0, 0, 0};
        int idleThreshold = 30;

        /* read the raw event data from the device */
        bytesRead = read(fd, events, sizeof(struct input_event) * 64);
        eventCnt = (int) ((long)bytesRead / (long)sizeof(struct input_event));
        if (bytesRead < (int) sizeof(struct input_event)) {
          perror("evtest: short read");
          return;
        }

        /* Increase all idle counts. They are later reset if there is activity */
        for(i = 4; i < 6; ++i) {
          ++idleFrameCount[i];
        }

        /* handle input events sequentially */
        for(i = 0; i < eventCnt; ++i) {
          if(EV_KEY == events[i].type) {
            switch(events[i].code) {
            case BTN_SOUTH:
              if(events[i].value) {
                rawValues->button1 = events[i].value;
              }
              idleFrameCount[4] = 0;
              break;
            case BTN_EAST:
              if(events[i].value) {
                rawValues->button2 = events[i].value;
              }
              idleFrameCount[5] = 0;
              break;
            }
          } else if(EV_ABS == events[i].type) {
            switch(events[i].code) {
            case ABS_X:
              rawValues->a1x = events[i].value;
              idleFrameCount[0] = 0;
              break;
            case ABS_Y:
              rawValues->a1y = events[i].value;
              idleFrameCount[1] = 0;
              break;
            case ABS_RX:
              rawValues->a2x = events[i].value;
              idleFrameCount[2] = 0;
              break;
            case ABS_RY:
              rawValues->a2y = events[i].value;
              idleFrameCount[3] = 0;
              break;
            }
          }
        }

        /* Set rawValue to zero if DoF was idle for more than idleThreshold frames */
        for(i = 0; i < 6; ++i) {
          if(idleFrameCount[i] >= idleThreshold) {
            if(0==i) {
              rawValues->a1x = 0;
            } else if (1==i) {
              rawValues->a1y = 0;
            } else if (2==i) {
              rawValues->a2x = 0;
            } else if (3==i) {
              rawValues->a2y = 0;
            } else if (4==i) {
              rawValues->button1 = 0;
            } else if (5==i) {
              rawValues->button2 = 0;
            }
          }
        }
      }

    } // end of namespace gamepad_plugin
  } // end of namespace plugins
} // end of namespace mars
