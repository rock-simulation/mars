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
 * \file VizPlugin.h
 * \author Malte Langosz
 */

#ifndef VIZ_PLUGIN_H
#define VIZ_PLUGIN_H

#include <mars/interfaces/sim/MarsPluginTemplate.h>
#include <mars/data_broker/ReceiverInterface.h>
#include <mars/osg_text_factory/TextFactoryInterface.h>
#include <mars/interfaces/graphics/GraphicsManagerInterface.h>
#include <mars/viz/Viz.h>

class VizPlugin : public lib_manager::LibInterface,
                  public mars::interfaces::GraphicsUpdateInterface,
                  public mars::data_broker::ReceiverInterface,
                  public mars::cfg_manager::CFGClient {
 
public:
  VizPlugin(lib_manager::LibManager *theManager);
  ~VizPlugin(void);

  // LibInterface methods
  int getLibVersion() const {return 1;}
  const std::string getLibName() const {return std::string("VizPlugin");}

  // DataBroker update
  void receiveData(const mars::data_broker::DataInfo &info,
                   const mars::data_broker::DataPackage &package,
                   int callbackParam);

  // cfg_manager update
  void cfgUpdateProperty(mars::cfg_manager::cfgPropertyStruct _property);

  // graphics update
  void preGraphicsUpdate();

 private:
  mars::interfaces::GraphicsManagerInterface *graphics;
  mars::viz::Viz *viz;
  mars::cfg_manager::CFGManagerInterface *cfg;
  mars::cfg_manager::cfgPropertyStruct vizProp;

  osg_text::TextFactoryInterface *textFactory;
  osg_text::TextInterface *text;
  char cText[255];
  bool updateText;
};

#endif
