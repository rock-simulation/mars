void math_rotate3d(in vec4 v, in float alpha, in float beta, in float gamma, out vec4 o) {
     mat4 matrix;
     float sa = sin(alpha);
     float sb = sin(beta);
     float sg = sin(gamma);
     float ca = cos(alpha);
     float cb = cos(beta);
     float cg = cos(gamma);
     matrix[0][0] = ca*cg - sa*cb*sg;
     matrix[0][1] = -ca*sg - sa*cb*cg;
     matrix[0][2] = sa*sb;
     matrix[0][3] = 0;
     matrix[1][0] = sa*cg + ca*cb*sg;
     matrix[1][1] = -sa*sg + ca*cb*cg;
     matrix[1][2] = -ca*sb;
     matrix[1][3] = 0;
     matrix[2][0] = sb*sg;
     matrix[2][1] = sb*cg;
     matrix[2][2] = cb;
     matrix[2][3] = 0;
     matrix[3][0] = 0;
     matrix[3][1] = 0;
     matrix[3][2] = 0;
     matrix[3][3] = 1;
     o = matrix * v;
}
