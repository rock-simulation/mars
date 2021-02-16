float rnd(float x, float y) {
  return fract(sin(dot(vec2(x,y) ,vec2(12.9898,78.233))) * 43758.5453);
}

void pixellight_frag(vec4 base, vec3 n, out vec4 outcol) {
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
  float testZ = gl_FragCoord.z*2.0-1.0;
  float map0 = step(testZ, zShadow0);
  float map1 = step(zShadow0, testZ)*step(testZ, zShadow1);
  float map2 = step(zShadow1, testZ)*step(testZ, zShadow2);
  float shadow0 = 0.0;
  float shadow1 = 0.0;
  float shadow2 = 0.0;

  if(useShadow == 1) {
    if(shadowSamples == 1) {
      shadow0 = shadow2D(shadowTexture0, gl_TexCoord[1].xyz).r;
      shadow0 = step(0.25,shadow0);
      shadow1 = shadow2D(shadowTexture1, gl_TexCoord[2].xyz).r;
      shadow1 = step(0.25,shadow1);
      shadow2 = shadow2D(shadowTexture2, gl_TexCoord[3].xyz).r;
      shadow2 = step(0.25,shadow2);
    }
    else {
      float da = 128/shadowSamples;
      float w = gl_TexCoord[1].w*invShadowTextureSize;
      float w2 = gl_TexCoord[2].w*invShadowTextureSize;
      float w3 = gl_TexCoord[3].w*invShadowTextureSize;
      vec2 offset = floor(da*screenPos.xy*10)*shadowSamples;
      for(int k=0; k<shadowSamples; ++k) {
        for(int l=0; l<shadowSamples; ++l) {
          x = offset.x*s + k*s;
          y = offset.y*s + l*s;
          v = texture2D( NoiseMap, vec2(x,y)).xy-0.5;
          v *= 8;
          shadowCoord = gl_TexCoord[1] + vec4(v.x*w, v.y*w, 0.0, 0);
          shadow0 += shadow2DProj( shadowTexture0, shadowCoord ).r * invShadowSamples;
          shadowCoord = gl_TexCoord[2] + vec4(v.x*w2, v.y*w2, 0.0, 0);
          shadow1 += shadow2DProj( shadowTexture1, shadowCoord ).r * invShadowSamples;
          shadowCoord = gl_TexCoord[3] + vec4(v.x*w3, v.y*w3, 0.0, 0);
          shadow2 += shadow2DProj( shadowTexture2, shadowCoord ).r * invShadowSamples;
        }
      }
    }
    float term0 = map0*(1-shadow0);
    float term1 = map1*(1-shadow1);
    float term2 = map2*(1-shadow2);
    shadow = 1-clamp(term0+term1+term2, 0.0, 1.0);
  } else {
    shadow = 1.0f;
  }

  for(int i=0; i<numLights; ++i) {
    if(lightIsSet[i]==1) {
      nDotL = max(dot( n, normalize(  -lightVec[i] ) ), 0.0);
      reflected = normalize(reflect(lightVec[i], n ) );
      rDotE = max(dot( reflected, eye ), 0.0);
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
        float spot = (spotEffect > lightCosCutoff[i]) ? 1.0 : 1.0-min(1.0, pow(lightSpotExponent[i]*(lightCosCutoff[i]-spotEffect), 2));
        diffuse_  += spot*diffuseShadow * (atten*diffuse[i] * nDotL);
        test_specular_ = (spot*specularShadow * atten*specular[i]
                          * pow(rDotE, gl_FrontMaterial.shininess));
        specular_ += (gl_FrontMaterial.shininess > 0) ? test_specular_ : vec4(0.0);
      }
      else {
        ambient  += lightAmbient[i]*gl_FrontMaterial.ambient;
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
    float noiseScale = 6;
    outcol.rg += noiseAmmount*(texture2D( NoiseMap, noiseScale*screenPos.xy).zw-0.5);
    outcol.b += noiseAmmount*(texture2D( NoiseMap, noiseScale*(screenPos.xy+vec2(0.5, 0.5))).z-0.5);
  }

  if(useFog == 1) {
    // FIXME: eyevec maybe in tbn space !
    float fog = gl_Fog.density*clamp(gl_Fog.scale*(length(eyeVec)-gl_Fog.start) , 0.0, 1.0);
    outcol = mix(gl_Fog.color, outcol, 1-fog);
  }
  //outcol = vec4(positionVarying.x, shadow, positionVarying.y, 1.0);
}
