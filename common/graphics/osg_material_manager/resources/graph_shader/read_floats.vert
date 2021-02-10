void read_floats(sampler2D texture, float index,
                 vec2 resolution, float skip,
                 float offset, out vec2 values) {
  float p = index*skip+offset;
  vec2 tex = vec2(floor(mod(p, resolution.x)), floor(p/resolution.x));
  float sx = 1./resolution.x;
  float sy = 1./resolution.y;
  tex.x *= sx;
  tex.y *= sy;
  tex += vec2(0.5*sx, 0.5*sy);
  vec4 color = texture2D(texture, tex);
  float gScale = 0.003891045;
  float rScale = 1-gScale;
  values = vec2(color.r*rScale+color.g*gScale, color.b*rScale+color.a*gScale);
}
