/**
 * This method calculates a matrix to transfrom the tangentspace to worldspace.
 * 
 * the normal map must contain tangents in tangent space, this value is used
 * directly in the fragment shader. the varyings are transformed because
 * the vectors must be in the same space for the light calculation to work.
 * the normal in the fragment sahder can also be transformed to eyespace instead of this,
 * but this would need more calculations, then doing it in the vertex shader.
 * @param n: the normal attribute at the processed vertex in eye space.
 **/

void bump(vec3 n) {
  // get the tangent in world space (multiplication by gl_NormalMatrix
  // transforms to eye space)
  // the tangent should point in positive u direction on the uv plane in the tangent space.
  vec3 t = normalize( (osg_ViewMatrixInverse*vec4(gl_NormalMatrix * vertexTangent.xyz, 0.0)).xyz );
  // calculate the binormal, cross makes sure tbn matrix is orthogonal
  // multiplicated by handeness.
  vec3 b = cross(n, t);
  ttw = mat3(t, b, n);
}
