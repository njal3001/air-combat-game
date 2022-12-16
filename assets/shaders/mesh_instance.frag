#version 330 core

out vec4 o_col;

in vec3 v_pos;
in vec2 v_uv;

uniform sampler2D u_sampler;

void main()
{
    o_col = texture(u_sampler, v_uv);
}
