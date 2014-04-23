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
 * \file HUDOSGNode.cpp
 * \author Malte Langosz
 * \brief 
 */

#include <osg/Node>
#include <osg/PositionAttitudeTransform>

#include "HUDOSGNode.h"
#include <stdio.h>

namespace mars {
  namespace graphics {

    HUDOSGNode::HUDOSGNode(osg::ref_ptr<osg::Group> group)
      : HUDElement(),
        parent(group),
        scaleTransform(new osg::MatrixTransform),
        cull_mask(0xffffffff),
        render_order(10),
        visible(true)
    {
      scaleTransform->setMatrix(osg::Matrix::scale(1.0, 1.0, 1.0));
      if(parent.get() != NULL) parent->addChild(scaleTransform);
    }
    HUDOSGNode::HUDOSGNode()
      : HUDElement(),
        parent(NULL),
        scaleTransform(new osg::MatrixTransform),
        cull_mask(0xffffffff),
        render_order(10),
        visible(true)
    {
      scaleTransform->setMatrix(osg::Matrix::scale(1.0, 1.0, 1.0));
    }

    HUDOSGNode::~HUDOSGNode(void) {
    }

    void HUDOSGNode::setPos(double x, double y) {
      posx = x;
      posy = y;
    }

    void HUDOSGNode::setViewSize(double width, double height) {
      view_width = width;
      view_height = height;
    }

    void HUDOSGNode::setOSGNode(osg::Node* node) {
      //fprintf(stderr, "add osg node to hud element\n");
      scaleTransform->addChild(node);
    }

    osg::Group* HUDOSGNode::getNode(void) {
      if(parent.get() == NULL)
        return scaleTransform.get();
      else
        return parent.get();
    }

    void HUDOSGNode::resize(double _width, double _height) {
      double scale_x = _width / view_width;
      double scale_y = _height / view_height;

      scaleTransform->setMatrix(osg::Matrix::scale(scale_x, scale_y, 1.0));
    }

    void HUDOSGNode::switchCullMask() {
      if(visible) {
        visible = false;
        scaleTransform->setNodeMask(0);
      }
      else {
        visible = true;
        scaleTransform->setNodeMask(cull_mask);
      }
    }

    void HUDOSGNode::xorCullMask(unsigned int mask) {
      cull_mask = cull_mask^mask;
      scaleTransform->setNodeMask(cull_mask);
    }

  } // end of namespace graphics
} // end of namespace mars
