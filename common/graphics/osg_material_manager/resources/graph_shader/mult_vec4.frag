void mult_vec4(in vec4 vector_a, in vec4 vector_b, in float fac_a, in float fac_b, out vec4 vector) {

 vec4 temp_vec_1;
 vec4 temp_vec_2;
 temp_vec_1 = (fac_a * vector_a);
 temp_vec_1[3] = vector_a[3];
 temp_vec_2 = (fac_b * vector_b);
 temp_vec_2[3] = vector_b[3];
 vector = temp_vec_1 * temp_vec_2;

}
