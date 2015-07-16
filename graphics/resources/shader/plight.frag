float rnd(float x, float y) {
  return fract(sin(dot(vec2(x,y) ,vec2(12.9898,78.233))) * 43758.5453);
}

void plight(vec4 base, vec3 n, out vec4 outcol) {
  vec4 ambient = vec4(0.0);
  vec4 diffuse_ = vec4(0.0);
  vec4 specular_ = vec4(0.0);
  vec4 test_specular_;
  vec3 eye = normalize(  eyeVec );
  vec3 reflected;
  float nDotL, rDotE, shadow, shadow2, diffuseShadow;
  float dist, atten, x, y;
  vec4 shadowCoord = gl_TexCoord[2];
  vec2 v;

  for(int i=0; i<numLights; ++i) {
    if(lightIsSet[i]==1) {
      nDotL = max(dot( n, normalize(  -lightVec[i] ) ), 0.0);
      reflected = normalize( reflect( lightVec[i], n ) );
      rDotE = max(dot( reflected, eye ), 0.0);
      if(useShadow == 1) {
        shadow = 0;
        for(int k=0; k<shadowSamples; ++k) {
          x = 100*positionVarying.x+100*shadowOffsets[k*2+1].x;
          y = 100*positionVarying.y+100*shadowOffsets[k*2+1].y;
          v.x = (rnd(x, y)-0.5)*invShadowTextureSize*50000*shadowScale;
          v.y = (rnd(y, x)-0.5)*invShadowTextureSize*50000*shadowScale;
          shadowCoord.xy = gl_TexCoord[2].xy + shadowOffsets[k*2]*invShadowTextureSize*20000*shadowScale + v;
          shadow += shadow2DProj( osgShadow_shadowTexture, shadowCoord ).r * invShadowSamples;
        }
        shadow *= shadow*shadow* osgShadow_ambientBias.y;
        shadow += osgShadow_ambientBias.x;
        shadowCoord.xy = gl_TexCoord[2].xy;
        //shadow = shadow2DProj( osgShadow_shadowTexture, shadowCoord ).r;
        shadowCoord.xy = gl_TexCoord[2].xy+vec2(1.)*invShadowTextureSize*20000*shadowScale;
        shadow2 = shadow2DProj( osgShadow_shadowTexture, shadowCoord ).r;
      } else {
        shadow = 1.0f;
      }
      //#if USE_LSPSM_SHADOW
      //shadow = shadow2DProj( shadowTexture, gl_TexCoord[6] ).r;
      //#elif USE_PSSM_SHADOW
      //shadow = pssmAmount();
      //#endif
      // real light still emits some diffuse light in shadowed region
      diffuseShadow = shadow;//0.1 + 0.9 * shadow;
      float specularShadow = shadow > 0.999 ? 1. : 0.;

      // add diffuse and specular light
      if(lightIsDirectional[i] == 1) {
        atten = 1.0;
      }
      else {
        dist = length(lightVec[i]);
        atten = 1.0/(lightConstantAtt[i] +
                     lightLinearAtt[i] * dist +
                     lightQuadraticAtt[i] * dist * dist);
      }
      if(lightIsSpot[i]==1) {
        float spotEffect = dot( normalize( spotDir[i] ), normalize( lightVec[i]  ) );
        float spot = (spotEffect > lightCosCutoff[i]) ? 1.0 : 1.0-min(1.0, pow(lightSpotExponent[i]*(lightCosCutoff[i]-spotEffect), 2));
        diffuse_  += spot*diffuseShadow * (atten*diffuse[i] * nDotL);
        test_specular_ = (spot*specularShadow * atten*specular[i]
                          * pow(rDotE, gl_FrontMaterial.shininess));
        specular_ += (gl_FrontMaterial.shininess > 0) ? test_specular_ : vec4(0.0);
      }
      else {
        ambient  += diffuseShadow*lightAmbient[i]*gl_FrontMaterial.ambient;
        diffuse_  += diffuseShadow * (atten*diffuse[i] * nDotL);
        test_specular_ = (specularShadow * specular[i]
                          * pow(rDotE, gl_FrontMaterial.shininess)); //needed as in some driver implementations, pow(0,0) yields NaN
        specular_ += (gl_FrontMaterial.shininess > 0) ? test_specular_ : vec4(0.0);
      }
    }
  }

  // calculate output color
  outcol = brightness* ((ambient + diffuse_)*base  + specular_ + gl_FrontMaterial.emission*base);

  if(drawLineLaser == 1) {
    vec3 lwP = positionVarying.xyz - lineLaserPos.xyz;
    if(abs(dot(lineLaserNormal.xyz, lwP)) < 0.002) {
      vec3 lwPNorm = normalize(lwP);
      vec3 directionNorm = normalize(lineLaserDirection);
      float v2Laser = acos( dot(directionNorm, lwPNorm) );
      if( v2Laser < (lineLaserOpeningAngle / 2.0f) ){
        outcol = lineLaserColor;
      }
    }
  }
  outcol.a = alpha*base.a;
  if(useNoise == 1) {
    vec3 vNoise, vNoise2;
    vec4 v = osg_ViewMatrix*positionVarying;
    vNoise.x = 0.000001*floor(100000.0*v.x);
    vNoise.y = 0.000001*floor(100000.0*v.y);
    vNoise.z = 0.000001*floor(100000.0*v.z);
    vNoise2.x = rnd(vNoise.x, vNoise.y+vNoise.z);
    vNoise2.y = rnd(vNoise.y, vNoise.x-vNoise.z);
    vNoise2.z = rnd(vNoise.x+vNoise.y, vNoise.y-vNoise.x+vNoise.z);
    outcol.r += 0.04*rnd(vNoise2.x, vNoise2.y+vNoise2.z);
    outcol.g += 0.04*rnd(vNoise2.y, vNoise2.x-vNoise2.z);
    outcol.b += 0.04*rnd(vNoise2.x+vNoise2.y, vNoise2.y-vNoise2.x+vNoise2.z);
  }

  if(useFog == 1) {
    // FIXME: eyevec maybe in tbn space !
    float fog = clamp(gl_Fog.scale*(gl_Fog.end + eyeVec.z), 0.0, 1.0);
    outcol = mix(gl_Fog.color, outcol, fog);
  }
  /*  
  outcol.r = shadow;
  outcol.g = shadow2;
  outcol.b = 0;
  */
}
