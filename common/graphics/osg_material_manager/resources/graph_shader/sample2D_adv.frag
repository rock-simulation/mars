void sample2D_adv(in vec2 texCoord, in sampler2D texture, in float tex_id, out vec4 color) {

  texCoord.x = mod(texCoord.x , 1.0) * 0.5;
  texCoord.y = mod(texCoord.y , 1.0) * 0.5;
  if (tex_id == 2) {
   texCoord.x = texCoord.x + 0.5;
  }
  if (tex_id == 3) {
   texCoord.y = texCoord.y + 0.5;
  }
  if (tex_id == 4) {
   texCoord.x = texCoord.x + 0.5;
   texCoord.y = texCoord.y + 0.5;
  }

  color = texture2D(texture, texCoord);

}
