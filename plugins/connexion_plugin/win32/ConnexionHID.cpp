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

#if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0501)
  #undef _WIN32_WINNT
  #define _WIN32_WINNT 0x0601
#endif

#include <mars/utils/Mutex.h>
#include <QApplication>
#include <cmath>
#include <windows.h>

namespace mars {
  namespace plugins {
    namespace connexion_plugin {

      static int idleFrameCount[3] = {0,0,0};
      static int idleThreshold = 4;

      bool myEventFilter(void *message, long *result);

      // let's have some global variables. great!
      static mars::utils::Mutex valueMutex;
      static connexionValues tmpValues;


      int registerRawDevices(HWND hwndMessagesWindow) {
        RAWINPUTDEVICE sRawInputDevices[1];
        sRawInputDevices[0].usUsagePage = 0x01;
        sRawInputDevices[0].usUsage     = 0x08;
        sRawInputDevices[0].dwFlags     = 0x00;
        sRawInputDevices[0].hwndTarget  = 0x00;
        //RIM_TYPEHID;

        unsigned int uiNumDevices = sizeof(sRawInputDevices)/sizeof(sRawInputDevices[0]);
        unsigned int cbSize = sizeof(sRawInputDevices[0]);

        unsigned int i;
        for (i = 0; i < uiNumDevices; ++i) {
          sRawInputDevices[i].hwndTarget = hwndMessagesWindow;
        }
        return RegisterRawInputDevices(sRawInputDevices, uiNumDevices, cbSize);
      }


      int initConnexionHID(void* windowID) {
        tmpValues.tx = 0.0;
        tmpValues.ty = 0.0;
        tmpValues.tz = 0.0;
        tmpValues.rx = 0.0;
        tmpValues.ry = 0.0;
        tmpValues.rz = 0.0;
        tmpValues.button1 = 0;
        tmpValues.button2 = 0;

        qApp->setEventFilter(myEventFilter);

        HWND handleForMessages = (HWND)windowID;
        return registerRawDevices(handleForMessages);
      }

      void getValue(mars::interfaces::sReal *coordinates,
                    struct connexionValues *rawValues) {
        valueMutex.lock();
        for(int i = 0; i < 3; ++i) {
          ++idleFrameCount[i];
        }
        *rawValues = tmpValues;

        coordinates[0] = tmpValues.tx * fabs(tmpValues.tx * 0.001);
        coordinates[1] = -tmpValues.tz * fabs(tmpValues.tz * 0.001);
        coordinates[2] = -tmpValues.ty * fabs(tmpValues.ty * 0.001);
        coordinates[3] = tmpValues.rx * fabs(tmpValues.rx * 0.0008);
        coordinates[4] = -tmpValues.rz * fabs(tmpValues.rz * 0.0008);
        coordinates[5] = -tmpValues.ry * fabs(tmpValues.ry * 0.0008);

        if(idleFrameCount[0] > idleThreshold) {
          tmpValues.tx = 0.0;
          tmpValues.ty = 0.0;
          tmpValues.tz = 0.0;
        }
        if(idleFrameCount[1] > idleThreshold) {
          tmpValues.rx = 0.0;
          tmpValues.ry = 0.0;
          tmpValues.rz = 0.0;
        }
        if(idleFrameCount[2] > idleThreshold) {
          tmpValues.button1 = 0;
          tmpValues.button2 = 0;
        }
        valueMutex.unlock();
      }

      void closeConnexionHID() {
      }


      bool myEventFilter(void *message, long *result) {

        MSG *msgStruct = (MSG*) message;

        if (msgStruct->message == WM_INPUT) {

    
          //cout << "Message: WM_INPUT erhalten" << endl;

          unsigned int dwSize = 0;

          GetRawInputData((HRAWINPUT)msgStruct->lParam, RID_INPUT, NULL,
                          &dwSize, sizeof(RAWINPUTHEADER));

          LPBYTE lpb = new BYTE[dwSize];

          if (GetRawInputData((HRAWINPUT)msgStruct->lParam, RID_INPUT,
                              lpb, &dwSize, sizeof(RAWINPUTHEADER))
              != dwSize) {
            delete[] lpb;
            return false;
          }

          RAWINPUT* raw = (RAWINPUT*) lpb;

          if (raw->header.dwType != RIM_TYPEHID) {
            delete[] lpb;
            return false;
          }

          RID_DEVICE_INFO sRidDeviceInfo;
          sRidDeviceInfo.cbSize = sizeof(RID_DEVICE_INFO);
          dwSize = sizeof(RID_DEVICE_INFO);

          if (GetRawInputDeviceInfo(raw->header.hDevice,
                                    RIDI_DEVICEINFO,
                                    &sRidDeviceInfo,
                                    &dwSize) == dwSize) {
            if (sRidDeviceInfo.hid.dwVendorId == LOGITECH_VENDOR_ID) {
              if (raw->data.hid.bRawData == 0x01) {
                // Translation vector

                short *pnData =  reinterpret_cast <short*> (&raw->data.hid.bRawData + 1);
                //cout << "packet1 X: " << pnData[0] << " Y: " << pnData[1] << " Z: " << pnData[2] << endl;
                //	  while(is_waiting) Sleep(1);

                valueMutex.lock();
                tmpValues.tx = pnData[0];
                tmpValues.ty = pnData[1];
                tmpValues.tz = pnData[2];
                idleFrameCount[0] = 0;
                valueMutex.unlock();
              } else if (raw->data.hid.bRawData == 0x02) {
                // Direct rotation vector (NOT Euler)
                short *pnData = reinterpret_cast <short*> (&raw->data.hid.bRawData + 1);
                //cout << "packet2 rX: " << pnData[0] << " rY: " << pnData[1] << " rZ: " << pnData[2] << endl;
                valueMutex.lock();
                tmpValues.rx = pnData[0];
                tmpValues.ry = pnData[1];
                tmpValues.rz = pnData[2];
                idleFrameCount[1] = 0;
                valueMutex.unlock();
              } else if (raw->data.hid.bRawData == 0x03) {
                // State of the keys
                unsigned long dwKeyState = *reinterpret_cast <unsigned long*> (&raw->data.hid.bRawData + 1);
                if (dwKeyState & 1) {
                  valueMutex.lock();
                  tmpValues.button1 = 1;
                  idleFrameCount[2] = 0;
                  valueMutex.unlock();
                }
                if (dwKeyState & 2) {
                  valueMutex.lock();
                  tmpValues.button2 = 1;
                  idleFrameCount[2] = 0;
                  valueMutex.unlock();
                }
                //cout << "key pressed: " << dwKeyState << endl;
              }
            }
          }

          // Do something
          *result = 1;
          delete[] lpb;
          return true;
        }
        else {
          // We do not want to handle this message so pass back to Windows
          // to handle it in a default way
          return false;
        }
      }

    } // end of namespace connexion_plugin
  } // end of namespace plugins
} // end of namespace mars
