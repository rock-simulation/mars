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

    /**
     * normal mapping fragment shader.
     * this shader function takes the normal, modifies it and returns it.
     * this technique only changes the normal, advanced techniques may
     * modify texture and vertex coordinates.
     */
    BumpMapFrag::BumpMapFrag(vector<string> &args)
      : ShaderFunc("bump", args)
    {
      addUniform( (GLSLUniform) { "float", "bumpNorFac"} );
    }

    string BumpMapFrag::code() const
    {
      stringstream s;
      s << "/**" << endl;
      s << " * this method returns a per fragment normal from a normal map." << endl;
      s << " * the normal exists in tangent space." << endl;
      s << " **/" << endl;
      s << "void " << name << "(vec4 texel, vec3 n, out vec3 normal)" << endl;
      s << "{" << endl;
      //s << "    normal = vec3( 0.0, 0.0, 1.0 );" << endl;
      s << "    normal = n+(normalize( osg_ViewMatrixInverse * vec4(gl_NormalMatrix * ( texel.xyz * 2.0 - 1.0 ), 0.0)).xyz - n)*bumpNorFac;" << endl;
      s << "    normal = normalize(normal);" << endl;
      s << "}" << endl;
      return s.str();
    }

    BumpMapVert::BumpMapVert(vector<string> &args,
                             int numLights)
      : ShaderFunc("bump", args), numLights(numLights)
    {
      stringstream s;

      s << "lightVec[" << numLights << "]";
      addVarying( (GLSLVarying) { "vec3", s.str() });
      s.str("");

      s << "spotDir[" << numLights << "]";
      addVarying( (GLSLVarying) { "vec3", s.str() });
      s.str("");

      addAttribute( (GLSLAttribute) { "vec4", "vertexTangent" });
    }

    string BumpMapVert::code() const
    {
      stringstream s;

      s << "/**" << endl;
      s << " * this method transforms the varyings eyeVec and lightVec to tangent space." << endl;
      s << " * the normal map must contain tangents in tangent space, this value is used" << endl;
      s << " * directly in the fragment shader. the varyings are transformed because" << endl;
      s << " * the vectors must be in the same space for the light calculation to work." << endl;
      s << " * the normal in the fragment sahder can also be transformed to eyespace instead of this," << endl;
      s << " * but this would need more calculations, then doing it in the vertex shader." << endl;
      s << " * @param n: the normal attribute at the processed vertex in eye space." << endl;
      s << " **/" << endl;
      s << "" << endl;
      s << "void " << name << "(vec3 n)" << endl;
      s << "{" << endl;
      s << "    // get the tangent in eye space (multiplication by gl_NormalMatrix transforms to eye space)" << endl;
      s << "    // the tangent should point in positive u direction on the uv plane in the tangent space." << endl;
      s << "    return; vec3 t = normalize( (osg_ViewMatrixInverse*vec4(gl_NormalMatrix * vertexTangent.xyz, 0.0)).xyz );" << endl;
      s << "    // calculate the binormal, cross makes sure tbn matrix is orthogonal" << endl;
      s << "    // multiplicated by handeness." << endl;
      s << "    vec3 b = cross(n, t);" << endl;
      s << "    // transpose tbn matrix will do the transformation to tangent space" << endl;
      s << "    mat3 tbn = transpose( mat3(t, b, n) );" << endl;
      //s << "    vec3 buf;" << endl;
      s << "" << endl;
      s << "    // do the transformation of the eye vector (used for specuar light)" << endl;
      //s << "    buf.x = dot( eyeVec.xyz, t );" << endl;
      //s << "    buf.y = dot( eyeVec.xyz, b );" << endl;
      //s << "    buf.z = dot( eyeVec.xyz, n );" << endl;
      //s << "    eyeVec = normalize( vec4(buf,eyeVec.w) );" << endl;
      s << "    eyeVec = tbn * eyeVec;" << endl;
      s << "" << endl;
      s << "    // do the transformation of the light vectors" << endl;

      s << " for(int i=0; i<"<< numLights << "; ++i) {" << endl;
        /*
        s << "        buf.x = dot( lightVec[" << lightIndex << "], t );" << endl;
        s << "        buf.y = dot( lightVec[" << lightIndex << "], b );" << endl;
        s << "        buf.z = dot( lightVec[" << lightIndex << "], n ) ;" << endl;
        s << "        lightVec[" << lightIndex << "] = normalize( buf  );" << endl;
        */
        s << "    lightVec[i] = tbn*lightVec[i];" << endl;

        /*
        s << "        buf.x = dot( spotDir[" << lightIndex << "], t );" << endl;
        s << "        buf.y = dot( spotDir[" << lightIndex << "], b );" << endl;
        s << "        buf.z = dot( spotDir[" << lightIndex << "], n ) ;" << endl;
        s << "        spotDir[" << lightIndex << "] = normalize( buf  );" << endl;
        */
        s << "    spotDir[i] = tbn*spotDir[i];" << endl;
      s << "}" << endl;
      s << "}" << endl;

      return s.str();
    }

  } // end of namespace graphics
} // end of namespace mars
