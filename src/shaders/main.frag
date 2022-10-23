#version 330 core

struct material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct dir_light
{
    vec3 dir;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct point_light
{
    vec3 pos;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

out vec4 o_col;

in vec3 v_pos;
in vec3 v_norm;
in vec2 v_uv;

uniform material u_material;
uniform sampler2D u_sampler;
uniform vec3 u_view_pos;
uniform dir_light u_dir_light;
uniform point_light u_point_light;

vec3 light_out(vec3 light_dir, vec3 light_ambient, vec3 light_diffuse, vec3 light_specular)
{
    vec3 view_dir = normalize(u_view_pos - v_pos);
    vec3 reflect_dir = reflect(-light_dir, v_norm);
    vec3 ambient = u_material.ambient * light_ambient;
    vec3 diffuse = max(dot(v_norm, light_dir), 0.0) * u_material.diffuse * light_diffuse;
    vec3 specular = pow(max(dot(view_dir, reflect_dir), 0.0), u_material.shininess) *
        u_material.specular * light_specular;

    return ambient + diffuse + specular;
}

vec3 dir_light_out(dir_light light)
{
    vec3 light_dir = normalize(-light.dir);
    return light_out(light_dir, light.ambient, light.diffuse, light.specular);
}

vec3 point_light_out(point_light light)
{
    float dist = length(light.pos - v_pos);
    vec3 light_dir = normalize(light.pos - v_pos);
    float atten = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);

    return light_out(light_dir, light.ambient, light.diffuse, light.specular) * atten;
}

void main()
{
    vec3 light_total = dir_light_out(u_dir_light) + point_light_out(u_point_light);
    // o_col = vec4(light_total, 1.0) * texture(u_sampler, v_uv);
    o_col = texture(u_sampler, v_uv);
}
