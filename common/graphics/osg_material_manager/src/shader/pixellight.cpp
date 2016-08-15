/*
 *  Copyright 2011, 2012, 2016, DFKI GmbH Robotics Innovation Center
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


#include "pixellight.h"

#include <sstream>
#include <iostream>
#include <fstream>

namespace osg_material_manager {

  using namespace std;

#ifndef max
#define max(x,y) (x>y ? x : y)
#endif

#define NUM_PSSM_SPLITS 3
  
  PixelLightVert::PixelLightVert(vector<string> &args, int numLights,
                                 std::string resPath)
    : ShaderFunc("plight", args) {
    std::stringstream s;
    funcs[0].second.push_back("vWorldPos");
    funcs[0].second.push_back("specularCol");
    s << "[" << numLights << "]";

    addVarying( (GLSLVarying) { "vec3", "lightVec" + s.str() } );
    addVarying( (GLSLVarying) { "vec3", "spotDir" + s.str() } );
    addVarying( (GLSLVarying) { "vec4", "diffuse" + s.str() } );
    addVarying( (GLSLVarying) { "vec4", "specular" + s.str() } );
    addVarying( (GLSLVarying) { "vec3", "eyeVec" } );

    addUniform( (GLSLUniform) { "vec3", "lightPos" + s.str() } );
    addUniform( (GLSLUniform) { "vec3", "lightSpotDir" + s.str() } );
    addUniform( (GLSLUniform) { "vec4", "lightDiffuse" + s.str() } );
    addUniform( (GLSLUniform) { "vec4", "lightSpecular" + s.str() } );
    addUniform( (GLSLUniform) { "int", "lightIsDirectional" + s.str() } );
    addUniform( (GLSLUniform) { "int", "lightIsSet" + s.str() } );
    addUniform( (GLSLUniform) { "mat4", "osg_ViewMatrixInverse" } );
    addUniform( (GLSLUniform) { "mat4", "osg_ViewMatrix" } );

    addUniform( (GLSLUniform) { "int", "useShadow" } );
    addUniform( (GLSLUniform) { "int", "numLights" } );
    resPath += "/shader/plight.vert";
    std::ifstream t(resPath.c_str());
    std::stringstream buffer;
    buffer << t.rdbuf();
    source = buffer.str();
    addMainVar( (GLSLVariable) { "vec4", "n", "normalize(osg_ViewMatrixInverse * vec4(gl_NormalMatrix * gl_Normal, 0.0))" } );

    addExport( (GLSLExport) { "normalVarying", "n.xyz" } );

    addExport( (GLSLExport) { "positionVarying", "vWorldPos" } );
    addExport( (GLSLExport) { "modelVertex", "vModelPos" } );
    addVarying( (GLSLVarying) { "vec4", "positionVarying" } );
    addVarying( (GLSLVarying) { "vec3", "normalVarying" } );
    addVarying( (GLSLVarying) { "vec4", "modelVertex" } );

  }

  string PixelLightVert::code() const {
    return source;
  }

  static std::string pssmAmount(double textureRes=2028,
                                bool filtered=false,
                                unsigned int textureOffset=1) {
    std::stringstream sstr;

    sstr << "float pssmAmount() {" << endl;
    sstr << "    float testZ = gl_FragCoord.z*2.0-1.0;" <<std::endl;
    sstr << "    float map0 = step(testZ, zShadow0);"<< std::endl;
    for (unsigned int i=1;i<NUM_PSSM_SPLITS;i++)    {
      sstr << "    float map" << i << "  = step(zShadow"<<i-1<<",testZ)*step(testZ, zShadow"<<i<<");"<< std::endl;
    }
    if (filtered) {
      sstr << "      float fTexelSize="<< (1.41 / textureRes ) <<";" << std::endl;
      sstr << "      float fZOffSet  = -0.001954;" << std::endl; // 2^-9 good value for ATI / NVidia
    }
    for (unsigned int i=0;i<NUM_PSSM_SPLITS;i++)    {
      if (!filtered) {
        sstr << "    float shadow" << i <<" = step(0.25,shadow2DProj( shadowTexture" << i <<
          ",gl_TexCoord[" << (i+textureOffset) <<"]).r);"   << std::endl;
      } else {
        sstr << "    float shadowOrg" << i <<" = shadow2DProj( shadowTexture" <<
          i <<",gl_TexCoord[" << (i+textureOffset)    <<"]+vec4(0.0,0.0,fZOffSet,0.0) ).r;"   << std::endl;
        sstr << "    float shadow0" << i <<" = shadow2DProj( shadowTexture" <<
          i <<",gl_TexCoord[" << (i+textureOffset)    <<"]+vec4(-fTexelSize,-fTexelSize,fZOffSet,0.0) ).r;" << std::endl;
        sstr << "    float shadow1" << i <<" = shadow2DProj( shadowTexture" <<
          i <<",gl_TexCoord[" << (i+textureOffset)    <<"]+vec4( fTexelSize,-fTexelSize,fZOffSet,0.0) ).r;" << std::endl;
        sstr << "    float shadow2" << i <<" = shadow2DProj( shadowTexture" <<
          i <<",gl_TexCoord[" << (i+textureOffset)    <<"]+vec4( fTexelSize, fTexelSize,fZOffSet,0.0) ).r;" << std::endl;
        sstr << "    float shadow3" << i <<" = shadow2DProj( shadowTexture" <<
          i <<",gl_TexCoord[" << (i+textureOffset)    <<"]+vec4(-fTexelSize, fTexelSize,fZOffSet,0.0) ).r;" << std::endl;
        sstr << "    float shadow" << i <<" = ( 2.0*shadowOrg"    <<    i
             <<" + shadow0" << i <<" + shadow1" << i <<" + shadow2" << i <<" + shadow3" << i << ")/6.0;"<< std::endl;
      }
    }

    sstr << "    float term0 = (1.0-shadow0)*map0; "    << std::endl;
    for (unsigned int i=1;i<NUM_PSSM_SPLITS;i++)    {
      sstr << "    float term" << i << " = map"<< i << "*(1.0-shadow"<<i<<");"<< std::endl;
    }
    sstr << "    return clamp(";
    for (unsigned int i=0;i<NUM_PSSM_SPLITS;i++)    {
      sstr << "term" << i;
      if ( i+1 < NUM_PSSM_SPLITS ){
        sstr << "+";
      }
    }
    sstr << "0.0, 1.0);" << std::endl;
    sstr << "}" << endl;
    cout << sstr.str() << endl;
    return sstr.str();
  }

  PixelLightFrag::PixelLightFrag(vector<string> &args, int numLights,
                                 std::string resPath, bool haveDiffuseMap,
				 bool havePCol)
    : ShaderFunc("plight", args) {
    stringstream s;
    funcs[0].second.push_back("pcol");
    funcs[0].second.push_back("n");
    funcs[0].second.push_back("col");
    s << "[" << numLights << "]";
    addVarying( (GLSLVarying) { "vec3", "lightVec" + s.str() } );
    addVarying( (GLSLVarying) { "vec3", "spotDir" + s.str() } );
    addVarying( (GLSLVarying) { "vec4", "diffuse" + s.str() } );
    addVarying( (GLSLVarying) { "vec4", "specular" + s.str() } );

    addVarying( (GLSLVarying) { "vec3", "eyeVec" } );
    addVarying( (GLSLVarying) { "vec4", "positionVarying" } );
    addVarying( (GLSLVarying) { "vec4", "modelVertex" } );

    addUniform( (GLSLUniform) { "vec4", "lightAmbient" + s.str() } );
    addUniform( (GLSLUniform) { "vec3", "lightEmission" + s.str() } );
    addUniform( (GLSLUniform) { "int", "lightIsSet" + s.str() } );
    addUniform( (GLSLUniform) { "int", "lightIsSpot" + s.str() } );
    addUniform( (GLSLUniform) { "float", "lightCosCutoff" + s.str() } );
    addUniform( (GLSLUniform) { "float", "lightSpotExponent" + s.str() } );
    addUniform( (GLSLUniform) { "mat4", "osg_ViewMatrixInverse" } );
    addUniform( (GLSLUniform) { "mat4", "osg_ViewMatrix" } );
    addUniform( (GLSLUniform) { "float", "lightConstantAtt" + s.str() } );
    addUniform( (GLSLUniform) { "float", "lightLinearAtt" + s.str() } );
    addUniform( (GLSLUniform) { "float", "lightQuadraticAtt" + s.str() } );
    addUniform( (GLSLUniform) { "int", "lightIsDirectional" + s.str() } );


    addUniform( (GLSLUniform) { "float", "brightness" } );
    addUniform( (GLSLUniform) { "float", "alpha" } );
    addUniform( (GLSLUniform) { "int", "useFog" } );
    addUniform( (GLSLUniform) { "int", "useNoise" } );
    addUniform( (GLSLUniform) { "int", "useShadow" } );
    addUniform( (GLSLUniform) { "int", "numLights" } );
    addUniform( (GLSLUniform) { "int", "drawLineLaser" } );

    addUniform( (GLSLUniform) { "sampler2DShadow", "osgShadow_shadowTexture" });
    addUniform( (GLSLUniform) { "sampler2D", "NoiseMap" } );
    addUniform( (GLSLUniform) { "vec2", "osgShadow_ambientBias" });
    addUniform( (GLSLUniform) { "float", "shadowScale" });
    addUniform( (GLSLUniform) { "int", "shadowSamples" } );
    addUniform( (GLSLUniform) { "float", "invShadowSamples" } );
    addUniform( (GLSLUniform) { "float", "invShadowTextureSize" } );

    addUniform( (GLSLUniform) { "vec3", "lineLaserPos" });
    addUniform( (GLSLUniform) { "vec3", "lineLaserNormal" });
    addUniform( (GLSLUniform) { "vec4", "lineLaserColor" });
    addUniform( (GLSLUniform) { "vec3", "lineLaserDirection"});
    addUniform( (GLSLUniform) { "float", "lineLaserOpeningAngle"});

#if USE_LSPSM_SHADOW
    addUniform( (GLSLUniform) { "sampler2DShadow", "shadowTexture" });
#elif USE_PSSM_SHADOW
    std::stringstream buf;
    for (unsigned int i=0; i < NUM_PSSM_SPLITS; ++i) {
      buf << "shadowTexture" << i;
      addUniform( (GLSLUniform) { "sampler2DShadow", buf.str() });
      buf.str("");

      buf << "zShadow" << i;
      addUniform( (GLSLUniform) { "float", buf.str() });
      buf.str("");
    }
    addUniform( (GLSLUniform) { "vec2", "ambientBias" });

    addDependencyCode("pssm", pssmAmount());
#endif
    resPath += "/shader/plight.frag";
    std::ifstream t(resPath.c_str());
    std::stringstream buffer;
    buffer << t.rdbuf();
    source = buffer.str();
    if(!havePCol) {
      if(haveDiffuseMap) {
	addMainVar( (GLSLVariable) { "vec4", "pcol",
	      "texture2D(diffuseMap, texCoord)" });
      }
      else {
	addMainVar( (GLSLVariable) { "vec4", "pcol", "col" });
      }
    }
    addMainVar( (GLSLVariable) { "vec3", "n",
	  "normalize( gl_FrontFacing ? normalVarying : -normalVarying )"} );
    addVarying( (GLSLVarying) { "vec3", "normalVarying" } );
  }

  string PixelLightFrag::code() const {
    return source;
  }

} // end of namespace osg_material_manager
