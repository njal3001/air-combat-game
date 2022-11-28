#version 330 core

out vec4 o_col;

in vec3 v_pos;

float shift(float a, float b, float c, float d, float t)
{
    return c + ((d - c) / (b - a)) * (t - a);
}

vec3 shift(float a, float b, float c, float d, vec3 t)
{
    vec3 res;
    res.x = shift(a, b, c, d, t.x);
    res.y = shift(a, b, c, d, t.y);
    res.z = shift(a, b, c, d, t.z);

    return res;
}

void main()
{
    // vec3 n = noise3(v_pos * 1000);
    float n = noise1(v_pos);
    n = shift(-1.0, 1.0, 0.0, 1.0, n);
    // o_col = vec4(n, 1.0);
    o_col = vec4(n, n, n, 1.0);
    // o_col = vec4(v_pos, 1.0);
}
