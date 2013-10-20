/*
 *  Copyright 2011, 2012, 2013, DFKI GmbH Robotics Innovation Center
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
 * \file HUDTexture.h
 * \author Malte Langosz
 * \brief The "HUDTexture" 
 */

#ifndef MARS_GRAPHICS_HUDTEXTURE_H
#define MARS_GRAPHICS_HUDTEXTURE_H

#include "HUDElement.h"

#define TEXTURE_UNKNOWN 0
#define TEXTURE_IMAGE   1
#define TEXTURE_RTT     2

#include <osg/MatrixTransform>

namespace mars {
  namespace graphics {

    class HUDTexture : public HUDElement {

    public:
      HUDTexture(osg::Group *group);
      HUDTexture(void);
      ~HUDTexture(void);
  
      void setSize(double width, double height);
      void setTextureSize(double width, double height);
      void setPos(double x, double y);
      void setBorderColor(double r, double g, double b, double a);
      void setBorderWidth(double border_width);
  
      void createBox(void);
      osg::Group* getNode(void);
      void switchCullMask();
      void xorCullMask(unsigned int mask);
      void setImageData(void *data);
      void setTexture(osg::Texture2D *texture = 0);

    private:
      osg::ref_ptr<osg::Group> parent;
      osg::ref_ptr<osg::MatrixTransform> scaleTransform;
      osg::ref_ptr<osg::PositionAttitudeTransform> posTransform;
      osg::ref_ptr<osg::Image> image;
      osg::ref_ptr<osg::Texture2D> texture;
      osg::ref_ptr<osg::Geode> geode;

      double width, height, t_width, t_height;
      double posx, posy;
      double border_color[4];
      double border_width;
      unsigned int cull_mask;
      bool visible;
      unsigned int texture_type;
    }; // end of class HUDTexture

  } // end of namespace graphics
} // end of namespace mars

#endif /* MARS_GRAPHICS_HUDTEXTURE_H */
