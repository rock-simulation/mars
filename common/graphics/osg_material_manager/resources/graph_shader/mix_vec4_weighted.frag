void mix_vec4_weighted(in vec4 vector_a, in vec4 vector_b, in float fac_a, in float fac_b, out vec4 vector) {

    vector = fac_a*vector_a + fac_b*vector_b;
}
