void vertexOut(in vec4 viewPos, in vec4 modelPos, in vec3 normalV) {
  gl_Position = gl_ModelViewProjectionMatrix * modelPos;
  gl_ClipVertex = viewPos;
  gl_TexCoord[0].xy = gl_MultiTexCoord0.xy;
  normalVarying = normalV;
  modelVertex = modelPos;
}