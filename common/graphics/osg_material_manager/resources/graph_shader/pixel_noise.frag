void pixel_noise(in vec4 in_vec4, in float noise_ammount, in float pixel_scale, out vec4 out_vec4) {
  vec4 screenPos = (gl_ModelViewProjectionMatrix * modelVertex);
  screenPos /= screenPos.w;
  out_vec4 = in_vec4;
  if(useNoise == 1) {
    out_vec4.rg += noiseAmmount*noise_ammount*(texture2D( NoiseMap, pixel_scale*screenPos.xy).zw-0.5);
    out_vec4.b += noiseAmmount*noise_ammount*(texture2D( NoiseMap, pixel_scale*(screenPos.xy+vec2(0.5, 0.5))).z-0.5);
  }
}
