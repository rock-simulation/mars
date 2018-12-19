void mix_vec4(in vec4 vec_a, in vec4 vec_b, in float fac, out vec4 vector) {
    vector = (1-fac)*vec_a + fac*vec_b;
}