float rnd_instance(float x, float y) {
  return fract(sin(dot(vec2(x,y) ,vec2(12.9898,78.233))) * 43758.5453);
}

void rnd_instance_offset(out vec4 offset) {
  offset = vec4(rnd(float(gl_InstanceIDARB)*0.1, float(gl_InstanceIDARB) *0.2), rnd(float(gl_InstanceIDARB)*0.2, float(gl_InstanceIDARB)*0.3), rnd(float(gl_InstanceIDARB)*0.1, float(gl_InstanceIDARB)*0.9), rnd(float(gl_InstanceIDARB)*0.7, float(gl_InstanceIDARB)*0.7)); //Priority: -1
}

