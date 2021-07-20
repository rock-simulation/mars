/*
 *  Copyright 2021, DFKI GmbH Robotics Innovation Center
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
 * \file FramesFactory.h
 * \author Malte Langosz
 * \brief 
 **/

#ifndef OSG_FRAMES_FACTORY_HPP
#define OSG_FRAMES_FACTORY_HPP

#ifdef _PRINT_HEADER_
#warning "FramesFactory.hpp"
#endif

#include "Frame.hpp"

namespace osg_frames {

  class FramesP;

  class FramesFactory {

  public:
    FramesFactory();
    ~FramesFactory();

    Frame* createFrame(void);
  };

} // end of namespace: osg_frames

#endif // OSG_FRAMES_FACTORY_H
