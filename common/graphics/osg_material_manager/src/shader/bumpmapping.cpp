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
#include <fstream>

namespace osg_material_manager {

  using namespace std;

  BumpMapVert::BumpMapVert(vector<string> &args, std::string resPath)
    : ShaderFunc("bump", args) {
    funcs[0].second.push_back("n.xyz");
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
    : ShaderFunc("bump", args) {
    funcs[0].second.push_back("nt");
    funcs[0].second.push_back("n");
    funcs[0].second.push_back("n");
    addVarying( (GLSLVarying) { "mat3", "ttw" } );
    addUniform( (GLSLUniform) { "float", "bumpNorFac"} );
    resPath += "/shader/normalmap.frag";
    std::ifstream t(resPath.c_str());
    std::stringstream buffer;
    buffer << t.rdbuf();
    source = buffer.str();
    addUniform( (GLSLUniform) { "sampler2D", "normalMap" } );
    addMainVar( (GLSLVariable) { "vec4", "nt", "texture2D( normalMap, texCoord )" });
  }

  string BumpMapFrag::code() const {
    return source;
  }

} // end of namespace osg_material_manager
