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
 * texgen.h
 *
 *  Created on: 27.04.2011
 *      Author: daniel
 */

#ifndef MARS_GRAPHICS_SHADER_TEXGEN_H
#define MARS_GRAPHICS_SHADER_TEXGEN_H

#include "shader-function.h"

#include <vector>
#include <string>

namespace mars {
  namespace graphics {

    /**
     * Eye-linear texture coordinate generation in vertex shader.
     * Using custom shaders disables opengls ffp texgen capability.
     */
    class EyeLinearTexgen : public ShaderFunc {
    public:
      EyeLinearTexgen(std::vector<std::string> &args, unsigned int index);
      std::string code() const;
      unsigned int index;
    }; // end of class ExeLinearTexgen

  } // end of namespace graphics
} // end of namespace mars

#endif /* MARS_GRAPHICS_SHADER_TEXGEN_H */
