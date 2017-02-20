void envMap(out vec4 col, out vec3 n) {
  vec2 texCoord;
  vec4 rcol;
  vec4 gcol;
  vec4 bcol;
  vec4 nt;
  vec4 globalDiffuse;
  vec4 scale;

  texCoord = positionVarying.xy*texScale+vec2(0.5, 0.5);
  globalDiffuse = texture2D(envMapD, texCoord);
  nt = texture2D(normalMap, texCoord*envMapScale.r);
  bcol = texture2D(envMapB, texCoord.xy*envMapScale.b);
  rcol = texture2D(envMapR, texCoord.xy*envMapScale.r);
  gcol = texture2D(envMapG, texCoord.xy*envMapScale.g);
  scale = texture2D(environmentMap, texCoord);

  col = vec4(0.7*(scale.r*rcol.rgb+scale.g*gcol.rgb+scale.b*bcol.rgb)
             +0.3*globalDiffuse.rgb, 1.0);
  n = vec3(bumpNorFac*scale.r*nt.xy, (1-scale.r)+
           bumpNorFac*(1-scale.r)*nt.z);
  n = normalize(n);
}
