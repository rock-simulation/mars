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
 * \file FramesFactory.cpp
 * \author Malte (malte.langosz@dfki.de)
 * \brief A
 *
 * Version 0.1
 */

#include "FramesFactory.hpp"
#include "FrameP.hpp"

#include <osgDB/ReadFile>

namespace osg_frames {

  FramesFactory::FramesFactory() {

    // load frames resource
    std::stringstream ss;
    ss << SHARE_DIR;
    ss << "/resources/frame.obj";
    fprintf(stderr, "osg_frames: load %s\n", ss.str().c_str());
    FrameP::frameNode = osgDB::readNodeFile(ss.str());
  }

  FramesFactory::~FramesFactory(void) {
  }

  Frame* FramesFactory::createFrame(void) {
    FrameP *frame = new FrameP();
    return frame;
  }

} // end of namespace: osg_frames
