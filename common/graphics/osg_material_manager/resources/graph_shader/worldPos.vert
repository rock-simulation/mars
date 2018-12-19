void worldPos(in vec4 viewPos, out vec4 worldPos) {
  worldPos = osg_ViewMatrixInverse * viewPos;
  positionVarying = worldPos;
}