/*
 *  Copyright 2011, 2012, DFKI GmbH Robotics Innovation Center
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
 * Clouds.h
 *
 *  Created on: 19.04.2011
 *      Author: daniel
 */

#ifndef MARS_GRAPHICS_CLOUDS_H
#define MARS_GRAPHICS_CLOUDS_H

#include <string>
#include <osg/Group>
#include <osg/TexMat>

namespace mars {
  namespace graphics {

    class Clouds : public osg::Group {
    public:
      Clouds(const std::string &texture_path);
    protected:
      osg::ref_ptr<osg::TexMat> texmat;
    }; // end of class Clouds

  } // end of namespace graphics
} // end of namespace mars

#endif /* MARS_GRAPHICS_CLOUDS_H */
