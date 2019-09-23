void pixellight_full_frag(vec4 base, vec3 n, out vec4 outcol) {
  vec4 ambient = vec4(0.0);
  vec4 diffuse_ = vec4(0.0);
  vec4 specular_ = vec4(0.0);
  vec4 test_specular_;
  vec3 eye = normalize(  eyeVec );
  vec3 reflected;
  float nDotL, rDotE, shadow, diffuseShadow;
  float dist, atten, x, y, s;
  vec4 shadowCoord = gl_TexCoord[2];
  vec2 v;
  vec4 screenPos = (gl_ModelViewProjectionMatrix * modelVertex);
  screenPos /= screenPos.w;
  s = 0.0078125; // 1/128
  for(int i=0; i<numLights; ++i) {
    if(lightIsSet[i]==1) {
      float spot = 1.0;
      nDotL = max(dot( n, normalize(  -lightVec[i] ) ), 0.0);
      reflected = normalize(reflect(lightVec[i], n ) );
      rDotE = max(dot( reflected, eye ), 0.0);
      if(useShadow == 1) {
        shadow = 0;
        if(shadowSamples == 1) {
          shadow += shadow2DProj( osgShadow_shadowTexture, gl_TexCoord[2] ).r * invShadowSamples;
        }
        else {
          float da = 128/shadowSamples;
          float w = gl_TexCoord[2].w*invShadowTextureSize;
          vec2 offset = floor(da*screenPos.xy*10)*shadowSamples;
          for(int k=0; k<shadowSamples; ++k) {
            for(int l=0; l<shadowSamples; ++l) {
              x = offset.x*s + k*s;
              y = offset.y*s + l*s;
              v = texture2D( NoiseMap, vec2(x,y)).xy-0.5;
              v *= 8;
              shadowCoord = gl_TexCoord[2] + vec4(v.x*w, v.y*w, 0.0, 0);
              shadow += shadow2DProj( osgShadow_shadowTexture, shadowCoord ).r * invShadowSamples;
            }
          }
        }
        //shadow *= shadow*shadow* osgShadow_ambientBias.y;
        //shadow *= osgShadow_ambientBias.y;
        //shadow += osgShadow_ambientBias.x;
      } else {
        shadow = 1.0f;
      }
      //#if USE_LSPSM_SHADOW
      //shadow = shadow2DProj( shadowTexture, gl_TexCoord[6] ).r;
      //#elif USE_PSSM_SHADOW
      //shadow = pssmAmount();
      //#endif
      // real light still emits some diffuse light in shadowed region
      diffuseShadow = 0.3 + 0.7 * shadow;
      float specularShadow = shadow > 0.999 ? 1. : 0.;
      // add diffuse and specular light
      if(lightIsDirectional[i] == 1) {
        atten = lightConstantAtt[i];
      }
      else {
        dist = length(lightVec[i]);
        atten = 1.0/(lightConstantAtt[i] +
                     lightLinearAtt[i] * dist +
                     lightQuadraticAtt[i] * dist * dist);
      }
      if(lightIsSpot[i]==1) {
        float spotEffect = dot( normalize( spotDir[i] ), normalize( lightVec[i]  ) );
        spot = (spotEffect > lightCosCutoff[i]) ? 1.0 : 1.0-min(1.0, pow(lightSpotExponent[i]*(lightCosCutoff[i]-spotEffect), 2));
      }
      ambient  += lightAmbient[i]*gl_FrontMaterial.ambient;
      diffuse_  += spot*diffuseShadow * (atten*diffuse[i] * nDotL);
      test_specular_ = (spot*specularShadow * atten*specular[i]
                        * pow(rDotE, gl_FrontMaterial.shininess));
      specular_ += (gl_FrontMaterial.shininess > 0) ? test_specular_ : vec4(0.0);
    }
  }

  // calculate output color
  outcol = brightness* ((ambient + diffuse_)*base  + specular_ + gl_FrontMaterial.emission*base);

  outcol.a = alpha*base.a;
}
