#version 330 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_uv;

uniform mat4 u_view;
uniform mat4 u_projection;

out vec3 v_pos;
out vec2 v_uv;

void main()
{
   gl_Position = vec4(a_pos, 1.0) * u_view * u_projection;
   v_uv = a_uv;
}
