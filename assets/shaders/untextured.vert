#version 330 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec4 a_col;

uniform mat4 u_view;
uniform mat4 u_projection;

out vec4 v_col;

void main()
{
   gl_Position = vec4(a_pos, 1.0) * u_view * u_projection;
   v_col = a_col;
}
