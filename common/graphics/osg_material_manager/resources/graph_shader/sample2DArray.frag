void sample2DArray(in vec2 texCoord, in sampler2D texture, in float layer, out vec4 color) {
  color = texture2DArray(texture, vec3(texCoord, layer));
}