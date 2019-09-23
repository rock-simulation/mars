void pixellight2_frag(vec4 base, vec3 n, out vec4 outcol) {
  vec4 ambient = vec4(0.0);
  vec4 diffuse_ = vec4(0.0);
  float nDotL;
  float spot = 1.0;
  float dist, atten, x, y, s;
  for(int i=0; i<numLights; ++i) {
    if(lightIsSet[i]==1) {
      nDotL = max(dot( n, normalize(  -lightVec[i] ) ), 0.0);

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
    }
  }

  // calculate output color
  outcol = brightness* ((ambient + diffuse_)*base  + gl_FrontMaterial.emission*base);
  outcol.a = alpha*base.a;
}
