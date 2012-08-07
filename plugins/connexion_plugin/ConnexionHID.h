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

#ifndef MARS_PLUGINS_CONNEXIONHID_H
#define MARS_PLUGINS_CONNEXIONHID_H

#include <mars/interfaces/MARSDefs.h>

#define LOGITECH_VENDOR_ID          0x046d
#define LOGITECH_SPACE_TRAVELLER_DEVICE_ID 0xc623
#define LOGITECH_SPACE_PILOT_DEVICE_ID     0xc625
#define LOGITECH_SPACE_NAVIGATOR_DEVICE_ID 0xc626
#define LOGITECH_SPACE_EXPLORER_DEVICE_ID  0xc627


namespace mars {
  namespace plugins {
    namespace connexion_plugin {
      
      struct connexionValues {
        double tx;
        double ty;
        double tz;
        double rx;
        double ry;
        double rz;
        int button1;
        int button2;
      };
      
      int initConnexionHID(void *windowID);
      void getValue(interfaces::sReal *coordiantes, struct connexionValues *rawValues);
      void closeConnexionHID();
      
    } // end of namespace connxeion_plugin
  } // end of namespace plugins
} // end of namespace mars

#endif /* MARS_PLUGINS_CONNEXIONHID_H */
