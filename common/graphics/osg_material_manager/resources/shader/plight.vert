float rnd(float x, float y) {
  return fract(sin(dot(vec2(x,y) ,vec2(12.9898,78.233))) * 43758.5453);
}

void pixellight_vert(vec4 v, vec4 eye, vec4 scol) {
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
        // generate coords for shadow mapping
        gl_TexCoord[1].s = dot( eye, gl_EyePlaneS[1] );
        gl_TexCoord[1].t = dot( eye, gl_EyePlaneT[1] );
        gl_TexCoord[1].p = dot( eye, gl_EyePlaneR[1] );
        gl_TexCoord[1].q = dot( eye, gl_EyePlaneQ[1] );

        gl_TexCoord[2].s = dot( eye, gl_EyePlaneS[2] );
        gl_TexCoord[2].t = dot( eye, gl_EyePlaneT[2] );
        gl_TexCoord[2].p = dot( eye, gl_EyePlaneR[2] );
        gl_TexCoord[2].q = dot( eye, gl_EyePlaneQ[2] );

        gl_TexCoord[3].s = dot( eye, gl_EyePlaneS[3] );
        gl_TexCoord[3].t = dot( eye, gl_EyePlaneT[3] );
        gl_TexCoord[3].p = dot( eye, gl_EyePlaneR[3] );
        gl_TexCoord[3].q = dot( eye, gl_EyePlaneQ[3] );

      }
    }
  }
}
