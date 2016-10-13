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
 * \file JoystickInterface.h
 * \author Michael Rohn
 * \brief "JoystcikInterface" is an interface to a plugin to get faked joystick values
 *
 * Version 0.1 (23.01.09): 
 *               - 
 */

#ifndef JOYSTICK_INTERFACE_H
#define JOYSTICK_INTERFACE_H


//#include "sim_common.h"

class JoystickInterface {
 public:
  JoystickInterface(void) {};
  virtual ~JoystickInterface(void) {};
  virtual void getNewValue(double* leftValue, double* rightValue) = 0;
};

#endif // JOYSTICK_INTERFACE_H
