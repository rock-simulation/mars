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

 /**
 * \file ConnexionPlugin.h
 * \author Michael Rohn
 * \brief A Plugin to use the 3D-Mouse by 3Dconnexion in MARS to control the camera
 */

#ifndef MARS_PLUGINS_CONNEXIONPLUGIN_H
#define MARS_PLUGINS_CONNEXIONPLUGIN_H

#if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0501)
  #undef _WIN32_WINNT
  #define _WIN32_WINNT 0x0601
#endif

#include "ConnexionWidget.h"

#include <lib_manager/LibInterface.hpp>
#include <mars/interfaces/MARSDefs.h>
#include <mars/utils/Vector.h>
#include <mars/utils/Quaternion.h>
#include <mars/interfaces/sim/ControlCenter.h>
#define PLUGIN_WITH_MARS_GUI // <- define this before MarsPluginTemplate
#include <mars/interfaces/sim/MarsPluginTemplate.h>
#include <mars/interfaces/graphics/GraphicsUpdateInterface.h>
#include <QThread>
#include <QMutex>
#include <string>
#include <cstdarg>
#include <cmath>



namespace mars {
  namespace plugins {
    namespace connexion_plugin {

      class ControlCenter;
      struct connexionValues;

      class ConnexionPlugin : public QThread, 
                              public mars::interfaces::MarsPluginTemplateGUI,
                              public mars::main_gui::MenuInterface,
                              public mars::interfaces::GraphicsUpdateInterface {

        Q_OBJECT

        public:
        ConnexionPlugin(lib_manager::LibManager *theManager);
        ~ConnexionPlugin(void);

        // LibInterface methods
        int getLibVersion() const {return 1;}
        const std::string getLibName() const {return std::string("ConnexionPlugin");}
        CREATE_MODULE_INFO();

        void update(mars::interfaces::sReal time_ms) { (void) time_ms; }
        virtual void reset(void) {}
        virtual void init(void);
        virtual void preGraphicsUpdate(void);

        virtual void menuAction(int action, bool checked = false);

#ifdef WIN32
        static bool myEventFilter(void *message, long *result);
#endif // WIN32

      private:
        std::string name;
        bool resetCam;
        bool isInit;
        QMutex camMutex;

        ConnexionWidget *myWidget;
        unsigned long object_id, win_id;
        int object_mode;
        bool use_axis[6];
        double sensitivity[6];
        mars::plugins::connexion_plugin::connexionValues *newValues;
        double motion[6];
        /*
         * state[0..2] -> trans: x,y,z
         * state[3..6] -> rot: x,y,z,w (quaternion)
         */
        mars::interfaces::sReal camState[7];

        bool run_thread;
        bool thread_closed;

        void camReset(void);

        void updateCam(mars::interfaces::sReal motion[6]);

        void qFromAxisAndAngle(mars::utils::Quaternion &q,
                               mars::utils::Vector vec,
                               mars::interfaces::sReal angle);

        void printMessage(const char *tmp_message, ...);
        void printWarning(const char *tmp_message, ...);
        void printError(const char *tmp_message, ...);

      protected:
        void run(void);

      protected slots:
        void hideWidget(void);
        void closeWidget(void);
        void objectSelected(unsigned long id);
        void windowSelected(unsigned long id);
        void setObjectMode(int mode);
        void setLockAxis(int axis, bool val);
        void setSensitivity(int axis, double val);
      }; // end of class ConnexionPlugin

    } // end of namespace connexion_plugin
  } // end of namespace plugins
} // end of namespace mars

#endif /* MARS_PLUGINS_CONNEXIONPLUGIN_H */
