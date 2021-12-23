void shadow_sm(out float shadow) {
  vec4 shadowCoord = gl_TexCoord[2];
  float x, y, s = 0.0078125; // 1/128
  vec4 screenPos = (gl_ModelViewProjectionMatrix * modelVertex);
  vec2 v;
  screenPos /= screenPos.w;

  if(useShadow == 1) {
    shadow = 0;
    if(shadowSamples == 1) {
      shadow += shadow2DProj( osgShadow_shadowTexture, gl_TexCoord[2] ).r;
    }
    else {
      float da = 128/shadowSamples;
      float w = gl_TexCoord[2].w*invShadowTextureSize;
      vec2 offset = floor(da*screenPos.xy*10)*shadowSamples;
      for(int k=0; k<shadowSamples; ++k) {
        for(int l=0; l<shadowSamples; ++l) {
          x = offset.x*s + k*s;
          y = offset.y*s + l*s;
          v = texture2D( NoiseMap, vec2(x,y)).xy-0.5;
          v *= 8;
          shadowCoord = gl_TexCoord[2] + vec4(v.x*w, v.y*w, 0.0, 0);
          shadow += shadow2DProj( osgShadow_shadowTexture, shadowCoord ).r * invShadowSamples;
        }
      }
    }
    //shadow *= shadow*shadow* osgShadow_ambientBias.y;
    //shadow *= osgShadow_ambientBias.y;
    //shadow += osgShadow_ambientBias.x;
  } else {
    shadow = 1.0f;
  }
}
