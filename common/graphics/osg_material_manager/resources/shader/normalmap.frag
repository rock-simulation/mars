/**
 * this method returns a per fragment normal from a normal map.
 * the normal exists in tangent space.
 **/

void bump(vec4 texel, vec3 n, out vec3 normal) {
  normal = n + (normalize(ttw*(texel.xyz * 2.0 - 1.0)) - n)*bumpNorFac;
  normal = normalize(normal);
}
