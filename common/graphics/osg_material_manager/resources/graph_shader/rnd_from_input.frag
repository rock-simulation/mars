float rnd_from_input_(float x, float y) {
  return fract((1+sin(dot(vec2(x,y) ,vec2(12.9898,78.233)))) * 43758.5453);
}

void rnd_from_input(float r, out vec4 v_rnd) {
  v_rnd = vec4(rnd_from_input_(r*10.0, r*8),
               rnd_from_input_(r*8, r*16),
               rnd_from_input_(r*4, r*4),
               rnd_from_input_(r*23, r*71))*2.0-1.0; //Priority: -1
}
