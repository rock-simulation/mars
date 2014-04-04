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
 * \file HUDOSGNode.h
 * \author Malte Langosz
 * \brief The "HUDOSGNode" 
 */

#ifndef MARS_GRAPHICS_HUDOSGNODE_H
#define MARS_GRAPHICS_HUDOSGNODE_H

#include "HUDElement.h"

#include <osg/MatrixTransform>
#include <string>
#include <vector>

namespace mars {
  namespace graphics {

    class HUDOSGNode : public HUDElement {

    public:
      HUDOSGNode(osg::ref_ptr<osg::Group> group);
      HUDOSGNode(void);
      ~HUDOSGNode(void);
  
      void setPos(double x, double y);
      void setViewSize(double widht, double height);
      void setOSGNode(osg::Node* node);
      void resize(double _width, double _height);
      void setRenderOrder(int val) {render_order = val;}
      osg::Group* getNode(void);
      void switchCullMask(void);
      void xorCullMask(unsigned int mask);
  
    private:
      osg::ref_ptr<osg::Group> parent;
      osg::ref_ptr<osg::MatrixTransform> scaleTransform;

      double view_width, view_height;
      unsigned int cull_mask;
      int render_order;
      bool visible, init;
      double posx, posy;
    }; // end of class HUDOSGNode

  } // end of namespace graphics
} // end of namespace mars

#endif /* MARS_GRAPHICS_HUDOSGNODE_H */
