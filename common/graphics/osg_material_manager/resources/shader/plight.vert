float rnd(float x, float y) {
  return fract(sin(dot(vec2(x,y) ,vec2(12.9898,78.233))) * 43758.5453);
}

void vertexInfo(out vec3 normal) {
  normal = normalize(osg_ViewMatrixInverse * vec4(gl_NormalMatrix * gl_Normal, 0.0)).xyz;
}

void vertexOut(in vec4 viewPos, in vec4 modelPos, in vec3 normalV) {
  gl_Position = gl_ModelViewProjectionMatrix * modelPos;
  gl_ClipVertex = viewPos;
  gl_TexCoord[0].xy = gl_MultiTexCoord0.xy;
  normalVarying = normalV;
  modelVertex = modelPos;
}

void viewPos(in vec4 modelPos, out vec4 viewPos) {
  viewPos = gl_ModelViewMatrix * modelPos;
}

void worldPos(in vec4 viewPos, out vec4 worldPos) {
  worldPos = osg_ViewMatrixInverse * viewPos;
  positionVarying = worldPos;
}

void pixellight_vert(vec4 v, vec4 scol) {
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
