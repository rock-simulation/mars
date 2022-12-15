void mult_vec4_mat4(in vec4 vector, in mat4 matrix, out vec4 result) {
  mat4 m = matrix;
  /* for(int i=0; i<4; ++i) { */
  /*   for(int l=0; l<4; ++l) { */
  /*     if(m[i][l] < 0.0001) m[i][l] = 0; */
  /*     if(m[i][l] > 1.0001) m[i][l] = 1.0; */
  /*   } */
  /* } */
  result = m*vector;
}
