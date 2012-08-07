/*
 *  Copyright 2012, DFKI GmbH Robotics Innovation Center
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

/**
 * \file __classname__.h
 * \author __author__ (__email__)
 * \brief __description__
 *
 * Version 0.1
 */

#ifndef __headerDef__
#define __headerDef__

#ifdef _PRINT_HEADER_
  #warning "__classname__.h"
#endif

#include <mars/interfaces/sim/MarsPluginTemplate.h>
#include <mars/interfaces/MARSDefs.h>
#include <mars/data_broker/ReceiverInterface.h>

#include <string>

namespace mars {

  namespace lib_manager {
    class LibManager;
  }

  namespace plugins {
    namespace __namespace__ {

      class __classname__: public mars::interfaces::MarsPluginTemplate, 
                         public mars::data_broker::ReceiverInterface {

      public:
        __classname__(mars::lib_manager::LibManager *theManager);
        ~__classname__();
        
        // LibInterface methods
        int getLibVersion() const
        { return 1; }
        const std::string getLibName() const
        { return std::string("__project__"); }

        // MarsPlugin methods
        void init();
        void reset();
        void update(mars::interfaces::sReal time_ms);
        
        // DataBrokerReceiver methods
        virtual void receiveData(const data_broker::DataInfo &info,
                                 const data_broker::DataPackage &package,
                                 int callbackParam);
        
        // __classname__ methods
        
      private:
        
      }; // end of class definition __classname__
      
    } // end of namespace __namespace__
  } // end of namespace plugins
} // end of namespace mars

#endif // __headerDef__
