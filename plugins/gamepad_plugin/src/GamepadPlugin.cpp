/**
 * \file GamepadPlugin.cpp
 * \author Malte Langosz
 * \brief ...
 */

#include "GamepadPlugin.hpp"
#include "GamepadHID.hpp"

#include <mars/interfaces/Logging.hpp>
#include <cstdio>

namespace mars {
  namespace plugins {
    namespace gamepad_plugin {

      using namespace lib_manager;
      using namespace mars::interfaces;
      using namespace mars::utils;

      GamepadPlugin::GamepadPlugin(LibManager *theManager)
        : MarsPluginTemplate(theManager, std::string("GamepadPlugin")) {

        thread_closed = false;
        name = "GamepadPlugin";

        isInit = false;
        newValues = new gamepadValues;
        init();
      }

      void GamepadPlugin::init() {
        if(isInit) return;
        // add options to the menu

        if (initGamepadHID(0)) {
          LOG_INFO("%s: Device registered",name.c_str());
          LOG_INFO("%s: running ...",name.c_str());
          isInit = true;
        } else {
          LOG_ERROR("%s: not able to register Device.",name.c_str());
          run_thread = false;
        }

        LOG_INFO("%s: loaded",name.c_str());
        if (isInit) {
          run_thread = true;

          dbPackageMapping.add("axis1/x", &(newValues->a1x));
          dbPackageMapping.add("axis1/y", &(newValues->a1y));
          dbPackageMapping.add("axis2/x", &(newValues->a2x));
          dbPackageMapping.add("axis2/y", &(newValues->a2y));
          dbPackageMapping.add("button1", &(newValues->button1));
          dbPackageMapping.add("button2", &(newValues->button2));
          control->dataBroker->registerTimedProducer(this, "GamepadPlugin", "values",
                                                     "_REALTIME_", 5);
          this->start();
        }
        else {
          run_thread = false;
          thread_closed = true;
          LOG_ERROR("%s: init not successful.",name.c_str());
        }

      }

      GamepadPlugin::~GamepadPlugin(void) {
        //fprintf(stderr, "Delete GamepadPlugin\n");
        run_thread = false;

        while (!thread_closed && isInit) {
          msleep(10);
        }

        if(isInit) closeGamepadHID();
        delete newValues;
      }

      void GamepadPlugin::produceData(const data_broker::DataInfo &info,
                                      data_broker::DataPackage *dbPackage,
                                      int callbackParam) {
        mutex.lock();
        dbPackageMapping.writePackage(dbPackage);
        mutex.unlock();
      }

      void GamepadPlugin::run() {

        while (run_thread) {
          mutex.lock();
          getValue(newValues);
          mutex.unlock();

          msleep(10);
        }
        LOG_INFO("%s: ... stopped",name.c_str());

        thread_closed = true;
      }

    } // end of namespace gamepad_plugin
  } // end of namespace plugins
} // end of namespace mars

DESTROY_LIB(mars::plugins::gamepad_plugin::GamepadPlugin);
CREATE_LIB(mars::plugins::gamepad_plugin::GamepadPlugin);
