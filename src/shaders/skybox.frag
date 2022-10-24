#version 330 core

out vec4 o_col;

in vec3 v_uv;

uniform samplerCube u_skybox;

void main()
{
    o_col = texture(u_skybox, v_uv);
}
