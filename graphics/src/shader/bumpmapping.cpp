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


#include "shader-types.h"
#include "bumpmapping.h"

#include <sstream>

namespace mars {
  namespace graphics {

    using namespace std;
    using mars::interfaces::LightData;

    BumpMapVert::BumpMapVert(vector<string> &args, std::string resPath)
      : ShaderFunc("bump", args) {
      addAttribute( (GLSLAttribute) { "vec4", "vertexTangent" });
      addVarying( (GLSLVarying) { "mat3", "ttw" } );

      resPath += "/shader/normalmap.vert";
      std::ifstream t(resPath.c_str());
      std::stringstream buffer;
      buffer << t.rdbuf();
      source = buffer.str();
    }

    string BumpMapVert::code() const {
      return source;
    }

    /**
     * normal mapping fragment shader.
     * this shader function takes the normal, modifies it and returns it.
     * this technique only changes the normal, advanced techniques may
     * modify texture and vertex coordinates.
     */
    BumpMapFrag::BumpMapFrag(vector<string> &args, std::string resPath)
      : ShaderFunc("bump", args)
    {
      addVarying( (GLSLVarying) { "mat3", "ttw" } );
      addUniform( (GLSLUniform) { "float", "bumpNorFac"} );
      resPath += "/shader/normalmap.frag";
      std::ifstream t(resPath.c_str());
      std::stringstream buffer;
      buffer << t.rdbuf();
      source = buffer.str();
    }

    string BumpMapFrag::code() const {
      return source;
    }

  } // end of namespace graphics
} // end of namespace mars
