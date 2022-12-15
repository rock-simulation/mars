void shadow_pssm_vert(vec4 eye) {
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
