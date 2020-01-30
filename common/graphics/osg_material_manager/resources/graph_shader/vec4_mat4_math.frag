void vec4_mat4_math(in mat4 matrix, in vec4 vec, in float skalar, out vec4 vector) {
    vector = (matrix * vec)*skalar;
}