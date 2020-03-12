void wave2d(float sin_, float cos_, vec4 s, out vec4 v) {
  v = vec4(s.z*(sin_*s.x + cos_*s.y), s.z*(sin_*s.y + cos_*s.x), 0, 0);
}
