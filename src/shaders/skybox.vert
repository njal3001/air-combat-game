#version 330 core

layout (location = 0) in vec3 a_pos;

out vec3 v_uv;

uniform mat4 u_view;
uniform mat4 u_projection;

void main()
{
    v_uv = a_pos;

    vec4 pos = vec4(a_pos, 1.0) * u_view * u_projection;
    gl_Position = pos.xyww;
}
