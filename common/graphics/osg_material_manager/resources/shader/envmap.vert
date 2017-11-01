void envmap_vert(out vec4 specularCol) {
  vec2 texCoord = gl_MultiTexCoord0.xy*texScale;
  vec4 scale = texture2D(environmentMap, texCoord);
  float rcol = scale.r*envMapSpecular.r;
  float gcol = scale.g*envMapSpecular.g;
  float bcol = scale.b*envMapSpecular.b;

  specularCol = vec4(gl_FrontMaterial.specular.rgb*rcol+gl_FrontMaterial.specular.rgb*gcol+gl_FrontMaterial.specular.rgb*bcol, 1);
}
