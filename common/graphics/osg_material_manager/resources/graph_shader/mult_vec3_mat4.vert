void mult_vec3_mat4(in vec3 vector, in mat4 matrix, out vec3 result) {

 vec4 temp_vec = vec4(vector, 1.0);
 temp_vec = matrix*temp_vec;
 result = temp_vec.xy;

}
