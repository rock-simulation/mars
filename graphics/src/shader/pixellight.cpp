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
 
    PixelLightVert::PixelLightVert(vector<string> &args,
                                   vector<LightData*> &lightList,
                                   bool drawLineLaser,
                                   bool marsShadow,
                                   int numLights)
      : ShaderFunc("plight", args), drawLineLaser(drawLineLaser),
        marsShadow(marsShadow), numLights(numLights) {
      std::stringstream s;
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
      addUniform( (GLSLUniform) { "float", "lightConstantAtt" + s.str() } );
      addUniform( (GLSLUniform) { "float", "lightLinearAtt" + s.str() } );
      addUniform( (GLSLUniform) { "float", "lightQuadraticAtt" + s.str() } );
      addUniform( (GLSLUniform) { "mat4", "osg_ViewMatrixInverse" } );
      addUniform( (GLSLUniform) { "mat4", "osg_ViewMatrix" } );
    }

    string PixelLightVert::code() const {
      stringstream s;
      s << "void " << name << "(vec4 v)" << endl;
      s << "{" << endl;
      s << "  float atten;" << endl;
      s << "  // save the vertex to eye vector in world space" << endl;
      s << "  eyeVec = osg_ViewMatrixInverse[3].xyz;" << endl;
      s << "  float dist;" << endl;
      s << "  for(int i=0; i<"<< numLights << "; ++i) {" << endl;
      s << "    if(lightIsSet[i] == 1) {" << endl;
      s << "      if(lightIsDirectional[i] == 1) {" << endl;
      s << "        lightVec[i] = -lightPos[i];" << endl;
      s << "        diffuse[i] = lightDiffuse[i]*gl_FrontMaterial.diffuse;" << endl;
      s << "        specular[i] = lightSpecular[i]*gl_FrontMaterial.specular;" << endl;
      s << "      } else {" << endl;
      s << "        lightVec[i] = v.xyz-lightPos[i];" << endl;
      s << "        dist = length(lightVec[i]);" << endl;
      s << "        atten = 1.0/(lightConstantAtt[i] +" << endl;
      s << "        lightLinearAtt[i] * dist +" << endl;
      s << "        lightQuadraticAtt[i] * dist * dist);" << endl;
      s << "        diffuse[i] = lightDiffuse[i]*gl_FrontMaterial.diffuse*atten;" << endl;
      s << "        specular[i] = lightSpecular[i]*gl_FrontMaterial.specular*atten;" << endl;
      s << "      }" << endl;
      s << "      spotDir[i] = lightSpotDir[i];" << endl;
      s << "      vec4 eye = vec4((gl_ModelViewMatrix * gl_Vertex).xyz, 1.);" << endl;
      if(marsShadow) {
        s << "      // generate coords for shadow mapping" << endl;
        s << "      gl_TexCoord[2].s = dot( eye, gl_EyePlaneS[2] );" << endl;
        s << "      gl_TexCoord[2].t = dot( eye, gl_EyePlaneT[2] );" << endl;
        s << "      gl_TexCoord[2].p = dot( eye, gl_EyePlaneR[2] );" << endl;
        s << "      gl_TexCoord[2].q = dot( eye, gl_EyePlaneQ[2] );" << endl;
      }

#if USE_LSPSM_SHADOW
      s << "      // generate coords for shadow mapping" << endl;
      s << "      gl_TexCoord[2].s = dot( eye, gl_EyePlaneS[2] );" << endl;
      s << "      gl_TexCoord[2].t = dot( eye, gl_EyePlaneT[2] );" << endl;
      s << "      gl_TexCoord[2].p = dot( eye, gl_EyePlaneR[2] );" << endl;
      s << "      gl_TexCoord[2].q = dot( eye, gl_EyePlaneQ[2] );" << endl;
#endif
      s << "    }" << endl;
      s << "  }" << endl;
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
                                   bool useNoise, bool drawLineLaser,
                                   bool marsShadow, int numLights)
      : ShaderFunc("plight", args), marsShadow(marsShadow), numLights(numLights)
    {
      stringstream s;
      s << "[" << numLights << "]";
      addVarying( (GLSLVarying) { "vec3", "lightVec" + s.str() } );
      addVarying( (GLSLVarying) { "vec3", "spotDir" + s.str() } );
      addVarying( (GLSLVarying) { "vec4", "diffuse" + s.str() } );
      addVarying( (GLSLVarying) { "vec4", "specular" + s.str() } );

      addVarying( (GLSLVarying) { "vec3", "eyeVec" } );
      addVarying( (GLSLVarying) { "vec4", "positionVarying" } );

      addUniform( (GLSLUniform) { "vec4", "lightAmbient" + s.str() } );
      addUniform( (GLSLUniform) { "vec3", "lightEmission" + s.str() } );
      addUniform( (GLSLUniform) { "int", "lightIsSet" + s.str() } );
      addUniform( (GLSLUniform) { "int", "lightIsSpot" + s.str() } );
      addUniform( (GLSLUniform) { "float", "lightCosCutoff" + s.str() } );
      addUniform( (GLSLUniform) { "float", "lightSpotExponent" + s.str() } );
      addUniform( (GLSLUniform) { "mat4", "osg_ViewMatrixInverse" } );


      addUniform( (GLSLUniform) { "float", "brightness" } );
      addUniform( (GLSLUniform) { "float", "alpha" } );

      if(marsShadow) {
        addUniform( (GLSLUniform) { "sampler2DShadow", "osgShadow_shadowTexture" });
        addUniform( (GLSLUniform) { "vec2", "osgShadow_ambientBias" });
      }
      if(drawLineLaser) {
        addUniform( (GLSLUniform) { "vec3", "lineLaserPos" });
        addUniform( (GLSLUniform) { "vec3", "lineLaserNormal" });
        addUniform( (GLSLUniform) { "vec4", "lineLaserColor" });
        addUniform( (GLSLUniform) { "vec3", "lineLaserDirection"});
        addUniform( (GLSLUniform) { "float", "lineLaserOpeningAngle"});
      }

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

      this->useFog = useFog;
      this->useNoise = useNoise;
      this->drawLineLaser = drawLineLaser;
    }

    string PixelLightFrag::code() const
    {
      stringstream s;

      // NOTE(daniel): a little messed up in here...
      //    i will clean up when shadow stuff is ready...


      s << "float rnd(float x, float y) {" << endl;
      s << "return fract(sin(dot(vec2(x,y) ,vec2(12.9898,78.233))) * 43758.5453);" << endl;
      s << "}" << endl << endl;

      s << "void " << name << "(vec4 base, vec3 n, out vec4 outcol)" << endl;
      s << "{" << endl;
      s << "  vec4 ambient = vec4(0.0);" << endl;
      s << "  vec4 diffuse_ = vec4(0.0);" << endl;
      s << "  vec4 specular_ = vec4(0.0);" << endl;
      s << "  vec4 test_specular_;" << endl;
      s << endl;
      s << "  vec3 eye = normalize(  eyeVec );" << endl;
      s << "  vec3 reflected;" << endl;
      s << "  float nDotL, rDotE, shadow, diffuseShadow;" << endl;
      s << endl;

      s << "  for(int i=0; i<" << numLights << "; ++i) {" << endl;
      s << "  if(lightIsSet[i]==1) {" << endl;
      s << "    nDotL = dot( n, normalize(  -lightVec[i] ) );" << endl;
      //s << "    if(nDotL < 0) nDotL = -nDotL;" << endl;
      s << endl;
      s << "    // if nDotL<=0, we can skip diffuse/specular calculation," << endl;
      s << "    // since this pixel is not lit anyway." << endl;
      s << endl;
      s << "    reflected = normalize( reflect( lightVec[i], n ) );" << endl;
      s << "    rDotE = max(dot( reflected, eye ), 0.0);" << endl << endl;
      if(marsShadow) {
        s << "    shadow = (osgShadow_ambientBias.x + shadow2DProj( osgShadow_shadowTexture, gl_TexCoord[2] ).r * osgShadow_ambientBias.y);" << endl;
        //s << "    shadow = (shadow2DProj( osgShadow_shadowTexture, gl_TexCoord[2] ).r);" << endl;
      }
      else {
        s << "    shadow = 1.0f;" << endl;
      }
#if USE_LSPSM_SHADOW
      s << "    shadow = shadow2DProj( shadowTexture, gl_TexCoord[6] ).r;" << endl;
#elif USE_PSSM_SHADOW
      s << "    shadow = pssmAmount();" << endl;
#endif
      // real light still emits some diffuse light in shadowed region
      s << "    diffuseShadow = shadow;//0.1 + 0.9 * shadow;" << endl;
      s << "    float specularShadow = shadow > 0.999 ? 1. : 0.;" << endl;
      //s << "    attenuation = attenFacs[" << lightIndex << "];" << endl;

      s << "    // add diffuse and specular light" << endl;
      s << "    if(lightIsSpot[i]==1) {" << endl;
      s << "      float spotEffect = dot( normalize( spotDir[i] ), normalize( lightVec[i]  ) );" << endl;
      s << "        float spot = (spotEffect > lightCosCutoff[i]) ? 1.0 : 1.0-min(1.0, pow(lightSpotExponent[i]*(lightCosCutoff[i]-spotEffect), 2));" << endl;
      //s << "        spotEffect = pow(spotEffect - ;" << endl;
      s << "        diffuse_  += spot*diffuseShadow * (diffuse[i] * nDotL);" << endl;
      s << "        test_specular_ = spot*specularShadow * specular[i]" << endl;
      s << "                     * pow(rDotE, gl_FrontMaterial.shininess);" << endl;
      s << "        specular_ += (gl_FrontMaterial.shininess > 0) ? test_specular_ : vec4(0.0);" << endl;
      s << "    }" << endl;
      s << "    else {" << endl;
      s << "      ambient  += diffuseShadow*lightAmbient[i]*gl_FrontMaterial.ambient;" << endl;
      s << "        diffuse_  += diffuseShadow * (diffuse[i] * nDotL);" << endl;
      s << "        test_specular_ = specularShadow * specular[i]" << endl;
      s << "                     * pow(rDotE, gl_FrontMaterial.shininess);" << endl; //needed as in some driver implementations, pow(0,0) yields NaN
      s << "        specular_ += (gl_FrontMaterial.shininess > 0) ? test_specular_ : vec4(0.0);" << endl;
      s << "    }" << endl;
      s << "  }" << endl;
      s << "  }" << endl;

      // calculate output color
      s << "" << endl;
      s << "  outcol = brightness* ((ambient + diffuse_)*base  + specular_ + gl_FrontMaterial.emission*base);" << endl;

      if(drawLineLaser) {

        s << "  vec3 lwP = positionVarying.xyz - lineLaserPos.xyz;" << endl;
        s << "  if(abs(dot(lineLaserNormal.xyz, lwP)) < 0.002) {" << endl;
        s << "		vec3 lwPNorm = normalize(lwP);" << std::endl;
        s << "		vec3 directionNorm = normalize(lineLaserDirection);" << std::endl;
        s << "		float v2Laser = acos( dot(directionNorm, lwPNorm) );" << std::endl;
        s << "		if( v2Laser < (lineLaserOpeningAngle / 2.0f) ){" << std::endl;
        s << "			outcol = lineLaserColor;" << std::endl;
        s << "		}" << std::endl;
        s << "	   }" << std::endl;
      }
      s << "  //outcol = ((gl_FrontLightModelProduct.sceneColor + ambient + diffuse_) * vec4(1) + specular_);" << endl;
      s << "  outcol.a = alpha*base.a;" << endl;
      if(useNoise) {
        s << "  vec3 vNoise, vNoise2;" << endl;
        s << "  vNoise.x = 0.000001*floor(100000.0*positionVarying.x);" << endl;
        s << "  vNoise.y = 0.000001*floor(100000.0*positionVarying.y);" << endl;
        s << "  vNoise.z = 0.000001*floor(100000.0*positionVarying.z);" << endl;
        s << "  vNoise2.x = rnd(vNoise.x, vNoise.y+vNoise.z);" << endl;
        s << "  vNoise2.y = rnd(vNoise.y, vNoise.x-vNoise.z);" << endl;
        s << "  vNoise2.z = rnd(vNoise.x+vNoise.y, vNoise.y-vNoise.x+vNoise.z);" << endl;
        s << "  outcol.r += 0.04*rnd(vNoise2.x, vNoise2.y+vNoise2.z);" << endl;
        s << "  outcol.g += 0.04*rnd(vNoise2.y, vNoise2.x-vNoise2.z);" << endl;
        s << "  outcol.b += 0.04*rnd(vNoise2.x+vNoise2.y, vNoise2.y-vNoise2.x+vNoise2.z);" << endl;
      }
      s << "  //outcol.rgb = vec3(gl_TexCoord[1],0);" << endl;
      s << "" << endl;
      if(useFog) {
        s << "  // FIXME: eyevec maybe in tbn space !" << endl;
        s << "  float fog = clamp(gl_Fog.scale*(gl_Fog.end + eyeVec.z), 0.0, 1.0);" << endl;
        s << "  outcol = mix(gl_Fog.color, outcol, fog);" << endl;
      }
      s << "}" << endl;

      return s.str();
    }

  } // end of namespace graphics
} // end of namespace mars
