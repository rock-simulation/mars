#ifndef MARS_PLUGINS_GAMEPAD_HPP
#define MARS_PLUGINS_GAMEPAD_HPP

#include <mars/interfaces/MARSDefs.h>


#define LOGITECH_VENDOR_ID          0x046d
#define LOGITECH_F510_ID            0xc21e
#define LOGITECH_SPACE_TRAVELLER_DEVICE_ID 0xc623
#define LOGITECH_SPACE_PILOT_DEVICE_ID     0xc625
#define LOGITECH_SPACE_NAVIGATOR_DEVICE_ID 0xc626
#define LOGITECH_SPACE_EXPLORER_DEVICE_ID  0xc627


namespace mars {
  namespace plugins {
    namespace gamepad_plugin {

      struct gamepadValues {
          gamepadValues():
            a1x(0),
            a1y(0),
            a2x(0),
            a2y(0),
            button1(0),
            button2(0)

          {
          }
        double a1x;
        double a1y;
        double a2x;
        double a2y;
        double button1;
        double button2;
      };

      int initGamepadHID(void *windowID);
      void getValue(struct gamepadValues *rawValues);
      void closeGamepadHID();

    } // end of namespace gamepad_plugin
  } // end of namespace plugins
} // end of namespace mars

#endif /* MARS_PLUGINS_GAMEPAD_HPP */
