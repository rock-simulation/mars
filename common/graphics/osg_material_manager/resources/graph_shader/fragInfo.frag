void fragInfo(out vec4 ambient, out vec4 diffuse, out vec4 specular, out vec4 emission, out vec2 texCoord) {
  texCoord = gl_TexCoord[0].xy*texScale;
  ambient = gl_FrontMaterial.ambient;
  diffuse = gl_FrontMaterial.diffuse;
  specular = gl_FrontMaterial.specular;
  emission = gl_FrontMaterial.emission;
}