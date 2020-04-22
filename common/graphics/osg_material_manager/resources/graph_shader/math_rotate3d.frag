void math_rotate3d(in vec4 v, in float alpha, in float beta, in float gamma, out vec4 o) {
     mat3 matrix;
     float sa = sin(-alpha);
     float sb = sin(-beta);
     float sg = sin(-gamma);
     float ca = cos(-alpha);
     float cb = cos(-beta);
     float cg = cos(-gamma);

     matrix[0][0] = cb*cg;
     matrix[0][1] = -cb*sg;
     matrix[0][2] = sb;
     //matrix[0][3] = 0;
     matrix[1][0] = sa*sb*cg+ca*sg;
     matrix[1][1] = -sa*sb*sg+ca*cg;
     matrix[1][2] = -sa*cb;
     //matrix[1][3] = 0;
     matrix[2][0] = -ca*sb*cg+sa*sg;
     matrix[2][1] = ca*sb*sg+sa*cg;
     matrix[2][2] = ca*cb;
     //matrix[2][3] = 0;
     //matrix[3][3] = 0;
     //matrix[3][3] = 0;
     //matrix[3][3] = 0;
     //matrix[3][3] = 1;

     o = vec4(matrix * v.xyz, v.w);
}
