#version 330 core

out vec4 o_col;

in vec3 v_pos;
in vec2 v_uv;

uniform sampler2D u_sampler;
uniform vec3 u_view_pos;
uniform vec4 u_fog_color;
uniform float u_fog_start;
uniform float u_fog_end;

void main()
{
    vec3 diff = v_pos - u_view_pos;
    float dist = length(diff);

    float fog_amount =
        clamp((dist - u_fog_start) / (u_fog_end - u_fog_start), 0.0, 1.0);
    o_col = mix(texture(u_sampler, v_uv), u_fog_color, fog_amount);
    o_col.a = 1.0 - fog_amount;
}
