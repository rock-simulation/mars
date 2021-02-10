void sampleCubeMap(in vec3 texDir, in samplerCube texture, out vec4 color) {
  vec3 tex = normalize(vec3(texDir.x, -texDir.z, texDir.y));
  color = textureCube(texture, tex);
}
