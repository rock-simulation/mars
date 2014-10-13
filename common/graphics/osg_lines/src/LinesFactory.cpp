/*
 *  Copyright 2014, DFKI GmbH Robotics Innovation Center
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
 * \file LinesFactory.cpp
 * \author Malte (malte.langosz@dfki.de)
 * \brief A
 *
 * Version 0.1
 */

#include "LinesFactory.h"
#include "LinesP.h"

namespace osg_lines {

  LinesFactory::LinesFactory() {
  }

  LinesFactory::~LinesFactory(void) {
  }

  Lines* LinesFactory::createLines(void) {
    LinesP *lines = new LinesP();
    return lines;
  }

} // end of namespace: osg_lines
