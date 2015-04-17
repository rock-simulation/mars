/*
 *  Copyright 2013, DFKI GmbH Robotics Innovation Center
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
 * \file __classname__.h
 * \author __author__ (__email__)
 * \brief __description__
 *
 * Version 0.1
 */

#ifndef __headerDef__MW
#define __headerDef__MW

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

class __classname___MainWin : public QWidget
{
private:
  QLabel *msg;
  QPushButton *btn;
  QGridLayout *lyt;
  
public:
  __classname___MainWin ();
  virtual ~__classname___MainWin ();
};

#endif