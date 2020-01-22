void pixellight2_vert(vec4 v, vec4 eye, vec4 scol) {
  // save the vertex to eye vector in world space
  eyeVec = osg_ViewMatrixInverse[3].xyz-v.xyz;
  for(int i=0; i<numLights; ++i) {
    if(lightIsSet[i] == 1) {
      if(lightIsDirectional[i] == 1) {
        lightVec[i] = -lightPos[i];
      } else {
        lightVec[i] = v.xyz-lightPos[i];
      }
      diffuse[i] = lightDiffuse[i]*gl_FrontMaterial.diffuse;
      specular[i] = lightSpecular[i]*scol;
      spotDir[i] = lightSpotDir[i];
    }
  }
}
