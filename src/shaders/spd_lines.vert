#version 330 core

layout (location = 0) in vec3 a_pos;

uniform mat4 u_model;

out vec3 v_pos;

void main()
{
    vec4 pos = vec4(a_pos, 1.0) * u_model;
    gl_Position = pos;
    v_pos = vec3(pos);
}
