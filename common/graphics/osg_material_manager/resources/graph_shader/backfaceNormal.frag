void backfaceNormal(in vec3 n_in, out vec3 n_out) {
  n_out = normalize( gl_FrontFacing ? n_in : -n_in );
}
