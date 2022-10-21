#version 330 core

struct material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct light
{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

out vec4 o_col;

in vec3 v_pos;
in vec3 v_norm;
in vec2 v_uv;

uniform material u_material;
uniform sampler2D u_sampler;
uniform vec3 u_view_pos;
uniform light u_light;

void main()
{
    vec3 view_dir = normalize(u_view_pos - v_pos);
    vec3 reflect_dir = reflect(-u_light.direction, v_norm);
    vec3 ambient = u_material.ambient * u_light.ambient;
    vec3 diffuse = max(dot(v_norm, u_light.direction), 0.0) * u_material.diffuse * u_light.diffuse;
    vec3 specular = pow(max(dot(view_dir, reflect_dir), 0.0), u_material.shininess) *
        u_material.specular * u_light.specular;
    o_col = vec4((u_material.ambient + diffuse + specular), 1.0) * texture(u_sampler, v_uv);
}
