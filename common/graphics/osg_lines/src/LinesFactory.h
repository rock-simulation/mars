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
 * \file LinesFactory.h
 * \author Malte Langosz
 * \brief 
 **/

#ifndef OSG_LINES_FACTORY_H
#define OSG_LINES_FACTORY_H

#ifdef _PRINT_HEADER_
#warning "LinesFactory.h"
#endif

#include "Lines.h"

namespace osg_lines {

  class LinesP;

  class LinesFactory {

  public:
    LinesFactory();
    ~LinesFactory();

    Lines* createLines(void);

  };

} // end of namespace: osg_lines

#endif // OSG_LINES_FACTORY_H
