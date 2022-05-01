/**
 * \file AnimationPlugin.cpp
 * \author Malte (malte.langosz@dfki.de)
 * \brief
 *
 * Version 0.1
 */


#include "AnimationPlugin.hpp"
//#include "Octree.hpp"

#include <mars/interfaces/graphics/GraphicsManagerInterface.h>
#include <mars/interfaces/sim/NodeManagerInterface.h>
#include <mars/utils/misc.h>
#include <osg/Texture2D>
#include <osgDB/WriteFile>

#include <osg_animation/AnimationFactory.hpp>

namespace mars {
  namespace plugin {
    namespace animation_plugin {

      using namespace mars::utils;
      using namespace mars::interfaces;

      AnimationPlugin::AnimationPlugin(lib_manager::LibManager *theManager)
        : MarsPluginTemplate(theManager, "AnimationPlugin"),
          materialManager(NULL) {
      }

      void AnimationPlugin::init() {

        if(!control->graphics) return;
        // scene = static_cast<osg::Group*>(control->graphics->getScene2());

        materialManager = libManager->getLibraryAs<osg_material_manager::OsgMaterialManager>("osg_material_manager", true);
        if(!materialManager) return;

        control->sim->switchPluginUpdateMode(PLUGIN_GUI_MODE, this);

        // 1. load instances: is done by loading scene
        std::string configFile;
        configmaps::ConfigMap map;

        try {
          // get config path
          configPath = control->cfg->getOrCreateProperty("Config", "config_path", ".").sValue;
          // the animations have to be linked with the scene loading / entity management
          std::string animationsPath = "";
          control->cfg->getPropertyValue("Scene", "animations_path", "value",
                                         &animationsPath);
          if(animationsPath != "") {
            char *envText = getenv("AUTOPROJ_CURRENT_ROOT");
            animationsPath = replaceString(animationsPath, "$AUTOPROJ_CURRENT_ROOT", envText);
            configPath = animationsPath;
          }

          configFile = utils::pathJoin(configPath, "animation_plugin.yml");
          LOG_INFO("load %s", configFile.c_str());
          map = configmaps::ConfigMap::fromYamlFile(configFile);
        } catch (...) {
          LOG_ERROR("no animation_plugin.yml found but animation_plugin plugin loaded! (%s)", configFile.c_str());
        }
        // 2. load positions into octree
        if(map.hasKey("animations")) {
          for(auto it: map["animations"]) {
            // information to load animations
            std::string material_name = it["material"];
            std::string texture_name = it["matrix_texture"];
            int pixel_offset = 0;
            if(it.hasKey("pixel_offset")) {
              pixel_offset = it["pixel_offset"];
            }
            std::string bones_config = it["bones_config_file"];
            osg_animation::AnimationFactory af;
            osg_animation::Animation *animation = af.createAnimation();
            animation->setGraphics(control->graphics);
            animation->setMaterialManager(materialManager);
            animation->setMatrixTexture(material_name, texture_name);
            animation->setLoadPath(configPath);
            animation->loadBonesFile(bones_config);
            animation->setName(it["name"]);
            //animation->printBones();
            if(it.hasKey("animations")) {
              for(auto it2: (configmaps::ConfigMap)it["animations"]) {
                animation->loadAnimation(it2.first, it2.second);
              }
            }
            animations.push_back(animation);
          }
        }
        control->graphics->addGraphicsUpdateInterface(this);
        if(control->cfg) {
          cfgAnimation = control->cfg->getOrCreateProperty("AnimationPlugin",
                                                           "animation",
                                                           "", this);
        }
        AnimationPlugin::reset();
      }

      void AnimationPlugin::reset()
      {
      }

      AnimationPlugin::~AnimationPlugin() {
        for(auto it: animations) {
          delete it;
        }
        if(materialManager) libManager->releaseLibrary("osg_material_manager");
      }


      void AnimationPlugin::update(sReal time_ms) {
      }

      void AnimationPlugin::preGraphicsUpdate() {
        for(auto it: animations) {
          it->updatePose();
        }
      }

      void AnimationPlugin::cfgUpdateProperty(cfg_manager::cfgPropertyStruct _property) {
        if(cfgAnimation.paramId == _property.paramId) {
          std::vector<std::string> cmd = utils::explodeString(':', _property.sValue);
          if(cmd.size() >= 2) {
            for(auto it: animations) {
              if(it->getName() == cmd[0]) {
                int repeat = -1;
                if(cmd.size() == 3) {
                  repeat = atoi(cmd[2].c_str());
                }
                it->playAnimation(cmd[1], repeat);
                break;
              }
            }
          }
        }
      }

    } // end of namespace animation_plugin
  } // end of namespace plugin
} // end of namespace mars

DESTROY_LIB(mars::plugin::animation_plugin::AnimationPlugin);
CREATE_LIB(mars::plugin::animation_plugin::AnimationPlugin);
