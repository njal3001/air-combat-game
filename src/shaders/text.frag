#version 330 core

out vec4 o_col;

in vec2 v_uv;

uniform sampler2D u_bitmap;

void main()
{
    o_col = texture(u_bitmap, v_uv);
}
