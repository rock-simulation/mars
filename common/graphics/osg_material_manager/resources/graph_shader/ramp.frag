void ramp(in float in1, in float start, in float end, in float scale, in float offset, out float out1) {
  float x1 = max(in1, start);
  x1 = min(x1, end);
  out1 = offset+scale*(x1-start)/(end-start);
}
