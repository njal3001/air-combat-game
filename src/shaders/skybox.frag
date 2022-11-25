#version 330 core

out vec4 o_col;

in vec3 v_uv;

uniform samplerCube u_skybox;
uniform vec4 u_fog_color;
uniform float u_fog_amount;

void main()
{
    o_col = mix(texture(u_skybox, v_uv), u_fog_color, u_fog_amount);
    // o_col = texture(u_skybox, v_uv);
}
