float d(float x, float step) {
  //return x;
  if(x > 0)
    return floor(x / step)*step;
  return ceil(x/step)*step;
}

void terrain_map(out vec4 vModelPos) {
  vec2 tex = vec2(gl_Vertex.y+d(osg_ViewMatrixInverse[3].y, 6.),
                  gl_Vertex.x+d(osg_ViewMatrixInverse[3].x, 6.))*texScale+vec2(0.5);
  //clamp(tex, 0, 1);
  vec4 terrainCol = texture2D(terrainMap, tex);
  vModelPos = vec4(gl_Vertex.xy, (terrainCol.r+terrainCol.g*0.0039215686)*terrainScaleZ, gl_Vertex.w);
  if(tex.x < 0.001 || tex.x > 0.999 || tex.y < 0.001 || tex.y > 0.999) {
    vModelPos.z = 0;
  } 
}
