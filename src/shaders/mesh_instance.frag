#version 330 core

out vec4 o_col;

in vec2 v_uv;

uniform sampler2D u_sampler;

void main()
{
    // TODO: Get color from attrib?
    o_col = vec4(3.0, 3.0, 3.0, 1.0) * texture(u_sampler, v_uv);
}
