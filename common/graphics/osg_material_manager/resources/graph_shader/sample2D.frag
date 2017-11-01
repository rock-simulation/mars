void sample2D(in vec2 texCoord, in sampler2D texture, out vec4 color) {
  color = texture2D(texture, texCoord);
}