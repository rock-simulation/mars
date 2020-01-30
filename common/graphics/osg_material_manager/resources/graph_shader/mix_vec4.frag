void mix_vec4(in vec4 vec_a, in vec4 vec_b, in float fac, out vec4 vector) {
  fac = max(0.0, min(fac, 1.0));
  vector = (1-fac)*vec_a + fac*vec_b;
}