#include <math.h>
#include <Carbon/Carbon.h> 
#include <AvailabilityMacros.h>
#include <IOKit/hid/IOHIDLib.h>
#include "../GamepadHID.hpp"

namespace mars {
  namespace plugins {
    namespace gamepad_plugin {

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

      int initGamepadHID(void *windowID) {
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
        char target_product[255] = "USB Joystick";
        //char target_product2[255] = "Space Navigator";
        //char target_product[255] = "Apple Optical USB Mouse";
        bool found_device = false;
        for ( idx = 0; idx < cnt; idx++ ) {
          myIOHIDDeviceRef = (IOHIDDeviceRef)CFArrayGetValueAtIndex(myDeviceCFArrayRef, idx);
    
    
          CFStringRef string = (CFStringRef)IOHIDDeviceGetProperty(myIOHIDDeviceRef,
                                                                   CFSTR(kIOHIDProductKey));
          if(string) {
            CFStringGetCString(string, product, sizeof(product),
                               kCFStringEncodingUTF8);
          }
          long result = 0;
          (void) IOHIDDevice_GetLongProperty(myIOHIDDeviceRef,
                                             CFSTR(kIOHIDProductIDKey), &result);

          if(!strcmp(target_product, product)) {
            found_device = true;
            break;
          }
          // if(!strcmp(target_product2, product)) {
          //   found_device = true;
          //   break;
          // }
        }
        if(!found_device) return 0;
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

      void getValue(struct gamepadValues *rawValues) {
        struct gamepadValues result;
        double values[17];
        int el;
        IOHIDValueRef value = NULL;

        for(el = 0; el < 17; el++) {
          if(myElementRef[el] != NULL) {
            if(IOHIDDeviceGetValue(myIOHIDDeviceRef, myElementRef[el],
                                   &value) == kIOReturnSuccess) {
              values[el] = (double)IOHIDValueGetIntegerValue(value);
              //fprintf(stderr, "%d: %g\n", el, values[el]);
            }
          }
        }

        rawValues->a1x = (values[15]-128)*250;
        rawValues->a1y = (values[16]-128)*250;
        rawValues->a2x = (values[13]-128)*250;
        rawValues->a2y = (values[14]-128)*250;
        rawValues->button1 = values[2];
        rawValues->button2 = values[1];
      }

      void closeGamepadHID() {
        int i;

        if(myIOHIDManagerRef) {
          IOHIDManagerClose(myIOHIDManagerRef, 0);
        }
        CFRelease(myDeviceCFArrayRef);
        CFRelease(myElementCFArrayRef);
      }

    } // end of namespace gamepad_plugin
  } // end of namespace plugins
} // end of namespace mars
