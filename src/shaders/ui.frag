#version 330 core

out vec4 o_col;

in vec2 v_uv;
in vec4 v_col;

uniform sampler2D u_texture;

void main()
{
    o_col = texture(u_texture, v_uv) * v_col;
}
