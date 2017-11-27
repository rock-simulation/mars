void instancing(out vec4 position) {
    position = gl_Vertex + vec4(0,gl_InstanceIDARB,0,0);
}