/*
 *  Copyright 2015, DFKI GmbH Robotics Innovation Center
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
 * \file SkyDomePlugin.h
 * \author Malte (malte.langosz@dfki.de)
 * \brief SkyDom
 *
 * Version 0.1
 */

#ifndef MARS_PLUGINS_SKYDOMPLUGIN_H
#define MARS_PLUGINS_SKYDOMPLUGIN_H

#ifdef _PRINT_HEADER_
  #warning "SkyDomePlugin.h"
#endif

// set define if you want to extend the gui
#include <mars_interfaces/sim/MarsPluginTemplateGUI.h>
#include <mars_interfaces/MARSDefs.h>
#include <data_broker/ReceiverInterface.h>
#include <cfg_manager/CFGManagerInterface.h>
#include <osg_material_manager/OsgMaterialManager.h>
#include <osg_material_manager/MaterialNode.h>

#include <string>

#include "SkyDome.h"

namespace mars {

  namespace plugin {
    namespace SkyDomePlugin {

      class SkyTransform : public osg::Transform {
      public:
        SkyTransform(): s1(1.0), s2(1.0) {}
        virtual bool computeLocalToWorldMatrix(osg::Matrix& matrix,
                                               osg::NodeVisitor* nv) const;

        virtual bool computeWorldToLocalMatrix(osg::Matrix& matrix,
                                               osg::NodeVisitor* nv) const;
        double s1, s2;
      };

      // inherit from MarsPluginTemplateGUI for extending the gui
      class SkyDomePlugin: public mars::interfaces::MarsPluginTemplateGUI,
                           public mars::data_broker::ReceiverInterface,
                           // for gui
                           public mars::main_gui::MenuInterface,
                           public mars::cfg_manager::CFGClient {

      public:
        SkyDomePlugin(lib_manager::LibManager *theManager);
        ~SkyDomePlugin();

        // LibInterface methods
        int getLibVersion() const
        { return 1; }
        const std::string getLibName() const
        { return std::string("SkyDomePlugin"); }
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

        // MenuInterface methods
        void menuAction(int action, bool checked = false);

        // SkyDomePlugin methods

      private:
        cfg_manager::cfgPropertyStruct cfgProp, cfgPropPath, cfgEnableSD;

        osg::ref_ptr<SkyDome> _skyDome;
        osg::ref_ptr<osg::Group> scene;
        osg::ref_ptr<osg_material_manager::MaterialNode> materialGroup;
        osg::ref_ptr<SkyTransform> posTransform, meshPos;
        std::string resPath, folder, meshPath, meshMaterialName;
        double meshScale;
        bool updateProp;
        osg_material_manager::OsgMaterialManager *materialManager;

        osg::ref_ptr<osg::TextureCubeMap> loadCubeMapTextures();
      }; // end of class definition SkyDomePlugin

    } // end of namespace SkyDomePlugin
  } // end of namespace plugin
} // end of namespace mars

#endif // MARS_PLUGINS_SKYDOMPLUGIN_H
