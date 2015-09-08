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
#include <osg/TextureCubeMap>
#include <osg/PositionAttitudeTransform>
#include <osg/Group>
#include <osgDB/ReadFile>

namespace mars {
  namespace plugin {
    namespace SkyDomePlugin {

      using namespace mars::utils;
      using namespace mars::interfaces;

      SkyDomePlugin::SkyDomePlugin(lib_manager::LibManager *theManager)
        : MarsPluginTemplate(theManager, "SkyDomePlugin") {
      }
  
      void SkyDomePlugin::init() {
        if(!control->graphics) return;
        posTransform = new osg::PositionAttitudeTransform();
        posTransform->setPosition(osg::Vec3(0.0, 0.0, 0.0));

        scene = static_cast<osg::Group*>(control->graphics->getScene());

        scene->addChild(posTransform);

        osg::ref_ptr<osg::TextureCubeMap> _cubemap = loadCubeMapTextures();
        _skyDome = new SkyDome( 1.9f, 24, 24, _cubemap.get() );
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

        control->graphics->addGraphicsUpdateInterface(this);
        control->sim->switchPluginUpdateMode(0, this);
      }

      void SkyDomePlugin::reset() {
      }

      SkyDomePlugin::~SkyDomePlugin() {
      }


      osg::ref_ptr<osg::TextureCubeMap> SkyDomePlugin::loadCubeMapTextures() {
        enum {POS_X, NEG_X, POS_Y, NEG_Y, POS_Z, NEG_Z};
        std::string path = ".";
        std::string filenames[6];

        if(control->cfg) {
          cfgProp = control->cfg->getOrCreateProperty("Graphics",
                                                      "resources_path",
                                                      ".");
          path = cfgProp.sValue;
        }

        filenames[POS_X] = path + "/Textures/cubemap/east.png";
        filenames[NEG_X] = path + "/Textures/cubemap/west.png";
        filenames[POS_Z] = path + "/Textures/cubemap/north.png";
        filenames[NEG_Z] = path + "/Textures/cubemap/south.png";
        filenames[POS_Y] = path + "/Textures/cubemap/down.png";
        filenames[NEG_Y] = path + "/Textures/cubemap/up.png";
        
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

      void SkyDomePlugin::preGraphicsUpdate() {
        interfaces::GraphicsCameraInterface *cam;
        cam = control->graphics->get3DWindow(1)->getCameraInterface();
        double tx, ty, tz, rx, ry, rz, rw;
        cam->getViewportQuat(&tx, &ty, &tz, &rx, &ry, &rz, &rw);
        posTransform->setPosition(osg::Vec3(tx, ty, tz));
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

      }

    } // end of namespace SkyDomePlugin
  } // end of namespace plugin
} // end of namespace mars

DESTROY_LIB(mars::plugin::SkyDomePlugin::SkyDomePlugin);
CREATE_LIB(mars::plugin::SkyDomePlugin::SkyDomePlugin);
