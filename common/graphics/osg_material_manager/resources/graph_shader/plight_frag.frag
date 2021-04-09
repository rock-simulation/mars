void pixellight2_frag(vec4 base, vec3 n, out vec4 outcol) {
  vec4 ambient = vec4(0.0);
  vec4 diffuse_ = vec4(0.0);
  vec4 specular_ = vec4(0.0);
  vec4 test_specular_;
  vec3 eye = normalize(  eyeVec );
  float nDotL, rDotE;
  float spot = 1.0;
  float dist, atten;
  for(int i=0; i<numLights; ++i) {
    if(lightIsSet[i]==1) {
      nDotL = max(dot( n, normalize(  -lightVec[i] ) ), 0.0);
      reflected = normalize(reflect(lightVec[i], n ) );
      rDotE = max(dot( reflected, eye ), 0.0);

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
      diffuse_  += spot * (atten*diffuse[i] * nDotL);
      test_specular_ = (spot*specularShadow * atten*specular[i]
                        * pow(rDotE, gl_FrontMaterial.shininess));
      specular_ += (gl_FrontMaterial.shininess > 0) ? test_specular_ : vec4(0.0);
    }
  }

  // calculate output color
  outcol = brightness* ((ambient + diffuse_)*base  + specular_ + gl_FrontMaterial.emission*base);
  outcol.a = alpha*base.a;
}
