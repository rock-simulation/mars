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
 * \file VizPlugin.cpp
 * \author Malte Langosz
 */

#include "VizPlugin.h"

#include <mars/utils/mathUtils.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

using namespace mars::utils;

VizPlugin::VizPlugin(lib_manager::LibManager *theManager) :
  lib_manager::LibInterface(theManager) {

  cfg = libManager->getLibraryAs<mars::cfg_manager::CFGManagerInterface>("cfg_manager");
  graphics = libManager->getLibraryAs<mars::interfaces::GraphicsManagerInterface>("mars_graphics");

  lib_manager::LibInterface *libInterface = libManager->acquireLibrary("mars_viz");
  if(libInterface){
    viz = static_cast<mars::viz::Viz*>(libInterface);
    //viz = (mars::viz::Viz*)libInterface;
    if(!viz) {
      fprintf(stderr, "have wrong library type\n");
      libManager->releaseLibrary("mars_viz");
    }
  }

  /*
  viz = libManager->getLibraryAs<mars::viz::Viz>("mars_viz");
  if(!viz) {
    fprintf(stderr, "could not load mars_viz!!!!!\n");
  }
  */

  if(cfg) {
    vizProp = cfg->getOrCreateProperty("VizPlugin", "some Value", 1, this);
  }

  if(graphics) {
    textFactory = libManager->getLibraryAs<osg_text::TextFactoryInterface>("osg_text_factory");
    osg_text::Color c, bgColor(0.0, 0.5, 0.0, 0.5);
    text = textFactory->createText("Hello World!", 50,
                                   osg_text::Color(0, .5, 0, 1), 50, 1000,
                                   osg_text::ALIGN_LEFT);

    graphics->addHUDOSGNode(text->getOSGNode());
    graphics->addGraphicsUpdateInterface(this);
  }
  updateText = true;
  if(viz) {
    LOG_INFO("laod scene");
    viz->loadScene("../../share/mars/plugins/viz_plugin/easy.scn");
  }
}

VizPlugin::~VizPlugin(void) {
  libManager->releaseLibrary("cfg_manager");
  libManager->releaseLibrary("mars_graphics");
}

void VizPlugin::preGraphicsUpdate() {
  static double foo = 0.0;
  foo += 0.01;
  foo = fmod(foo, 6.28);
  viz->setJointValue("leg0_joint0", sin(foo));
  viz->setJointValue("leg0_joint1", -2*sin(foo));
  if(graphics) {
    if(updateText) {
      updateText= false;
      sprintf(cText,
              "Hello World: %d\n", vizProp.iValue);
      text->setText(cText);
    }
  }
}

// here we get the position and orientation of the light sensors
void VizPlugin::receiveData(const mars::data_broker::DataInfo &info,
        const mars::data_broker::DataPackage &package,
        int callbackParam) {
  (void) info;
  (void) package;
  (void) callbackParam;
}

void VizPlugin::cfgUpdateProperty(mars::cfg_manager::cfgPropertyStruct _property) {
  if(vizProp.paramId == _property.paramId) {
    vizProp.iValue = _property.iValue;
    updateText = true;
  }
}


CREATE_LIB(VizPlugin);
DESTROY_LIB(VizPlugin);
