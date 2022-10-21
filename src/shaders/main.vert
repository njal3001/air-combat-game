#version 330 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_norm;
layout (location = 2) in vec2 a_uv;

uniform mat4 u_model;
uniform mat4 u_normal;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec3 v_pos;
out vec3 v_norm;
out vec2 v_uv;

void main()
{
   gl_Position = vec4(a_pos, 1.0) * u_model * u_view * u_projection;
   v_pos = vec3(vec4(a_pos, 1.0) * u_model);
   v_norm = vec3(vec4(a_norm, 1.0) * u_normal);
   v_uv = a_uv;
}
