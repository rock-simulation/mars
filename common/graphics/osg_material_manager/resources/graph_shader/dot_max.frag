void dot_max(in vec3 vector_a, in vec3 vector_b, in float m, out float value) {
  value = max(dot( vector_a, vector_b ), m);
}
