float rnd(float x, float y) {
  return fract(sin(dot(vec2(x,y) ,vec2(12.9898,78.233))) * 43758.5453);
}

void pixellight_frag(vec4 base, vec3 n, float shadow, out vec4 outcol) {
  vec4 ambient = vec4(0.0);
  vec4 diffuse_ = vec4(0.0);
  vec4 specular_ = vec4(0.0);
  vec4 test_specular_;
  vec3 eye = normalize(  eyeVec );
  vec3 reflected;
  float nDotL, rDotE;
  float dist, atten;
  vec2 v;
  vec4 screenPos = (gl_ModelViewProjectionMatrix * modelVertex);
  screenPos /= screenPos.w;

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
        diffuse_  += spot * shadow * (atten*diffuse[i] * nDotL);
        test_specular_ = (spot * shadow * atten*specular[i]
                          * pow(rDotE, gl_FrontMaterial.shininess));
        specular_ += (gl_FrontMaterial.shininess > 0) ? test_specular_ : vec4(0.0);
        ambient  += lightAmbient[i]*gl_FrontMaterial.ambient;
      }
      else {
        ambient  += lightAmbient[i]*gl_FrontMaterial.ambient;
        diffuse_  += shadow*(atten*diffuse[i] * nDotL);
        test_specular_ = shadow*(specular[i]
                          * pow(rDotE, gl_FrontMaterial.shininess)); //needed as in some driver implementations, pow(0,0) yields NaN
        specular_ += (gl_FrontMaterial.shininess > 0) ? test_specular_ : vec4(0.0);
      }
    }
  }

  // calculate output color
  outcol = brightness* ((ambient + diffuse_)*base  + specular_ + gl_FrontMaterial.emission*base);

  /* if(drawLineLaser == 1) { */
  /*   vec3 lwP = positionVarying.xyz - lineLaserPos.xyz; */
  /*   if(abs(dot(lineLaserNormal.xyz, lwP)) < 0.002) { */
  /*     vec3 lwPNorm = normalize(lwP); */
  /*     vec3 directionNorm = normalize(lineLaserDirection); */
  /*     float v2Laser = acos( dot(directionNorm, lwPNorm) ); */
  /*     if( v2Laser < (lineLaserOpeningAngle / 2.0f) ){ */
  /*       outcol = lineLaserColor; */
  /*     } */
  /*   } */
  /* } */

  outcol.a = alpha*base.a;

  if(useNoise == 1) {
    float noiseScale = 6;
    outcol.rg += noiseAmmount*(texture2D( NoiseMap, noiseScale*screenPos.xy).zw-0.5);
    outcol.b += noiseAmmount*(texture2D( NoiseMap, noiseScale*(screenPos.xy+vec2(0.5, 0.5))).z-0.5);
  }

  /* if(useFog == 1) { */
  /*   // FIXME: eyevec maybe in tbn space ! */
  /*   float fog = gl_Fog.density*clamp(gl_Fog.scale*(length(eyeVec)-gl_Fog.start) , 0.0, 1.0); */
  /*   outcol = mix(gl_Fog.color, outcol, 1-fog); */
  /* } */

  //outcol = vec4(positionVarying.x, shadow, positionVarying.y, 1.0);
}
