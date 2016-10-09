/*
 *  Copyright 2016, DFKI GmbH Robotics Innovation Center
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
 * \file VirtualJoystickPlugin.h
 * \author Michael Rohn
 * \brief A Plugin to map a virtual JoyStick to motors in the sim
 */

#ifndef VIRTUAL_JOYSTICK_PLUGIN_H
#define VIRTUAL_JOYSTICK_PLUGIN_H

#include <mars/main_gui/GuiInterface.h>
#include <mars/main_gui/MenuInterface.h>
#include <mars/lib_manager/LibManager.h>
#include <mars/data_broker/DataBrokerInterface.h>
#include <mars/cfg_manager/CFGManagerInterface.h>

#include <QObject>
#include "JoystickWidget.h"
//#include "JoystickInterface.h"

class VirtualJoystickPlugin :
  public QObject, public lib_manager::LibInterface,
  public mars::main_gui::MenuInterface {
  
Q_OBJECT
	
 public:
  VirtualJoystickPlugin(lib_manager::LibManager *theManager);
  ~VirtualJoystickPlugin(void);

  // LibInterface methods
  int getLibVersion() const {return 1;}
  const std::string getLibName() const {return std::string("VirtualJoystick");}

  void getNewValue(double* leftValue, double* rightValue);

  virtual void menuAction(int action, bool checked = false);

 private slots:

  void newSpeed(double leftValue, double rightValue);
  void hideWidget(void);
  void closeWidget(void);


 private:
  mars::main_gui::GuiInterface* gui;
  JoystickWidget *joystick;
  double leftValue, rightValue;
  mars::data_broker::DataBrokerInterface *dataBroker;
  mars::data_broker::DataPackage dataPackage;
  mars::cfg_manager::CFGManagerInterface *cfg;
  unsigned long dataPackageId;
};

#endif // VIRTUAL_JOYSTICK_PLUGIN_H

