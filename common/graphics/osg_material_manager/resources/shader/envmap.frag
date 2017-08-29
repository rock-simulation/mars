void envmap_frag(out vec4 col, out vec4 nt) {
  vec2 texCoord;
  vec4 rcol;
  vec4 gcol;
  vec4 bcol;
  vec4 rnt;
  vec4 gnt;
  vec4 bnt;
  vec4 globalDiffuse;
  vec4 scale;

  texCoord = positionVarying.xy*texScale+vec2(0.5, 0.5);
  globalDiffuse = texture2D(envMapD, texCoord);
  bcol = texture2D(envMapB, texCoord.xy*envMapScale.b);
  rcol = texture2D(envMapR, texCoord.xy*envMapScale.r);
  gcol = texture2D(envMapG, texCoord.xy*envMapScale.g);
  bnt = texture2D(normalMapB, texCoord.xy*envMapScale.b);
  rnt = texture2D(normalMapR, texCoord.xy*envMapScale.r);
  gnt = texture2D(normalMapG, texCoord.xy*envMapScale.g);
  scale = texture2D(environmentMap, texCoord);

  col = vec4(0.7*(scale.r*rcol.rgb+scale.g*gcol.rgb+scale.b*bcol.rgb)
             +0.3*globalDiffuse.rgb, 1.0);
  nt = vec4(scale.r*rnt.rgb+scale.g*gnt.rgb+scale.b*bnt.rgb, 1.0);
}
