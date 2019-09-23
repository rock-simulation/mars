void specular_frag(vec4 base, vec3 normal, out vec4 outcol) {
  vec4 specular_ = vec4(0.0);
  vec4 test_specular_;
  vec3 eye = normalize(  eyeVec );
  vec3 reflected;
  float rDotE;

  float dist, atten;
  for(int i=0; i<numLights; ++i) {
    if(lightIsSet[i]==1) {
      reflected = normalize(reflect(lightVec[i], normal ) );
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
        float spot = (spotEffect > lightCosCutoff[i]) ? 1.0 : 1.0-min(1.0, pow(lightSpotExponent[i]*(lightCosCutoff[i]-spotEffect), 2));
        test_specular_ = (spot*atten*specular[i]
                          * pow(rDotE, gl_FrontMaterial.shininess));
        specular_ += (gl_FrontMaterial.shininess > 0) ? test_specular_ : vec4(0.0);
      }
      else {
        test_specular_ = (specular[i]
                          * pow(rDotE, gl_FrontMaterial.shininess)); //needed as in some driver implementations, pow(0,0) yields NaN
        specular_ += (gl_FrontMaterial.shininess > 0) ? test_specular_ : vec4(0.0);
      }
    }
  }

  // calculate output color
  outcol = brightness*specular_ + base;

}
