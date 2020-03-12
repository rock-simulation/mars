void ramp_color(in float in1, in float start, in float end, in vec4 color_1, in vec4 color_2, in vec4 color_3, in vec4 color_4, out vec4 out1) {

float p1 = start;
float p2 = (((end-start)/3) + start);
float p3 = (end - ((end-start)/3));
float p4 = end;
float x1;
float fac;

if (in1 < p2) {

  x1 = max(in1, p1);
  x1 = min(x1, p2);
  fac = (x1-p1)/(p2-p1);
  out1 = (1-fac)*color_1 + fac*color_2;
}

if ((in1 > p2) && (in1 < p3)) {

  x1 = max(in1, p2);
  x1 = min(x1, p3);
  fac = (x1-p2)/(p3-p2);
  out1 = (1-fac)*color_2 + fac*color_3;
}

if (in1 > p3) {

  x1 = max(in1, p3);
  x1 = min(x1, p4);
  fac = (x1-p3)/(p4-p3);
  out1 = (1-fac)*color_3 + fac*color_4;
}

}
