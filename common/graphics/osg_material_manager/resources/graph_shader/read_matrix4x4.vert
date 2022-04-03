void read_matrix4x4_get_coords(float index, vec2 r, float sx, float sy, out vec2 tex) {
  tex = vec2(floor(mod(floor(index+0.5), r.x)), floor(floor(index+0.5)/r.x));
  tex.x *= sx;
  tex.y *= sy;
  tex += vec2(0.5*sx, 0.5*sy);
}

void read_matrix4x4(sampler2D texture, float index,
                 vec2 resolution, float skip,
                 float offset, out mat4 matrix) {
  float sx = 1./resolution.x;
  float sy = 1./resolution.y;
  float p;
  vec4 color;
  float gScale = 0.003891045;
  float rScale = 1-gScale;
  vec2 v, tex;
  int c, r;
  if(index < 0.0) index = 0.0;
  for(c=0; c<4; ++c) {
    for(r=0; r<2; ++r) {
      p = floor(index+0.5)*skip+offset+(c*2+r);
      read_matrix4x4_get_coords(p, resolution, sx, sy, tex);
      color = texture2D(texture, tex);
      v = vec2(color.r*rScale+color.g*gScale, color.b*rScale+color.a*gScale);
      v -= vec2(0.5);
      v *= 4;
      matrix[c][r*2] = v.x;
      matrix[c][r*2+1] = v.y;
    }
  }
}
