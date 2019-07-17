void crop_vec2(in vec2 vec_a, out float f) {
  f = 0;
  f = vec_a.x < 0?1:f;
  f = vec_a.x > 1?1:f;
  f = vec_a.y < 0?1:f;
  f = vec_a.y > 1?1:f;
}