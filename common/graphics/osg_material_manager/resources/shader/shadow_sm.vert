void shadow_sm_vert(vec4 eye) {
  // generate coords for shadow mapping

  gl_TexCoord[2].s = dot( eye, gl_EyePlaneS[2] );
  gl_TexCoord[2].t = dot( eye, gl_EyePlaneT[2] );
  gl_TexCoord[2].p = dot( eye, gl_EyePlaneR[2] );
  gl_TexCoord[2].q = dot( eye, gl_EyePlaneQ[2] );
}
