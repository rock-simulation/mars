float rnd_instance2(float x, float y) {
  return fract(sin(dot(vec2(x,y) ,vec2(12.9898,78.233))) * 43758.5453);
}

void rnd_instance_offset2(vec4 p, out vec4 v_rnd) {
  v_rnd = vec4(normalize(vec2(p.x*0.1+0.5*rnd(float(gl_InstanceIDARB)*0.16, float(gl_InstanceIDARB)*0.8), p*0.1+0.5*rnd(float(gl_InstanceIDARB)*0.8, float(gl_InstanceIDARB)*0.16))), rnd(float(gl_InstanceIDARB)*0.4, float(gl_InstanceIDARB)*0.4), rnd(float(gl_InstanceIDARB)*0.23, float(gl_InstanceIDARB)*0.71))-0.5; //Priority: -1
}
