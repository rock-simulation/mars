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

/*
 * OSGLightStruct.cpp
 *
 *  Created on: 19.04.2011
 *      Author: daniel
 */

#include "OSGLightStruct.h"
#include "gui_helper_functions.h"

#define CREATE_LIGHT_MARKER

namespace mars {
  namespace graphics {

    using mars::interfaces::LightData;

    static osg::ref_ptr<osg::Geode> createLightMarker(const LightData &ls) {
#ifdef CREATE_LIGHT_MARKER
      osg::ref_ptr<osg::Geode> lightMarkerGeode (new osg::Geode);
      osg::ShapeDrawable *shape = new osg::ShapeDrawable(new osg::Sphere(
                                                                         osg::Vec3f(ls.pos.x(), ls.pos.y(), ls.pos.z()), 0.07f));
      shape->setColor(osg::Vec4f(1.0f, 0.95f, 0.35f, 1.0f));
      lightMarkerGeode->addDrawable(shape);
      return lightMarkerGeode;
#else
      return NULL;
#endif
    }

    OSGLightStruct::OSGLightStruct(const LightData &ls)
      : osg::LightSource()
    {
      light_ = getLight();
      light_->setLightNum(ls.index);

      //set colors
      light_->setAmbient(toOSGVec4(ls.ambient));
      light_->setDiffuse(toOSGVec4(ls.diffuse));
      light_->setSpecular(toOSGVec4(ls.specular));

      //set spotlight parameters
      if (ls.type == mars::interfaces::SPOTLIGHT) {
        osg::Vec3 pos = osg::Vec3(ls.pos.x(), ls.pos.y(), ls.pos.z());
        osg::Vec3 lookPos = osg::Vec3(ls.lookAt.x(), ls.lookAt.y(),
                                      ls.lookAt.z());

        light_->setDirection(lookPos);
        light_->setSpotCutoff(ls.angle*3.14/180);
        light_->setSpotExponent(ls.exponent);

      } else {
        light_->setSpotCutoff(180.0);
      }
      //set light position
      if(ls.directional) {
        light_->setPosition(osg::Vec4(ls.pos.x(), ls.pos.y(), ls.pos.z(), 0.0f));
      } else {
        light_->setPosition(osg::Vec4(ls.pos.x(), ls.pos.y(), ls.pos.z(), 1.0f));
      }
      //set attenuation parameters
      if (ls.constantAttenuation>0) {
        light_->setConstantAttenuation(ls.constantAttenuation);
      }
      if (ls.linearAttenuation>0) {
        light_->setLinearAttenuation(ls.linearAttenuation);
      }
      if (ls.quadraticAttenuation>0) {
        light_->setQuadraticAttenuation(ls.quadraticAttenuation);
      }

      lightMarkerGeode = createLightMarker(ls);

      //addChild(lightMarkerGeode.get());
      setLight(light_.get());
    }

    void OSGLightStruct::update(const LightData &ls) {
      //set light position
      if(ls.directional) {
        light_->setPosition(toOSGVec4(ls.pos,0.0f));
      }
      else {
        light_->setPosition(toOSGVec4(ls.pos,1.0f));
      }
      if(lightMarkerGeode.get()) {
        removeChild(lightMarkerGeode.get());
        lightMarkerGeode = createLightMarker(ls);
        addChild(lightMarkerGeode.get());
      }
      //set colors
      light_->setAmbient(toOSGVec4(ls.ambient));
      light_->setDiffuse(toOSGVec4(ls.diffuse));
      light_->setSpecular(toOSGVec4(ls.specular));

      //set spotlight parameters
      if (ls.type == mars::interfaces::SPOTLIGHT){
        osg::Vec3 lookPos = osg::Vec3(ls.lookAt.x(), ls.lookAt.y(), ls.lookAt.z());

        light_->setDirection(lookPos);
        light_->setSpotCutoff(ls.angle*3.14/180.);
        light_->setSpotExponent(ls.exponent);
      }
      //if no spotlight, set standard values for Omnilight
      else {
        light_->setSpotCutoff(180.0f);
        light_->setSpotExponent(0.0f);
      }

      //set light attenuation. If no Attenuation selected set standard values
      if (ls.constantAttenuation>0){
        light_->setConstantAttenuation(
                                       ls.constantAttenuation);
      } else {
        light_->setConstantAttenuation(1.0f);
      }

      if (ls.linearAttenuation>0){
        light_->setLinearAttenuation(
                                     ls.linearAttenuation);
      } else {
        light_->setLinearAttenuation(0.0f);
      }

      if (ls.quadraticAttenuation>0){
        light_->setQuadraticAttenuation(
                                        ls.quadraticAttenuation);
      } else {
        light_->setQuadraticAttenuation(0.0f);
      }
    }

  } // end of namespace graphics
} // end of namespace mars
