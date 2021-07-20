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
 * \file FrameP.hpp
 * \author Malte Langosz
 * \brief 
 **/

#ifndef OSG_FRAME_P_HPP
#define OSG_FRAME_P_HPP

#ifdef _PRINT_HEADER_
#warning "FrameP.hpp"
#endif

#include "Frame.hpp"

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Node>
#include <osg/PositionAttitudeTransform>
#include <osg/MatrixTransform>

namespace osg_frames {

  class FrameP : public Frame {

  public:
    FrameP();
    ~FrameP();

    virtual void setPosition(double x, double y, double z);
    virtual void setRotation(double x, double y, double z, double w);
    virtual void setScale(double x);
    virtual void* getOSGNode();

    static osg::ref_ptr<osg::Node> frameNode;

  private:
    osg::ref_ptr<osg::Geometry> linesGeom;
    osg::ref_ptr<osg::PositionAttitudeTransform> poseTransform;
    osg::ref_ptr<osg::MatrixTransform> scaleTransform;
    osg::ref_ptr<osg::Geode> node;
  };

} // end of namespace: osg_frames

#endif // OSG_FRAME_P_HPP
