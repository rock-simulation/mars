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


#include "pixellight.h"

#include "GraphicsManager.h"

#include <sstream>
#include <iostream>

namespace mars {
  namespace graphics {

    using namespace std;
    using mars::interfaces::LightData;

#ifndef max
#define max(x,y) (x>y ? x : y)
#endif
 
    PixelLightVert::PixelLightVert(vector<string> &args, vector<LightData*> &lightList)
      : ShaderFunc("plight", args)
    {
      stringstream s;
      s << "lightVec[" << max(1,lightList.size()) << "]";
      addVarying( (GLSLVarying) { "vec3", s.str() } );
      s.str("");

      s << "attenFacs[" << max(1,lightList.size()) << "]";
      addVarying( (GLSLVarying) { "float", s.str() } );

      addVarying( (GLSLVarying) { "vec4", "eyeVec" } );
      addVarying( (GLSLVarying) { "vec4", "positionVarying" } );

      this->lightList = lightList;
    }

    string PixelLightVert::code() const
    {
      stringstream s;
      vector<LightData*>::const_iterator it;
      int lightIndex = 0;

      s << "void " << name << "(vec4 v)" << endl;
      s << "{" << endl;
      s << "    positionVarying = gl_Vertex;" << endl;
      s << "    // save the vertex vector in eye space" << endl;
      s << "    eyeVec = v;" << endl;
      s << "    float dist;" << endl;
      for(it = lightList.begin(); it != lightList.end(); ++it) {
        stringstream lightSource;
        lightSource << "gl_LightSource[" << (*it)->index << "]";

        s << "    if(" << lightSource.str() << ".position.w < 0.001 ) {" << endl;
        s << "        // directional light" << endl;
        s << "        lightVec[" << lightIndex << "] = " << lightSource.str() << ".position.xyz;" << endl;
        s << "    } else {" << endl;
        s << "        // point/spot light" << endl;
        s << "        lightVec[" << lightIndex << "] = vec3(" << lightSource.str() << ".position.xyz - v.xyz);" << endl;
        s << "    }" << endl;
        s << "    dist = length(lightVec[" << lightIndex << "]);" << endl;
        s << "    attenFacs[" << lightIndex << "] = 1.0/(gl_LightSource[" << lightIndex << "].constantAttenuation +" << endl;
        s << "                 gl_LightSource[" << lightIndex << "].linearAttenuation * dist +" << endl;
        s << "                 gl_LightSource[" << lightIndex << "].quadraticAttenuation * dist * dist);" << endl;
#if USE_LSPSM_SHADOW or USE_SM_SHADOW
        s << "    // generate coords for shadow mapping" << endl;
        s << "    gl_TexCoord[1].s = dot( eyeVec, gl_EyePlaneS[1] );" << endl;
        s << "    gl_TexCoord[1].t = dot( eyeVec, gl_EyePlaneT[1] );" << endl;
        s << "    gl_TexCoord[1].p = dot( eyeVec, gl_EyePlaneR[1] );" << endl;
        s << "    gl_TexCoord[1].q = dot( eyeVec, gl_EyePlaneQ[1] );" << endl;
#endif
        ++lightIndex;
      }
      s << "}" << endl;
      return s.str();
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

    PixelLightFrag::PixelLightFrag(vector<string> &args, bool useFog,
                                   bool useNoise,
                                   vector<LightData*> &lightList)
      : ShaderFunc("plight", args)
    {
      stringstream s;

      s << "lightVec[" << max(1,lightList.size()) << "]";
      addVarying( (GLSLVarying) { "vec3", s.str() } );
      s.str("");

      s << "attenFacs[" << max(1,lightList.size()) << "]";
      addVarying( (GLSLVarying) { "float", s.str() } );

      addVarying( (GLSLVarying) { "vec4", "eyeVec" } );
      addVarying( (GLSLVarying) { "vec4", "positionVarying" } );

      addUniform( (GLSLUniform) { "float", "brightness" } );
      addUniform( (GLSLUniform) { "float", "alpha" } );

#if USE_LSPSM_SHADOW
      addUniform( (GLSLUniform) { "sampler2DShadow", "shadowTexture" });
#elif USE_SM_SHADOW
      addUniform( (GLSLUniform) { "sampler2DShadow", "osgShadow_shadowTexture" });
      addUniform( (GLSLUniform) { "vec2", "osgShadow_ambientBias" });
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

      this->useFog = useFog;
      this->useNoise = useNoise;
      this->lightList = lightList;
    }

    string PixelLightFrag::code() const
    {
      stringstream s;
      vector<LightData*>::const_iterator it;
      int lightIndex = 0;

      // NOTE(daniel): a little messed up in here...
      //    i will clean up when shadow stuff is ready...


      s << "float rnd(float x, float y) {" << endl;
      s << "return fract(sin(dot(vec2(x,y) ,vec2(12.9898,78.233))) * 43758.5453);" << endl;
      s << "}" << endl << endl;

      s << "void " << name << "(vec4 base, vec3 n, out vec4 outcol)" << endl;
      s << "{" << endl;
      s << "    vec4 ambient = vec4(0.0);" << endl;
      s << "    vec4 diffuse = vec4(0.0);" << endl;
      s << "    vec4 specular = vec4(0.0);" << endl;
      s << endl;
      s << "    vec3 eye = normalize( - eyeVec.xyz );" << endl;
      s << "    float pixelDistance = length( eyeVec );" << endl;
      s << "    float nDotL, rDotE, shadow, diffuseShadow;" << endl;
      s << endl;

      for(it = lightList.begin(); it != lightList.end(); ++it) {
        stringstream lightSource;
        lightSource << "gl_LightSource[" << (*it)->index << "]";

        s << "    nDotL = dot( n, normalize(  lightVec[" << lightIndex << "] ) );" << endl;
        s << endl;
        s << "    //if( gl_LightSource[" << (*it)->index << "].position.w < 0.001 ) { // directional" << endl;
        s << "        ambient += gl_FrontLightProduct[" << (*it)->index << "].ambient;" << endl;
        s << "    //} else {" << endl;
        s << "    //    ambient += attenFacs[" << lightIndex <<
          "] * gl_FrontLightProduct[" << (*it)->index << "].ambient;" << endl;
        s << "    //}" << endl;
        s << "    // if nDotL<=0, we can skip diffuse/specular calculation," << endl;
        s << "    // since this pixel is not lit anyway." << endl;
        s << "    rDotE = 1.0; " << endl;

        //s << "    if (nDotL <= 0.0) {nDotL = nDotL; rDotE = -1;}" << endl;
        //s << "    if (nDotL <= 0.0) return;" << endl;
        s << endl;

        s << "    if(gl_FrontMaterial.shininess > 0.0) {" << endl;
        s << "        vec3 reflected = normalize( reflect( - lightVec[" << lightIndex << "], n ) );" << endl;
        s << "        rDotE = max(dot( reflected, eye ), 0.0);" << endl << endl;
        s << "    }" << endl;

#if USE_LSPSM_SHADOW
        s << "    shadow = shadow2DProj( shadowTexture, gl_TexCoord[6] ).r;" << endl;
#elif USE_SM_SHADOW
        s << "    shadow = (osgShadow_ambientBias.x + shadow2DProj( osgShadow_shadowTexture, gl_TexCoord[1] ).r * osgShadow_ambientBias.y);" << endl;
#elif USE_PSSM_SHADOW
        s << "    shadow = pssmAmount();" << endl;
#else
        s << "    shadow = 1.0f;" << endl;
#endif
        // real light still emits some diffuse light in shadowed region
        s << "    diffuseShadow = 0.1 + 0.9 * shadow;" << endl;

        s << "    // add diffuse and specular light" << endl;
        s << "    //if( gl_LightSource[" << (*it)->index << "].position.w < 0.001 ) { // directional" << endl;
        s << "        diffuse  += diffuseShadow * gl_FrontLightProduct[" << (*it)->index << "].diffuse * nDotL;" << endl;
        s << "        specular += shadow * gl_FrontLightProduct[" << (*it)->index <<
          "].specular * pow(rDotE, gl_FrontMaterial.shininess);" << endl;
        s << "    //} else if( gl_LightSource[" << (*it)->index << "].spotCutoff > 179.0 ) { // point " << endl;
        s << "    //    float dist = length(lightVec[" << lightIndex << "]);" << endl;
        s << "    //    float attenuation = 1.0/(";
        s << "    //                " << lightSource.str() << ".constantAttenuation +" << endl;
        s << "    //                " << lightSource.str() << ".linearAttenuation * dist +" << endl;
        s << "    //                " << lightSource.str() << ".quadraticAttenuation * dist * dist );" << endl;
        s << "    //    diffuse  += diffuseShadow * attenuation * (gl_FrontLightProduct[" << (*it)->index << "].diffuse * nDotL);" << endl;
        s << "    //    if(gl_FrontMaterial.shininess > 0.0)" << endl;
        s << "    //        specular += shadow * attenuation * gl_FrontLightProduct[" << (*it)->index << "].specular" << endl;
        s << "    //            * pow(rDotE, gl_FrontMaterial.shininess);" << endl;
        s << "    //} else { // spot" << endl;
        s << "    //    float spotEffect = dot( normalize( gl_LightSource[" << (*it)->index <<
          "].spotDirection ), -normalize( lightVec[" << lightIndex << "]  ) );" << endl;
        s << "    //    if (spotEffect > gl_LightSource[" << (*it)->index << "].spotCosCutoff) {" << endl;
        s << "    //        float dist = length(lightVec[" << lightIndex << "]);" << endl;
        s << "    //        float attenuation = spotEffect / (" << endl;
        s << "    //            " << lightSource.str() << ".constantAttenuation +" << endl;
        s << "    //            " << lightSource.str() << ".linearAttenuation * dist +" << endl;
        s << "    //            " << lightSource.str() << ".quadraticAttenuation * dist * dist);" << endl;
        s << "    //        diffuse  += diffuseShadow * attenuation * (gl_FrontLightProduct[" << (*it)->index << "].diffuse * nDotL);" << endl;
        s << "    //        specular += shadow * attenuation * gl_FrontLightProduct[" << (*it)->index << "].specular * pow(rDotE, gl_FrontMaterial.shininess);" << endl;
        s << "    //    }" << endl;
        s << "    //}" << endl;

        ++lightIndex;
      }

      // calculate output color
      s << "" << endl;
      s << "    outcol = brightness* ((ambient + diffuse)*base  + specular + gl_FrontMaterial.emission*base);" << endl;
      s << "    //outcol = ((gl_FrontLightModelProduct.sceneColor + ambient + diffuse) * vec4(1) + specular);" << endl;
      s << "    outcol.a = alpha*base.a;" << endl;
      if(useNoise) {
        s << "    vec3 vNoise, vNoise2;" << endl;
        s << "    vNoise.x = 0.000001*floor(100000.0*positionVarying.x);" << endl;
        s << "    vNoise.y = 0.000001*floor(100000.0*positionVarying.y);" << endl;
        s << "    vNoise.z = 0.000001*floor(100000.0*positionVarying.z);" << endl;
        s << "    vNoise2.x = rnd(vNoise.x, vNoise.y+vNoise.z);" << endl;
        s << "    vNoise2.y = rnd(vNoise.y, vNoise.x-vNoise.z);" << endl;
        s << "    vNoise2.z = rnd(vNoise.x+vNoise.y, vNoise.y-vNoise.x+vNoise.z);" << endl;
        s << "    outcol.r += 0.04*rnd(vNoise2.x, vNoise2.y+vNoise2.z);" << endl;
        s << "    outcol.g += 0.04*rnd(vNoise2.y, vNoise2.x-vNoise2.z);" << endl;
        s << "    outcol.b += 0.04*rnd(vNoise2.x+vNoise2.y, vNoise2.y-vNoise2.x+vNoise2.z);" << endl;
      }
      s << "    //outcol.rgb = vec3(gl_TexCoord[1],0);" << endl;
      s << "" << endl;
      if(useFog) {
        s << "    // FIXME: eyevec maybe in tbn space !" << endl;
        s << "    float fog = clamp(gl_Fog.scale*(gl_Fog.end + eyeVec.z), 0.0, 1.0);" << endl;
        s << "    outcol = mix(gl_Fog.color, outcol, fog);" << endl;
      }
      s << "}" << endl;

      return s.str();
    }

  } // end of namespace graphics
} // end of namespace mars
