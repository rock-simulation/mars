void viewPos(in vec4 modelPos, out vec4 viewPos) {
  viewPos = gl_ModelViewMatrix * modelPos;
}