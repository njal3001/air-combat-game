#version 330 core

layout (location = 0) in vec4 a_vert;

out vec2 v_uv;

uniform mat4 u_projection;

void main()
{
    gl_Position = vec4(a_vert.xy, 0.0, 1.0) * u_projection;

    v_uv = a_vert.zw;
}
