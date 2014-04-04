/*
 *  Copyright 2014, DFKI GmbH Robotics Innovation Center
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
 * \file Text3D.h
 * \author Malte (malte.langosz@dfki.de)
 * \brief A
 *
 * Version 0.1
 */

#ifndef MARS_PLUGINS_TEXT3D_H
#define MARS_PLUGINS_TEXT3D_H

#ifdef _PRINT_HEADER_
  #warning "Text3D.h"
#endif

// set define if you want to extend the gui
//#define PLUGIN_WITH_MARS_GUI
#include <mars/interfaces/sim/MarsPluginTemplate.h>
#include <mars/interfaces/MARSDefs.h>
#include <mars/data_broker/ReceiverInterface.h>
#include <mars/cfg_manager/CFGManagerInterface.h>
//#include <mars/interfaces/graphics/GraphicsUpdateInterface.h>

#include <string>

#include <mars/osg_text_factory/TextFactoryInterface.h>

namespace mars {

  namespace lib_manager {
    class LibManager;
  }

  namespace plugins {
    namespace Text3D {

      struct TextData {
        osg_text::TextInterface *text;
        std::string name;
        std::string value;
        unsigned long hudID;
        double posX, posY;
        cfg_manager::cfgParamId vId, pxId, pyId;
      };

      // inherit from MarsPluginTemplateGUI for extending the gui
      class Text3D: public mars::interfaces::MarsPluginTemplate,
                    public mars::data_broker::ReceiverInterface,
                    // for gui
                    //public mars::main_gui::MenuInterface,
                    public mars::cfg_manager::CFGClient {

      public:
        Text3D(mars::lib_manager::LibManager *theManager);
        ~Text3D();

        // LibInterface methods
        int getLibVersion() const
        { return 1; }
        const std::string getLibName() const
        { return std::string("Text3D"); }
        CREATE_MODULE_INFO();

        // MarsPlugin methods
        void init();
        void reset();
        void update(mars::interfaces::sReal time_ms);

        // DataBrokerReceiver methods
        virtual void receiveData(const data_broker::DataInfo &info,
                                 const data_broker::DataPackage &package,
                                 int callbackParam);
        // CFGClient methods
        virtual void cfgUpdateProperty(cfg_manager::cfgPropertyStruct _property);

        //virtual void preGraphicsUpdate(void);

        // Text3D methods

      private:
        cfg_manager::cfgPropertyStruct example;
        osg_text::TextFactoryInterface *textFactory;
        std::map<cfg_manager::cfgParamId, TextData*> textMap;

      }; // end of class definition Text3D

    } // end of namespace Text3D
  } // end of namespace plugins
} // end of namespace mars

#endif // MARS_PLUGINS_TEXT3D_H
