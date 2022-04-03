/**
 * \file AnimationPlugin.hpp
 * \author Malte (malte.langosz@dfki.de)
 * \brief
 *
 * Version 0.1
 */

#ifndef MARS_PLUGINS_ANIMATION_PLUGIN_HPP
#define MARS_PLUGINS_ANIMATION_PLUGIN_HPP

// set define if you want to extend the gui
#include <mars/interfaces/sim/MarsPluginTemplate.h>
#include <mars/interfaces/MARSDefs.h>
#include <mars/osg_material_manager/OsgMaterialManager.h>
#include <osg_animation/Animation.hpp>
#include <mars/interfaces/graphics/GraphicsUpdateInterface.h>
#include <mars/cfg_manager/CFGManagerInterface.h>
//#include <mars/osg_terrain/Terrain.h>

//#include <osg/Group>
#include <string>

namespace mars {

  namespace plugin {
    namespace animation_plugin {

      class AnimationPlugin: public mars::interfaces::MarsPluginTemplate,
                             public interfaces::GraphicsUpdateInterface,
                             public mars::cfg_manager::CFGClient {
      public:
        AnimationPlugin(lib_manager::LibManager *theManager);
        ~AnimationPlugin();

        // LibInterface methods
        int getLibVersion() const
        { return 1; }
        const std::string getLibName() const
        { return std::string("AnimationPlugin"); }
        CREATE_MODULE_INFO();

        // MarsPlugin methods
        void init();
        void reset();
        void update(mars::interfaces::sReal time_ms);

        // graphics update
        virtual void preGraphicsUpdate(void);

        // CFGClient methods
        virtual void cfgUpdateProperty(cfg_manager::cfgPropertyStruct _property);

        // AnimationPlugin methods

      private:
        cfg_manager::cfgPropertyStruct cfgAnimation;
        std::vector<osg_animation::Animation*> animations;
        osg_material_manager::OsgMaterialManager *materialManager;

      }; // end of class definition AnimationPlugin

    } // end of namespace animation_plugin
  } // end of namespace plugin
} // end of namespace mars

#endif // MARS_PLUGINS_ANIMATION_PLUGIN_HPP
