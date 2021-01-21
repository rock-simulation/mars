void math_float(in float value_a, in float value_b, in float offset, in float scale, out float value) {
    value = offset + scale * pow(value_a, value_b);
}