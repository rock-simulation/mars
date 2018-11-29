float rnd(float x, float y) {
  return fract(sin(dot(vec2(x,y) ,vec2(12.9898,78.233))) * 43758.5453);
}

void terrain_map(vec2 v, out float h) {
  vec2 tex = vec2(-v.y+tarrain_map_d(-osg_ViewMatrixInverse[3].y, 6.),
                  v.x+tarrain_map_d(osg_ViewMatrixInverse[3].x, 6.))*texScale+vec2(0.5);
  //clamp(tex, 0, 1);
  // todo: use texture resolution in x and y
  int res = terrainDim;
  float step = 1./(res-1);
  vec2 mtex = mod(tex, vec2(step));
  vec2 dtex = mtex*(res-1);
  float step2 = 1./res;
  float stepx = 1.5*step2;
  float stepy = stepx;
  float step05 = 0.5*step2;
  vec2 stex = (tex - mtex)*(res-1)*step2;
  //vec4 terrainCol = texture2D(terrainMap, tex);
  vec4 terrainCol = texture2D(terrainMap, stex+vec2(step05));
  float gScale = 0.0039215686;
  float rScale = 1-gScale;
  float z = (terrainCol.r*rScale+terrainCol.g*gScale);
  //float z = (terrainCol.r+terrainCol.g*0.0039215686);
  stepx = stex.x+stepx > 1. ? step05 : stepx;
  stepy = stex.y+stepy > 1. ? step05 : stepy;
  
  terrainCol = texture2D(terrainMap, stex+vec2(stepx, step05));
  float z10 = (terrainCol.r*rScale+terrainCol.g*gScale);
  terrainCol = texture2D(terrainMap, stex+vec2(stepx, stepy));
  float z11 = (terrainCol.r*rScale+terrainCol.g*gScale);
  terrainCol = texture2D(terrainMap, stex+vec2(step05, stepy));
  float z01 = (terrainCol.r*rScale+terrainCol.g*gScale);
  float fi = dtex.x+dtex.y-1.0;
  z = fi <= 0 ? z : z11;
  float dx = fi <= 0 ? dtex.x  : 1. - dtex.y;
  float dy = fi <= 0 ? dtex.y  : 1. - dtex.x;
  z = z + (z10-z)*dx + (z01-z)*dy;
  h = z*terrainScaleZ;
  if(tex.x < 0.001 || tex.x > 0.999 || tex.y < 0.001 || tex.y > 0.999) {
    h = 0;
  }
}
