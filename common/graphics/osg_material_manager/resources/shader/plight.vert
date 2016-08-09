float rnd(float x, float y) {
  return fract(sin(dot(vec2(x,y) ,vec2(12.9898,78.233))) * 43758.5453);
}

void plight(vec4 v, vec4 scol) {
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
      if(useShadow == 1) {
        vec4 eye = vec4((gl_ModelViewMatrix * gl_Vertex).xyz, 1.);
        // generate coords for shadow mapping
        gl_TexCoord[2].s = dot( eye, gl_EyePlaneS[2] );
        gl_TexCoord[2].t = dot( eye, gl_EyePlaneT[2] );
        gl_TexCoord[2].p = dot( eye, gl_EyePlaneR[2] );
        gl_TexCoord[2].q = dot( eye, gl_EyePlaneQ[2] );
      }
    }
  }
}
