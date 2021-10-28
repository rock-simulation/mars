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
 * \file Frame.cpp
 * \author Malte (malte.langosz@dfki.de)
 * \brief A
 *
 * Version 0.1
 */

#include "FrameP.hpp"

namespace osg_frames {

  osg::ref_ptr<osg::Node> FrameP::frameNode = NULL;

  
  FrameP::FrameP() {

    poseTransform = new osg::PositionAttitudeTransform;
    scaleTransform = new osg::MatrixTransform;
    scaleTransform->addChild(FrameP::frameNode);
    poseTransform->addChild(scaleTransform);
  }

  FrameP::~FrameP(void) {
  }

  void FrameP::setPosition(double x, double y, double z) {
    poseTransform->setPosition(osg::Vec3(x, y, z));
  }

  void FrameP::setRotation(double x, double y, double z, double w) {
    poseTransform->setAttitude(osg::Quat(x, y, z, w));
  }

  void FrameP::setScale(double x) {
    scaleTransform->setMatrix(osg::Matrix::scale(x, x, x));
  }
  
  void* FrameP::getOSGNode() {
    return (void*)(osg::PositionAttitudeTransform*)poseTransform.get();
  }

} // end of namespace: osg_frames
