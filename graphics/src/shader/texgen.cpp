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
 * texgen.cpp
 *
 *  Created on: 27.04.2011
 *      Author: daniel
 */

#include "texgen.h"

#include <sstream>

namespace mars {
  namespace graphics {

    EyeLinearTexgen::EyeLinearTexgen(std::vector<std::string> &args,
                                     unsigned int index)
      : ShaderFunc("demoTexgen", args) {
      this->index = index;
    }

    std::string EyeLinearTexgen::code() const {
      std::stringstream s;
      s << "void " << name << "() {" << std::endl;
      s << "  vec4 ev = gl_ModelViewMatrix * gl_Vertex;" << std::endl;
      s << "  gl_TexCoord["<<index<<"] = ev* mat4("
        " gl_EyePlaneS["<<index<<"]," <<
        " gl_EyePlaneT["<<index<<"]," <<
        " gl_EyePlaneR["<<index<<"]," <<
        " gl_EyePlaneQ["<<index<<"]);" << std::endl;
      s << "  //gl_TexCoord["<<index<<"] *= gl_TextureMatrix["<<index<<"];" << std::endl;
      s << "}" << std::endl;
      return s.str();
    }

  } // end of namespace graphics
} // end of namespace mars
