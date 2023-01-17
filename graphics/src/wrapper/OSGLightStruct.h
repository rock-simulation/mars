/*
 *  Copyright 2011, DFKI GmbH Robotics Innovation Center
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

/*
 * OSGLightStruct.h
 *
 *  Created on: 19.04.2011
 *      Author: daniel
 */

#ifndef MARS_GRAPHICS_OSGLIGHTSTRUCT_H
#define MARS_GRAPHICS_OSGLIGHTSTRUCT_H

#include <mars_interfaces/LightData.h>

#include <osg/LightSource>
#include <osg/ShapeDrawable>

namespace mars {
  namespace graphics {

    /**
     * Wraps lightStruct in osg::LightSource.
     */
    class OSGLightStruct : public osg::LightSource
    {
    public:
      /**
       * Constructor creates osg::LightSource from lightStruct.
       */
      OSGLightStruct(const mars::interfaces::LightData &ls);
      void update(const mars::interfaces::LightData &ls);
    private:
      osg::ref_ptr<osg::Light> light_;
      osg::ref_ptr<osg::Geode> lightMarkerGeode;
    };

  } // end of namespace graphics
} // end of namespace mars

#endif /* MARS_GRAPHICS_OSGLIGHTSTRUCT_H */
