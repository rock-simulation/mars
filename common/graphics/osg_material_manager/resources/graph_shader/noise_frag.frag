void noise_frag(vec4 base, float scale, float intensity, out vec4 outcol) {
  outcol = base;
  if(useNoise == 1) {
    vec4 screenPos = (gl_ModelViewProjectionMatrix * modelVertex);
    screenPos /= screenPos.w;
    outcol.rg += intensity*(texture2D( NoiseMap, scale*screenPos.xy).zw-0.5);
    outcol.b += intensity*(texture2D( NoiseMap, scale*(screenPos.xy+vec2(0.5, 0.5))).z-0.5);
  }
}
