void shadow_pssm(out float shadow) {
  vec4 shadowCoord;
  float x, y, s = 0.0078125; // 1/128
  float testZ = gl_FragCoord.z*2.0-1.0;
  float map0 = step(testZ, zShadow0);
  float map1 = step(zShadow0, testZ)*step(testZ, zShadow1);
  float map2 = step(zShadow1, testZ)*step(testZ, zShadow2);
  float shadow0 = 0.0;
  float shadow1 = 0.0;
  float shadow2 = 0.0;
  vec4 screenPos = (gl_ModelViewProjectionMatrix * modelVertex);
  vec2 v;
  screenPos /= screenPos.w;

  if(shadowSamples == 1) {
    shadow0 = shadow2D(shadowTexture0, gl_TexCoord[1].xyz).r;
    shadow0 = step(0.25,shadow0);
    shadow1 = shadow2D(shadowTexture1, gl_TexCoord[2].xyz).r;
    shadow1 = step(0.25,shadow1);
    shadow2 = shadow2D(shadowTexture2, gl_TexCoord[3].xyz).r;
    shadow2 = step(0.25,shadow2);
  }
  else {
    float da = 128/shadowSamples;
    float w = gl_TexCoord[1].w*invShadowTextureSize;
    float w2 = gl_TexCoord[2].w*invShadowTextureSize;
    float w3 = gl_TexCoord[3].w*invShadowTextureSize;
    vec2 offset = floor(da*screenPos.xy*10)*shadowSamples;
    for(int k=0; k<shadowSamples; ++k) {
      for(int l=0; l<shadowSamples; ++l) {
        x = offset.x*s + k*s;
        y = offset.y*s + l*s;
        v = texture2D( NoiseMap, vec2(x,y)).xy-0.5;
        v *= 8;
        shadowCoord = gl_TexCoord[1] + vec4(v.x*w, v.y*w, 0.0, 0);
        shadow0 += shadow2DProj( shadowTexture0, shadowCoord ).r * invShadowSamples;
        shadowCoord = gl_TexCoord[2] + vec4(v.x*w2, v.y*w2, 0.0, 0);
        shadow1 += shadow2DProj( shadowTexture1, shadowCoord ).r * invShadowSamples;
        shadowCoord = gl_TexCoord[3] + vec4(v.x*w3, v.y*w3, 0.0, 0);
        shadow2 += shadow2DProj( shadowTexture2, shadowCoord ).r * invShadowSamples;
      }
    }
  }
  float term0 = map0*(1-shadow0);
  float term1 = map1*(1-shadow1);
  float term2 = map2*(1-shadow2);
  shadow = 1-clamp(term0+term1+term2, 0.0, 1.0);
}
