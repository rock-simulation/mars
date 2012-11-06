/**
 *  Copyright 2011, DFKI GmbH Robotics Innovation Center
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

#include <math.h>
#include <Carbon/Carbon.h> 
#include <AvailabilityMacros.h>
#include <IOKit/hid/IOHIDLib.h>
#include "../ConnexionHID.h"

namespace mars {
  namespace plugins {
    namespace connexion_plugin {

      IOHIDManagerRef myIOHIDManagerRef = NULL;
      CFMutableArrayRef myDeviceCFArrayRef = NULL;
      IOHIDDeviceRef myIOHIDDeviceRef = NULL;
      CFArrayRef myElementCFArrayRef = NULL;
      IOHIDElementRef myElementRef[8] = {NULL, NULL, NULL, NULL,
                                         NULL, NULL, NULL, NULL};
      //IOHIDTransactionRef transaction = NULL;

      static const ControlID gScrolledControlID = { 'Scrl', 0 };
      static void CFSetApplierFunctionCopyToCFArray(const void *value,
                                                    void *context) {
        CFArrayAppendValue((CFMutableArrayRef) context, value);
      }

      static Boolean IOHIDDevice_GetLongProperty(IOHIDDeviceRef inIOHIDDeviceRef,
                                                 CFStringRef inKey,
                                                 long *outValue) {
        Boolean result = FALSE;
	
        if(inIOHIDDeviceRef) {
          CFTypeRef tCFTypeRef = IOHIDDeviceGetProperty(inIOHIDDeviceRef, inKey);
          if(tCFTypeRef) {
            // if this is a number
            if(CFNumberGetTypeID() == CFGetTypeID(tCFTypeRef)) {
              // get it's value
              result = CFNumberGetValue((CFNumberRef) tCFTypeRef,
                                        kCFNumberSInt32Type, outValue);
            }
          }
        }
        return result;
      }	// IOHIDDevice_GetLongProperty

      int initConnexionHID(void *windowID) {
        (void) windowID;
        // init device manager
        myIOHIDManagerRef = IOHIDManagerCreate(kCFAllocatorDefault,
                                               kIOHIDOptionsTypeNone );
  
        if(myIOHIDManagerRef) {
          // open it
          IOReturn tIOReturn = IOHIDManagerOpen(myIOHIDManagerRef,
                                                kIOHIDOptionsTypeNone);
          if(kIOReturnSuccess != tIOReturn) {
            fprintf(stderr, "%s: Couldn't open IOHIDManager.",
                    __PRETTY_FUNCTION__);
            return 0;
          }
        } else {
          fprintf(stderr, "%s: Couldn't create a IOHIDManager.",
                  __PRETTY_FUNCTION__);
          return 0;
        }
  
        // set it for IOHIDManager to use to match against
        IOHIDManagerSetDeviceMatchingMultiple(myIOHIDManagerRef,
                                              NULL );
        // get device list
        CFSetRef devCFSetRef = IOHIDManagerCopyDevices(myIOHIDManagerRef);
        if(devCFSetRef) {
          // create an empty array
          myDeviceCFArrayRef = CFArrayCreateMutable( kCFAllocatorDefault, 0,
                                                     &kCFTypeArrayCallBacks );
          // now copy the set to the array
          CFSetApplyFunction( devCFSetRef, CFSetApplierFunctionCopyToCFArray,
                              ( void * ) myDeviceCFArrayRef );
          // and release the set we copied from the IOHID manager
          CFRelease( devCFSetRef );
        }

        // get the space navigator
        if(!myDeviceCFArrayRef) return 0;
        CFIndex idx, cnt = CFArrayGetCount( myDeviceCFArrayRef );
        char product[255] = "";
        char target_product[255] = "SpaceNavigator";
        char target_product2[255] = "Space Navigator";
        //char target_product[255] = "Apple Optical USB Mouse";
        for ( idx = 0; idx < cnt; idx++ ) {
          myIOHIDDeviceRef = (IOHIDDeviceRef)CFArrayGetValueAtIndex(myDeviceCFArrayRef, idx);
    
    
          CFStringRef string = (CFStringRef)IOHIDDeviceGetProperty(myIOHIDDeviceRef,
                                                                   CFSTR(kIOHIDProductKey));
          if(string) {
            verify(CFStringGetCString(string, product, sizeof(product),
                                      kCFStringEncodingUTF8));
          }
          long result = 0;
          (void) IOHIDDevice_GetLongProperty(myIOHIDDeviceRef,
                                             CFSTR(kIOHIDProductIDKey), &result);

          if(!strcmp(target_product, product)) {
            break;
          }
          if(!strcmp(target_product2, product)) {
            break;
          }
        }
        myElementCFArrayRef = IOHIDDeviceCopyMatchingElements(myIOHIDDeviceRef, 
                                                              NULL, 0);
        if(myElementCFArrayRef) {
          CFIndex eleIndex, eleCount = CFArrayGetCount(myElementCFArrayRef);
          int e_count = 0;
          for(eleIndex=0; eleIndex<eleCount; eleIndex++) {
            IOHIDElementRef tIOHIDElementRef = (IOHIDElementRef)CFArrayGetValueAtIndex(myElementCFArrayRef, eleIndex);
            if ( !tIOHIDElementRef ) continue;
            IOHIDElementType tIOHIDElementType = IOHIDElementGetType(tIOHIDElementRef);
            if(tIOHIDElementType>kIOHIDElementTypeInput_ScanCodes) continue;
            long usagePage = IOHIDElementGetUsagePage(tIOHIDElementRef);
            long usage = IOHIDElementGetUsage(tIOHIDElementRef);
            if ( !usagePage || !usage ) continue;
            if ( -1 == usage ) continue;
            myElementRef[e_count++] = tIOHIDElementRef;
          }
    				
        }
        return 1;
      }

      void getValue(double *coordinates, struct connexionValues *rawValues) {
        struct connexionValues result;
        double values[8];
        int el;
        IOHIDValueRef value = NULL;

        for(el = 0; el < 8; el++) {
          if(myElementRef[el] != NULL) {
            if(IOHIDDeviceGetValue(myIOHIDDeviceRef, myElementRef[el],
                                   &value) == kIOReturnSuccess) {
              values[el] = (double)IOHIDValueGetIntegerValue(value);
            }
          }
        }
        rawValues->tx = values[0];
        rawValues->ty = values[1];
        rawValues->tz = values[2];
        rawValues->rx = values[3];
        rawValues->ry = values[4];
        rawValues->rz = values[5];
        rawValues->button1 = (int)values[6];
        rawValues->button2 = (int)values[7];

        coordinates[0] = rawValues->tx * fabs(rawValues->tx * 0.001);
        coordinates[1] = -rawValues->ty * fabs(rawValues->ty * 0.001);
        coordinates[2] = rawValues->tz * fabs(rawValues->tz * 0.001);
        coordinates[3] = rawValues->rx * fabs(rawValues->rx * 0.01);
        coordinates[4] = -rawValues->ry * fabs(rawValues->ry * 0.01);
        coordinates[5] = rawValues->rz * fabs(rawValues->rz * 0.01);
      }

      void closeConnexionHID() {
        int i;

        if(myIOHIDManagerRef) {
          IOHIDManagerClose(myIOHIDManagerRef, 0);
        }
        CFRelease(myDeviceCFArrayRef);
        CFRelease(myElementCFArrayRef);
      }

    } // end of namespace connexion_plugin
  } // end of namespace plugins
} // end of namespace mars
