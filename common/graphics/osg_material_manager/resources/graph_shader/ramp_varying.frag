void ramp_varying(in float in1, in float point1, in float point2, in float point3, in vec4 color_1, in vec4 color_2, in vec4 color_3, out vec4 out1) {

float x1;
float fac;

if (in1 < point2) {

  x1 = max(in1, point1);
  x1 = min(x1, point2);
  fac = (x1-point1)/(point2-point1);
  out1 = (1-fac)*color_1 + fac*color_2;
}

if (in1 > point2) {

  x1 = max(in1, point2);
  x1 = min(x1, point3);
  fac = (x1-point2)/(point3-point2);
  out1 = (1-fac)*color_2 + fac*color_3;
}

}
