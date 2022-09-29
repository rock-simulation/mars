 /**
 * \file GamepadPlugin.hpp
 * \author Malte Langosz
 * \brief ...
 */

#ifndef MARS_PLUGINS_GAMEPAD_PLUGIN_H
#define MARS_PLUGINS_GAMEPAD_PLUGIN_H

#include <lib_manager/LibInterface.hpp>
#include <mars/data_broker/ProducerInterface.h>
#include <mars/data_broker/ReceiverInterface.h>
#include <mars/data_broker/DataPackageMapping.h>
#include <mars/interfaces/MARSDefs.h>
#include <mars/utils/Vector.h>
#include <mars/utils/Quaternion.h>
#include <mars/interfaces/sim/ControlCenter.h>
#include <mars/interfaces/sim/MarsPluginTemplate.h>
#include <mars/utils/Thread.h>
#include <mars/utils/Mutex.h>

#include <string>
#include <cstdarg>
#include <cmath>



namespace mars {
  namespace plugins {
    namespace gamepad_plugin {

      struct gamepadValues;

      class GamepadPlugin : public utils::Thread,
                            public mars::interfaces::MarsPluginTemplate,
                            public data_broker::ProducerInterface {

        public:
        GamepadPlugin(lib_manager::LibManager *theManager);
        ~GamepadPlugin(void);

        // LibInterface methods
        int getLibVersion() const {return 1;}
        const std::string getLibName() const {return std::string("GamepadPlugin");}
        CREATE_MODULE_INFO();

        void update(mars::interfaces::sReal time_ms) { (void) time_ms; }
        virtual void reset(void) {}
        virtual void init(void);

        virtual void produceData(const data_broker::DataInfo &info,
                               data_broker::DataPackage *package,
                               int callbackParam);

      private:
        std::string name;
        bool isInit;
        utils::Mutex mutex;

        gamepadValues *newValues;
        bool run_thread;
        bool thread_closed;

        data_broker::DataPackageMapping dbPackageMapping;

      protected:
        void run(void);

      }; // end of class GamepadPlugin

    } // end of namespace gamepad_plugin
  } // end of namespace plugins
} // end of namespace mars

#endif /* MARS_PLUGINS_GAMEPAD_PLUGIN_HPP */
