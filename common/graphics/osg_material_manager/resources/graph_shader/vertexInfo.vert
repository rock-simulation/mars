void vertexInfo(out vec3 normal) {
  normal = normalize(osg_ViewMatrixInverse * vec4(gl_NormalMatrix * gl_Normal, 0.0)).xyz;
}