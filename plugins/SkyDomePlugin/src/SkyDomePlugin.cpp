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
 * \file SkyDomePlugin.cpp
 * \author Malte (malte.langosz@dfki.de)
 * \brief SkyDom
 *
 * Version 0.1
 */


#include "SkyDomePlugin.h"
#include <mars/data_broker/DataBrokerInterface.h>
#include <mars/data_broker/DataPackage.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>
#include <mars/interfaces/sim/SimulatorInterface.h>
#include <mars/utils/misc.h>
#include <osg/TextureCubeMap>
#include <osg/PositionAttitudeTransform>
#include <osg/Group>
#include <osgUtil/CullVisitor>
#include <osgDB/ReadFile>

namespace mars {
  namespace plugin {
    namespace SkyDomePlugin {

      using namespace mars::utils;
      using namespace mars::interfaces;

      class SkyTransform : public osg::Transform {
      public:
        virtual bool computeLocalToWorldMatrix(osg::Matrix& matrix,
                                               osg::NodeVisitor* nv) const  {
          osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
          if (cv) {
            osg::Vec3 eyePointLocal = cv->getEyeLocal();
            matrix.preMultTranslate(eyePointLocal);
          }
          return true;
        }

        virtual bool computeWorldToLocalMatrix(osg::Matrix& matrix,
                                               osg::NodeVisitor* nv) const {
          osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
          if (cv) {
            osg::Vec3 eyePointLocal = cv->getEyeLocal();
            matrix.postMultTranslate(-eyePointLocal);
          }
          return true;
        }
      };


      SkyDomePlugin::SkyDomePlugin(lib_manager::LibManager *theManager)
        : MarsPluginTemplateGUI(theManager, "SkyDomePlugin") {
      }

      void SkyDomePlugin::init() {
        if(!control->graphics) return;
        posTransform = new SkyTransform();
        posTransform->setCullingActive(false);
        updateProp = true;
        scene = static_cast<osg::Group*>(control->graphics->getScene());

        resPath = ".";
        if(control->cfg) {
          cfgProp = control->cfg->getOrCreateProperty("Graphics",
                                                      "resources_path",
                                                      ".");
          resPath = cfgProp.sValue;
          cfgProp = control->cfg->getOrCreateProperty("Scene",
                                                      "skydome_path",
                                                      "cubemap",
                                                      this);
          folder = cfgProp.sValue;
          cfgEnableSD = control->cfg->getOrCreateProperty("Scene",
                                                          "skydome_enabled",
                                                          false, this);
        }
        else {
          cfgEnableSD.bValue = false;
        }

        if(cfgEnableSD.bValue) {
          scene->addChild(posTransform);
        }

        if(pathExists(resPath+"/Textures/"+folder)) {
          folder = resPath+"/Textures/"+folder;
        }

        // todo: handle wrong path
        osg::ref_ptr<osg::TextureCubeMap> _cubemap = loadCubeMapTextures();

        _skyDome = new SkyDome( 1.9f, 24, 24, _cubemap.get() );
        _skyDome->setCullingActive(false);
        posTransform->addChild(_skyDome.get());

        osg::StateSet *states = _skyDome->getOrCreateStateSet();
        states->setMode(GL_LIGHTING,
                        osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
        //states->setMode(GL_BLEND,osg::StateAttribute::OFF);
        states->setMode(GL_FOG, osg::StateAttribute::OFF);
        states->setRenderBinDetails( -1, "RenderBin");
        states->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
        //states->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        //states->setMode(GL_BLEND,osg::StateAttribute::ON);

        control->sim->switchPluginUpdateMode(0, this);
        gui->addGenericMenuAction("../View/", 0, NULL, 0, "", 0, -1); // separator
        gui->addGenericMenuAction("../View/Sky dome", 1, this, 0, "", 0,
                                  1 + cfgEnableSD.bValue);
      }

      void SkyDomePlugin::reset() {
      }

      SkyDomePlugin::~SkyDomePlugin() {
      }


      osg::ref_ptr<osg::TextureCubeMap> SkyDomePlugin::loadCubeMapTextures() {
        enum {POS_X, NEG_X, POS_Y, NEG_Y, POS_Z, NEG_Z};
        std::string filenames[6];
        filenames[POS_X] = folder + "/east.png";
        filenames[NEG_X] = folder + "/west.png";
        filenames[POS_Z] = folder + "/north.png";
        filenames[NEG_Z] = folder + "/south.png";
        filenames[POS_Y] = folder + "/down.png";
        filenames[NEG_Y] = folder + "/up.png";

        osg::ref_ptr<osg::TextureCubeMap> cubeMap = new osg::TextureCubeMap;
        cubeMap->setInternalFormat(GL_RGBA);

        cubeMap->setFilter( osg::Texture::MIN_FILTER,
                            osg::Texture::LINEAR_MIPMAP_LINEAR);
        cubeMap->setFilter( osg::Texture::MAG_FILTER,
                            osg::Texture::LINEAR);
        cubeMap->setWrap  ( osg::Texture::WRAP_S,
                            osg::Texture::CLAMP_TO_EDGE);
        cubeMap->setWrap  ( osg::Texture::WRAP_T,
                            osg::Texture::CLAMP_TO_EDGE);

        cubeMap->setImage(osg::TextureCubeMap::NEGATIVE_X,
                          osgDB::readImageFile( filenames[NEG_X] ) );
        cubeMap->setImage(osg::TextureCubeMap::POSITIVE_X,
                          osgDB::readImageFile( filenames[POS_X] ) );
        cubeMap->setImage(osg::TextureCubeMap::NEGATIVE_Y,
                          osgDB::readImageFile( filenames[NEG_Y] ) );
        cubeMap->setImage(osg::TextureCubeMap::POSITIVE_Y,
                          osgDB::readImageFile( filenames[POS_Y] ) );
        cubeMap->setImage(osg::TextureCubeMap::NEGATIVE_Z,
                          osgDB::readImageFile( filenames[NEG_Z] ) );
        cubeMap->setImage(osg::TextureCubeMap::POSITIVE_Z,
                          osgDB::readImageFile( filenames[POS_Z] ) );

        return cubeMap;
      }

      void SkyDomePlugin::update(sReal time_ms) {

        // control->motors->setMotorValue(id, value);
      }

      void SkyDomePlugin::receiveData(const data_broker::DataInfo& info,
                                    const data_broker::DataPackage& package,
                                    int id) {
        // package.get("force1/x", force);
      }

      void SkyDomePlugin::cfgUpdateProperty(cfg_manager::cfgPropertyStruct _property) {
        if(cfgEnableSD.paramId == _property.paramId) {
          if(updateProp) {
            cfgEnableSD.bValue = _property.bValue;
            updateProp = false;
            gui->setMenuActionSelected("../View/Sky dome", cfgEnableSD.bValue);
            scene->removeChild(posTransform.get());
            if(cfgEnableSD.bValue) {
              scene->addChild(posTransform.get());
            }
            updateProp = true;
          }
        }
        else if(cfgProp.paramId == _property.paramId) {
          folder = cfgProp.sValue = _property.sValue;
          if(pathExists(resPath + "/Textures/"+folder)) {
            folder = resPath + "/Textures/"+folder;
          }
          if(pathExists(folder)) {
            _skyDome->setCubeMap(loadCubeMapTextures().get());
          }
        }
      }

      void SkyDomePlugin::menuAction (int action, bool checked) {
        if(updateProp) {
          cfgEnableSD.bValue = checked;
          control->cfg->setProperty(cfgEnableSD);
        }
      }

    } // end of namespace SkyDomePlugin
  } // end of namespace plugin
} // end of namespace mars

DESTROY_LIB(mars::plugin::SkyDomePlugin::SkyDomePlugin);
CREATE_LIB(mars::plugin::SkyDomePlugin::SkyDomePlugin);
